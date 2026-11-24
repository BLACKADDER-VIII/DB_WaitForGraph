enum OpType {Read, Write, Commit};

class TransactionOperation {
    public:
    TransactionOperation(OpType o, char c, int t, int v=-1): op_type(o), op_obj(c), trans_num(t), op_val(v){}
    OpType opType(){return this->op_type;}
    char opObj(){return this->op_obj;}
    int transNum(){return this->trans_num;}
    int opVal(){return this->op_val;}   // Value in W(x,value); -1 if the op has none

    private:
    OpType op_type=Read;
    char op_obj;
    int trans_num;
    int op_val;
};