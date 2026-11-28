// Author: Dhroov Pandey
// Last modified: November 28, 2026

#include "ingester.hpp"

void TransactionList::ingest(){
    // Read one transaction per line until EOF, blank line, or not 'T' at line start
    std::vector<std::string> transaction_strings;
    std::string line;
    while (std::getline(std::cin, line) && !line.empty() && line[0] == 'T'){
        transaction_strings.push_back(line);
    }
    int num_trans = transaction_strings.size();
    // One schedule per transaction, in input order
    this->_schedules = std::vector<Schedule>(num_trans);
    // Matches R(x), W(x) and W(x,value); captures the op, data obj, and value (if exists)
    std::regex pattern(R"(([RW])\(([a-z]),?([0-9]*)\))");
    for (int i = 0; i<num_trans; i++){
        const std::string& curr_trans_str = transaction_strings[i];
        std::vector<TransactionOperation>& trans_ops = this->_schedules[i].schedule();
        // Collect every read/write on the line in order
        std::sregex_iterator match(curr_trans_str.begin(), curr_trans_str.end(), pattern);
        std::sregex_iterator no_more_matches;
        for (; match != no_more_matches; ++match){
            std::string val_str = (*match)[3].str();
            int val = val_str.empty()? -1 : std::stoi(val_str);
            trans_ops.push_back(TransactionOperation(((*match)[1]=='R')? Read:Write, (*match)[2].str()[0], i, val));
        }
        // Every transaction ends with a commit
        trans_ops.push_back(TransactionOperation(Commit, '_', i));
    }
}

// Prints the round-robin ordering of all operations on one line
void TransactionList::print_round_robin(std::ostream& out){
    // The longest transaction decides how many passes are needed
    size_t max_num_ops = 0;
    for (Schedule& sched : this->_schedules){
        if (sched.schedule().size() > max_num_ops)
            max_num_ops = sched.schedule().size();
    }
    bool first_op = true;
    for (size_t i = 0; i<max_num_ops; i++){
        for (Schedule& sched : this->_schedules){
            std::vector<TransactionOperation>& trans_ops = sched.schedule();
            if (trans_ops.size() <= i)
                continue;   // This transaction has already committed
            TransactionOperation& op = trans_ops[i];
            if (!first_op)
                out << "; ";
            first_op = false;
            out << op.opStr();
        }
    }
    out << std::endl;
}

std::vector<Schedule>& TransactionList::schedules(){
    return this->_schedules;
}

std::vector<TransactionOperation>& Schedule::schedule(){
    return this->_schedule;
}

std::queue<TransactionOperation> Schedule::schedule_stack(){
    std::queue<TransactionOperation> ret;
    for (auto s: this->_schedule){
        ret.push(s);
    }
    return ret;
}
