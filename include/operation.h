// Author: Dhroov Pandey
// Last modified: November 25, 2026

#pragma once
#include <string>

enum OpType {Read, Write, Commit};

class TransactionOperation {
    public:
    TransactionOperation(OpType o, char c, int t, int v=-1): op_type(o), op_obj(c), trans_num(t), op_val(v){}
    OpType opType(){return this->op_type;}
    char opObj(){return this->op_obj;}
    int transNum(){return this->trans_num;}
    int opVal(){return this->op_val;}   // Value in W(x,value); -1 if the op has none
    // The operation as written in the schedule, e.g. "T1:R(a)", "T2:W(a,2)" or
    // "T3:C". Transactions are labelled T1, T2, ... so T1 is transNum() == 0.
    std::string opStr(){
        std::string s = "T" + std::to_string(this->trans_num+1) + ":";
        if (this->op_type == Commit) return s + "C";
        s += (this->op_type == Read)? "R(" : "W(";
        s += this->op_obj;
        if (this->op_val != -1) s += "," + std::to_string(this->op_val);
        return s + ")";
    }

    private:
    OpType op_type=Read;
    char op_obj;
    int trans_num;
    int op_val;
};