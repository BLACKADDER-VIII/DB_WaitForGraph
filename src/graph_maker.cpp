#include "graph_maker.hpp"
#include <filesystem>
#include <fstream>

// Creates one lock-table row per data object that appears in any transaction.
// Each row holds one entry per transaction, all starting out as NoLock.
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

// Creates one node per transaction and no edges. Node i stands for
// transaction i (0-based, so T1 is node 0) and sits at position i in nodes().
void WaitForGraph::init_graph(int num_trans){
    for (int i = 0; i<num_trans; i++){
        Node n(i);
        _graph.add_nodes(n);
    }
}

// Returns one queue per transaction, in input order, holding that
// transaction's operations with the next one to execute at the front.
std::vector<std::queue<TransactionOperation>> WaitForGraph::get_sched_qs(TransactionList& ts){
    std::vector<std::queue<TransactionOperation>> sched_qs;
    for (Schedule& sched : ts.schedules()){
        sched_qs.push_back(sched.schedule_stack());
    }
    return sched_qs;
}

// Writes the wait-for graph to the file `path` in GraphML, replacing the file
// if it exists and creating the directory it is in if that is missing.
// Returns false if the file could not be opened or written. With no argument
// the file is graphs/wait_for_graph.graphml, relative to the directory the
// program is run from.
//
// The graph is directed. Each transaction is a node with id T1, T2, ... and
// each edge Tx -> Ty means Tx waits for a lock held by Ty. Every edge has a
// string attribute "transaction_op" holding the operation of Tx that was
// blocked, e.g. "T2:W(a,2)". For the example in Project2.pdf the body is:
//
//     <node id="T1"/> <node id="T2"/> <node id="T3"/>
//     <edge source="T1" target="T2">  transaction_op = T1:W(b,1)
//     <edge source="T2" target="T1">  transaction_op = T2:W(a,2)
//     <edge source="T2" target="T3">  transaction_op = T2:W(a,2)
//     <edge source="T3" target="T2">  transaction_op = T3:W(c,3)
bool WaitForGraph::write_graphml(const std::string& path){
    // Make sure the output directory (graphs/ by default) exists. A failure
    // here is caught just below, when the file cannot be opened.
    std::filesystem::path dir = std::filesystem::path(path).parent_path();
    if (!dir.empty()){
        std::error_code ec;
        std::filesystem::create_directories(dir, ec);
    }
    std::ofstream out(path);
    if (!out) return false;
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
    // One edge per wait. The label needs no XML escaping: an operation string
    // only ever holds letters, digits and the characters : ( ) ,
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
