#include "graph.h"
#include "ingester.hpp"
#include <unordered_map>

class WaitForGraph{
    public:
    WaitForGraph(int num_trans, TransactionList& scheds){
        init_lock_table(scheds);
        init_graph(num_trans);
        std::vector<char> wait_q(num_trans, '_');   // '_' represents no resource otherwise waiting for a data_obj
        // Traverse schedule
        std::vector<bool> done(num_trans, false);
        auto check_done = [&done](){
            bool is_done = done[0];
            for (bool b: done)
                is_done = is_done & b;
            return is_done;
        }
        while(!check_done()){
            // Update to use wait-for graph for deadlock detection
            bool did_move = false;
            for (int i = 0; i<num_trans; i++){
                if (wait_q[i] != '_')
                    continue;
                did_move = true;

            }
            if (!did_move)
                break;  // Deadlock
        }
    }
    private:
    Graph _graph;
    std::unordered_map<char, int> _lock_table;
    void init_lock_table(const TransactionList&);
    void init_graph(int);
};