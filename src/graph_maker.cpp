// Author: Dhroov Pandey
// Last modified: November 28, 2026

#include "graph_maker.hpp"
#include <filesystem>
#include <fstream>

// Inits the lock table; matrix form: data_obj x transactions
void WaitForGraph::init_lock_table(TransactionList& ts){
    int num_trans = ts.schedules().size();
    for (Schedule& sched : ts.schedules()){
        for (TransactionOperation& op : sched.schedule()){
            if (op.opType() == Commit) continue;   // A commit touches no data object
            // emplace leaves the row alone if the object has been seen before
            _lock_table.emplace(op.opObj(), std::vector<LockType>(num_trans, NoLock));
        }
    }
}

// Creates one node per transaction; 0 indexed
void WaitForGraph::init_graph(int num_trans){
    for (int i = 0; i<num_trans; i++){
        Node n(i);
        _graph.add_nodes(n);
    }
}

// Returns one sorted op queue per transaction (order of queues also sorted)
std::vector<std::queue<TransactionOperation>> WaitForGraph::get_sched_qs(TransactionList& ts){
    std::vector<std::queue<TransactionOperation>> sched_qs;
    for (Schedule& sched : ts.schedules()){
        sched_qs.push_back(sched.schedule_stack());
    }
    return sched_qs;
}

// Writes the wait-for graph to path
bool WaitForGraph::write_graphml(const std::string& path){
    // Make sure the output directory (graphs/ by default) exists. If not, it is created
    std::filesystem::path dir = std::filesystem::path(path).parent_path();
    if (!dir.empty()){
        std::error_code ec;
        std::filesystem::create_directories(dir, ec);
    }
    std::ofstream out(path);
    if (!out) return false;     // Returns false if the file could not be opened or written. 
    // Standard XML format header; entering those before node attributes, edge list along with edge attrs are entered.
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        << "<graphml xmlns=\"http://graphml.graphdrawing.org/xmlns\"\n"
        << "         xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n"
        << "         xsi:schemaLocation=\"http://graphml.graphdrawing.org/xmlns"
        << " http://graphml.graphdrawing.org/xmlns/1.0/graphml.xsd\">\n"
        // Declares the edge attribute; edges refer to it by its key id
        << "  <key id=\"transaction_op\" for=\"edge\" attr.name=\"transaction_op\" attr.type=\"string\"/>\n"
        << "  <graph id=\"wait_for_graph\" edgedefault=\"directed\">\n";
    // One node per transaction
    for (const Node& n : _graph.nodes()){
        out << "    <node id=\"T" << n.id()+1 << "\"/>\n";
    }

    for (const Node& n : _graph.nodes()){
        const std::vector<Node*> neighbors = n.neighbors();
        const std::vector<std::string>& labels = n.edge_labels();
        for (size_t k = 0; k<neighbors.size(); k++){
            out << "    <edge source=\"T" << n.id()+1 << "\" target=\"T" << neighbors[k]->id()+1 << "\">\n"
                << "      <data key=\"transaction_op\">" << labels[k] << "</data>\n"
                << "    </edge>\n";
        }
    }
    out << "  </graph>\n"
        << "</graphml>\n";
    out.close();
    return !out.fail();
}
