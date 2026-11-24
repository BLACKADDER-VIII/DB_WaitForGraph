// Tests for the ingester module (TransactionList::ingest).
//
// ingest() reads transactions from stdin, one per line, in the format given
// in Project2.pdf:
//
//     T1: R(a); W(b,1); C
//     T2: R(b); R(c); W(a,2); C
//     T3: R(a); W(c,3); C
//
// and must produce one Schedule per transaction, in input order, each holding
// only that transaction's operations in the order they were written:
//
//     schedules()[0]:  T1:R(a); T1:W(b,1); T1:C
//     schedules()[1]:  T2:R(b); T2:R(c); T2:W(a,2); T2:C
//     schedules()[2]:  T3:R(a); T3:W(c,3); T3:C
//
// ingest() does not interleave anything. The round-robin ordering is produced
// by print_round_robin(), which must write "Output 1" from the PDF to stdout:
//
//     T1:R(a); T2:R(b); T3:R(a); T1:W(b,1); T2:R(c); T3:W(c,3); T1:C; T2:W(a,2); T3:C; T2:C
//
// Each test feeds a string to std::cin and compares the resulting schedules
// against the expected ones. Every test runs in its own child process with a
// timeout, so a crash or an infinite loop in ingest() is reported as a failure
// of that one test instead of taking down the run.
//
// Build and run from the project root:
//     c++ -std=c++17 -Iinclude src/ingester.cpp tests/ingester_test.cpp -o ingester_test
//     ./ingester_test
//
// Exit status is 0 if every test passes, 1 otherwise.

#include "ingester.hpp"  // Included first: the header must compile on its own.

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <sys/wait.h>
#include <unistd.h>

