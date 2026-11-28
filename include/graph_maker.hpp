// Author: Dhroov Pandey
// Last modified: November 28, 2026

#pragma once
#include "graph.h"
#include "ingester.hpp"
#include <unordered_map>

enum LockType{NoLock, ReadLock, WriteLock};

const char* const DEFAULT_GRAPHML_PATH = "graphs/wait_for_graph.graphml";

class WaitForGraph{
    public:
    WaitForGraph(int num_trans, TransactionList& scheds){
        init_lock_table(scheds);
        init_graph(num_trans);
        // Traverse schedule
        std::vector<bool> done(num_trans, false);
        std::vector<std::queue<TransactionOperation>> sched_qs = get_sched_qs(scheds);
        auto check_done = [&done](){
            bool is_done = true;
            for (bool b: done)
                is_done = is_done & b;
            return is_done;
        };
        while(!check_done()){
            bool did_move = false;
            for (int i = 0; i<num_trans; i++){
                // Check if transaction waiting for a resource
                std::queue<TransactionOperation>& curr_q = sched_qs[i];
                // if already commited
                if (curr_q.empty()) continue;
                TransactionOperation& curr_op = curr_q.front();
                // check if resource freed
                // Check the type of lock required Read or write
                std::vector<int> lock_holders;
                bool draw_edge = false;
                bool op_done = false;   // Set once curr_op has executed and can leave the queue
                if (curr_op.opType() == Read){
                    // Check lock table for if there are any write-locks
                    std::vector<LockType>& curr_locks = _lock_table.at(curr_op.opObj());
                    bool is_avail = true;
                    for (int j = 0; j<num_trans; j++){
                        if (j == i) continue;
                        if (curr_locks[j] == WriteLock) {is_avail = false; lock_holders.push_back(j);}
                    }
                    if (is_avail){
                        _lock_table[curr_op.opObj()][i] = (_lock_table[curr_op.opObj()][i] == WriteLock)? WriteLock : ReadLock;
                        op_done = true;
                    }
                    else draw_edge = true;
                }

                if (curr_op.opType() == Write) {
                    // Check if any locks exist
                    std::vector<LockType>& curr_locks = _lock_table.at(curr_op.opObj());
                    bool is_avail = true;
                    for (int j = 0; j<num_trans; j++){
                        if (j == i) continue;
                        if (curr_locks[j] != NoLock) {is_avail = false; lock_holders.push_back(j);}
                        }
                    if (is_avail){
                        _lock_table[curr_op.opObj()][i] = WriteLock;
                        op_done = true;
                    }
                    else draw_edge = true;
                }

                if (curr_op.opType() == Commit) {
                    // Release all locks
                    for (auto& kvp : _lock_table){
                        kvp.second[i] = NoLock;
                    }
                    // remove in-edges to i
                    _graph.remove_edges_to(i);
                    done[i] = true;
                    op_done = true;
                }
                if (draw_edge){
                    // Waiting: draw edge from i to all lock holders
                    for (int holder : lock_holders)
                        _graph.add_edge(i, holder, curr_op.opStr());
                }
                if (op_done){
                    // Pop last: curr_op refers to the front of the queue
                    curr_q.pop();
                    did_move = true;
                }
            }
            if (!did_move)
                break;  // Deadlock
        }
    }
    Graph& graph(){return this->_graph;}   // Getter function for the wait-for graph
    // Write the wait-for graph to a GraphML file; false if the file could not be written
    bool write_graphml(const std::string& path = DEFAULT_GRAPHML_PATH);
    private:
    Graph _graph;
    std::unordered_map<char, std::vector<LockType>> _lock_table;  // Maps data obj -> array of transaction locks: 0 for not lock, 1 for read lock, 2 for write lock
    void init_lock_table(TransactionList&);
    void init_graph(int);
    std::vector<std::queue<TransactionOperation>> get_sched_qs(TransactionList&);
};
