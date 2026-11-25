// Driver: reads the transactions, prints the round-robin schedule and writes
// the wait-for graph for the downstream analysis.
//
// Input (stdin): one transaction per line, in the format of Project2.pdf:
//
//     T1: R(a); W(b,1); C
//     T2: R(b); R(c); W(a,2); C
//     T3: R(a); W(c,3); C
//
// Output:
//   - stdout: the round-robin ordering of the operations ("Output 1"),
//     followed by a line saying where the graph was written.
//   - graphs/wait_for_graph.graphml: the wait-for graph in GraphML, with one
//     node per transaction (T1, T2, ...) and a "transaction_op" attribute on
//     every edge. The path is relative to the directory the program is run
//     from, so run it from the project root.
//
// Exit status is 0 on success, 1 if the graph file could not be written.
//
// Build and run from the project root:
//     c++ -std=c++17 -Iinclude src/driver.cpp src/graph_maker.cpp src/ingester.cpp -o wait_for_graph
//     ./wait_for_graph < input.txt

#include "ingester.hpp"
#include "graph_maker.hpp"

int main(){
    // Read the transactions and print the order they will be executed in
    TransactionList transactions;
    transactions.ingest();
    transactions.print_round_robin();

    // Execute the schedule, drawing an edge whenever a transaction must wait
    WaitForGraph wait_for_graph(transactions.schedules().size(), transactions);

    // Hand the graph over to the downstream analysis
    if (!wait_for_graph.write_graphml()){
        std::cerr << "Could not write the wait-for graph to " << DEFAULT_GRAPHML_PATH << std::endl;
        return 1;
    }
    std::cout << "Wait-for graph written to " << DEFAULT_GRAPHML_PATH << std::endl;
    return 0;
}
