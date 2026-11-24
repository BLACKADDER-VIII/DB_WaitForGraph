#include <iostream>
#include <vector>
#include "operation.h"
#include <string>
#include <regex>

class Schedule{
    public:
    Schedule(){}
    std::vector<TransactionOperation>& schedule();   // Getter function for schedule
    private:
    std::vector<TransactionOperation> _schedule;
};

class TransactionList{
    public:
    TransactionList(){}
    void ingest();
    std::vector<Schedule>& schedules();
    void print_round_robin();
    private:
    std::vector<Schedule> _schedules;
};