namespace {

// ingest() numbers transactions by their position in the input, starting at
// 0, so "T1" in the PDF is transNum() == 0. If the ingester is changed to use
// the label from the input instead, set this to 1.
const int FIRST_TRANS_NUM = 0;

const int TIMEOUT_SECONDS = 5;

// One expected operation. `obj` is ignored for commits. The transaction it
// belongs to is implied by which schedule it is listed under.
struct Expected {
    OpType type;
    char obj;
};

const Expected C = {Commit, '\0'};
Expected R(char obj) { return {Read, obj}; }
Expected W(char obj) { return {Write, obj}; }

// The expected operations of one transaction, and of a whole input.
typedef std::vector<Expected> ExpectedSchedule;
typedef std::vector<ExpectedSchedule> ExpectedList;

std::string describe(int trans_label, OpType type, char obj) {
    std::string s = "T" + std::to_string(trans_label) + ":";
    if (type == Commit) return s + "C";
    return s + (type == Read ? "R(" : "W(") + obj + ")";
}

// Feeds `input` to std::cin, runs ingest(), and returns the schedules.
std::vector<Schedule> ingest(const std::string& input) {
    std::istringstream in(input);
    std::streambuf* saved = std::cin.rdbuf(in.rdbuf());
    std::cin.clear();

    TransactionList transactions;
    transactions.ingest();

    std::cin.rdbuf(saved);
    return transactions.schedules();
}

// True if schedule number `index` holds exactly the operations in `expected`,
// all tagged with that transaction's number.
bool matches(Schedule& actual, const ExpectedSchedule& expected, size_t index) {
    std::vector<TransactionOperation>& ops = actual.schedule();
    if (ops.size() != expected.size()) return false;
    for (size_t i = 0; i < expected.size(); i++) {
        if (ops[i].opType() != expected[i].type) return false;
        if (ops[i].transNum() != static_cast<int>(index) + FIRST_TRANS_NUM) return false;
        if (expected[i].type != Commit && ops[i].opObj() != expected[i].obj) return false;
    }
    return true;
}

// Compares the schedules produced for `input` against `expected`, printing
// both in full on a mismatch.
bool check(const std::string& input, const ExpectedList& expected) {
    std::vector<Schedule> actual = ingest(input);

    bool ok = actual.size() == expected.size();
    for (size_t t = 0; ok && t < expected.size(); t++)
        ok = matches(actual[t], expected[t], t);
    if (ok) return true;

    std::cout << "    expected " << expected.size() << " schedule(s):\n";
    for (size_t t = 0; t < expected.size(); t++) {
        std::cout << "      [" << t << "]";
        for (const Expected& e : expected[t])
            std::cout << " " << describe(t + 1, e.type, e.obj) << ";";
        std::cout << "\n";
    }
    std::cout << "    actual " << actual.size() << " schedule(s):\n";
    for (size_t t = 0; t < actual.size(); t++) {
        std::cout << "      [" << t << "]";
        for (TransactionOperation& a : actual[t].schedule())
            std::cout << " "
                      << describe(a.transNum() + 1 - FIRST_TRANS_NUM, a.opType(), a.opObj())
                      << ";";
        std::cout << "\n";
    }
    return false;
}

// Feeds `input` to std::cin, runs ingest(), and compares the line that
// print_round_robin() writes to std::cout against `expected`.
bool check_round_robin(const std::string& input, const std::string& expected) {
    std::istringstream in(input);
    std::streambuf* saved_in = std::cin.rdbuf(in.rdbuf());
    std::cin.clear();
    TransactionList transactions;
    transactions.ingest();
    std::cin.rdbuf(saved_in);

    std::ostringstream out;
    std::streambuf* saved_out = std::cout.rdbuf(out.rdbuf());
    transactions.print_round_robin();
    std::cout.rdbuf(saved_out);

    if (out.str() == expected + "\n") return true;
    std::cout << "    expected: " << expected << "\n"
              << "    actual:   " << out.str();
    if (out.str().empty() || out.str().back() != '\n')
        std::cout << "  (no trailing newline)\n";
    return false;
}

// ---------------------------------------------------------------------------
// Tests
// ---------------------------------------------------------------------------

// The exact input from Project2.pdf: three schedules, one per transaction,
// with no interleaving.
bool pdf_example() {
    return check(
        "T1: R(a); W(b,1); C\n"
        "T2: R(b); R(c); W(a,2); C\n"
        "T3: R(a); W(c,3); C\n",
        {{R('a'), W('b'), C},
         {R('b'), R('c'), W('a'), C},
         {R('a'), W('c'), C}});
}

// Same input, but the last line ends at EOF rather than with a newline.
bool pdf_example_without_trailing_newline() {
    return check(
        "T1: R(a); W(b,1); C\n"
        "T2: R(b); R(c); W(a,2); C\n"
        "T3: R(a); W(c,3); C",
        {{R('a'), W('b'), C},
         {R('b'), R('c'), W('a'), C},
         {R('a'), W('c'), C}});
}

// A lone transaction gives a list holding a single schedule.
bool single_transaction() {
    return check("T1: R(a); W(b,1); C\n",
                 {{R('a'), W('b'), C}});
}

// Each schedule holds only its own transaction's operations, however much
// the transactions differ in length.
bool uneven_transaction_lengths() {
    return check(
        "T1: R(a); C\n"
        "T2: R(b); W(b,5); R(c); W(c,7); C\n",
        {{R('a'), C},
         {R('b'), W('b'), R('c'), W('c'), C}});
}

// The value in W(x,value) may have more than one digit.
bool multi_digit_write_value() {
    return check(
        "T1: W(a,42); C\n"
        "T2: W(b,100); R(a); C\n",
        {{W('a'), C},
         {W('b'), R('a'), C}});
}

// A blank line ends the input; anything after it is not ingested.
bool blank_line_ends_input() {
    return check(
        "T1: R(a); C\n"
        "T2: W(a,1); C\n"
        "\n"
        "T3: R(b); C\n",
        {{R('a'), C},
         {W('a'), C}});
}

// No transactions in, no schedules out.
bool empty_input() {
    return check("", {});
}

// The exact input and "Output 1" from Project2.pdf.
bool round_robin_pdf_example() {
    return check_round_robin(
        "T1: R(a); W(b,1); C\n"
        "T2: R(b); R(c); W(a,2); C\n"
        "T3: R(a); W(c,3); C\n",
        "T1:R(a); T2:R(b); T3:R(a); T1:W(b,1); T2:R(c); T3:W(c,3); "
        "T1:C; T2:W(a,2); T3:C; T2:C");
}

// A lone transaction is printed in its original order.
bool round_robin_single_transaction() {
    return check_round_robin("T1: R(a); W(b,1); C\n",
                             "T1:R(a); T1:W(b,1); T1:C");
}

// Once a short transaction has committed, the remaining passes only take
// operations from the transactions that still have some.
bool round_robin_uneven_transaction_lengths() {
    return check_round_robin(
        "T1: R(a); C\n"
        "T2: R(b); W(b,5); R(c); W(c,7); C\n",
        "T1:R(a); T2:R(b); T1:C; T2:W(b,5); T2:R(c); T2:W(c,7); T2:C");
}

// Write values are printed back exactly, including multi-digit ones.
bool round_robin_multi_digit_write_value() {
    return check_round_robin(
        "T1: W(a,42); C\n"
        "T2: W(b,100); R(a); C\n",
        "T1:W(a,42); T2:W(b,100); T1:C; T2:R(a); T2:C");
}

// ---------------------------------------------------------------------------
// Runner
// ---------------------------------------------------------------------------

struct Test {
    const char* name;
    bool (*run)();
};

const Test TESTS[] = {
    {"pdf_example", pdf_example},
    {"pdf_example_without_trailing_newline", pdf_example_without_trailing_newline},
    {"single_transaction", single_transaction},
    {"uneven_transaction_lengths", uneven_transaction_lengths},
    {"multi_digit_write_value", multi_digit_write_value},
    {"blank_line_ends_input", blank_line_ends_input},
    {"empty_input", empty_input},
    {"round_robin_pdf_example", round_robin_pdf_example},
    {"round_robin_single_transaction", round_robin_single_transaction},
    {"round_robin_uneven_transaction_lengths", round_robin_uneven_transaction_lengths},
    {"round_robin_multi_digit_write_value", round_robin_multi_digit_write_value},
};

// Runs one test in a child process and returns true if it passed.
bool run_isolated(const Test& test) {
    std::cout.flush();
    pid_t pid = fork();
    if (pid < 0) {
        std::cout << "    could not fork\n";
        return false;
    }
    if (pid == 0) {
        alarm(TIMEOUT_SECONDS);
        bool ok = false;
        try {
            ok = test.run();
        } catch (const std::exception& e) {
            std::cout << "    threw: " << e.what() << "\n";
        } catch (...) {
            std::cout << "    threw an unknown exception\n";
        }
        std::cout.flush();
        _exit(ok ? 0 : 1);
    }

    int status = 0;
    waitpid(pid, &status, 0);
    if (WIFSIGNALED(status)) {
        if (WTERMSIG(status) == SIGALRM)
            std::cout << "    timed out after " << TIMEOUT_SECONDS
                      << "s (infinite loop in ingest()?)\n";
        else
            std::cout << "    crashed with signal " << WTERMSIG(status) << "\n";
        return false;
    }
    return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}

}  // namespace

int main() {
    int failed = 0;
    int total = 0;
    for (const Test& test : TESTS) {
        total++;
        std::cout << "[ RUN  ] " << test.name << std::endl;
        bool ok = run_isolated(test);
        std::cout << (ok ? "[ PASS ] " : "[ FAIL ] ") << test.name << std::endl;
        if (!ok) failed++;
    }
    std::cout << "\n" << (total - failed) << "/" << total << " tests passed\n";
    return failed == 0 ? 0 : 1;
}
