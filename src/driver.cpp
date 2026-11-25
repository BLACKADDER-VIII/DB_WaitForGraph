// Driver: reads the transactions, then writes the round-robin schedule and
// the wait-for graph for the downstream analysis.
//
// Argument: i, a number that names this run's output files, so that several
// inputs can be processed without overwriting each other's results.
//
// Input (stdin): one transaction per line, in the format of Project2.pdf:
//
//     T1: R(a); W(b,1); C
//     T2: R(b); R(c); W(a,2); C
//     T3: R(a); W(c,3); C
//
// Output:
//   - output_schedules/out_i.txt: the round-robin ordering of the operations
//     ("Output 1"), on one line.
//   - graphs/wait_for_graph_i.graphml: the wait-for graph in GraphML, with one
//     node per transaction (T1, T2, ...) and a "transaction_op" attribute on
//     every edge.
//   - stdout: one line per file saying where it was written.
// Both paths are relative to the directory the program is run from, so run it
// from the project root. The two directories are created if they are missing.
//
// Exit status is 0 on success, 1 if the argument is missing or not a number,
// or if either file could not be written.
//
// Build and run from the project root:
//     c++ -std=c++17 -Iinclude src/driver.cpp src/graph_maker.cpp src/ingester.cpp -o wait_for_graph
//     ./wait_for_graph 1 < tests/test1.txt

#include "ingester.hpp"
#include "graph_maker.hpp"
#include <filesystem>
#include <fstream>

// True if `s` is a non-empty string of digits. The argument ends up in file
// names, so nothing else is accepted.
bool is_number(const std::string& s){
    if (s.empty()) return false;
    for (char c : s)
        if (c < '0' || c > '9') return false;
    return true;
}

int main(int argc, char** argv){
    if (argc != 2 || !is_number(argv[1])){
        std::cerr << "Usage: " << argv[0] << " i < input.txt\n"
                  << "  i: a number naming the outputs, output_schedules/out_i.txt"
                  << " and graphs/wait_for_graph_i.graphml" << std::endl;
        return 1;
    }
    const std::string run_num = argv[1];
    const std::string schedule_dir = "output_schedules";
    const std::string schedule_path = schedule_dir + "/out_" + run_num + ".txt";
    const std::string graph_path = "graphs/wait_for_graph_" + run_num + ".graphml";

    // Read the transactions
    TransactionList transactions;
    transactions.ingest();

    // Write the order the operations will be executed in
    std::error_code ec;
    std::filesystem::create_directories(schedule_dir, ec);   // A failure shows up when the file is opened
    std::ofstream schedule_file(schedule_path);
    if (schedule_file) transactions.print_round_robin(schedule_file);
    schedule_file.close();
    if (schedule_file.fail()){
        std::cerr << "Could not write the schedule to " << schedule_path << std::endl;
        return 1;
    }
    std::cout << "Schedule written to " << schedule_path << std::endl;

    // Execute the schedule, drawing an edge whenever a transaction must wait
    WaitForGraph wait_for_graph(transactions.schedules().size(), transactions);

    // Hand the graph over to the downstream analysis
    if (!wait_for_graph.write_graphml(graph_path)){
        std::cerr << "Could not write the wait-for graph to " << graph_path << std::endl;
        return 1;
    }
    std::cout << "Wait-for graph written to " << graph_path << std::endl;
    return 0;
}
