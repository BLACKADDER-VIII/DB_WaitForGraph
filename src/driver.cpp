// Author: Dhroov Pandey
// Last modified: November 28, 2026
//
// Argument: i, a number that suffixes this run's output files
//
// Input (stdin): one transaction per line
//
// Output: (i) Round robin schedules, (ii) wait-for graph in xml, (iii) stdout for every output file path
//
// Exit status: 0 -> success, 1 -> argument invalid/missing or disk write failed.

#include "ingester.hpp"
#include "graph_maker.hpp"
#include <filesystem>
#include <fstream>

// True only if s is a non-empty string of digits
bool is_number(const std::string& s){
    if (s.empty()) return false;
    for (char c : s)
        if (c < '0' || c > '9') return false;
    return true;
}

int main(int argc, char** argv){
    if (argc != 2 || !is_number(argv[1])){
        // Standard usage message
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
