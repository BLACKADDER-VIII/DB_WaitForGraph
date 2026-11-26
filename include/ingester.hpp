// Author: Dhroov Pandey
// Last modified: November 25, 2026

#pragma once
#include <iostream>
#include <vector>
#include "operation.h"
#include <string>
#include <regex>
#include <queue>

class Schedule{
    public:
    Schedule(){}
    std::vector<TransactionOperation>& schedule();   // Getter function for schedule
    std::queue<TransactionOperation> schedule_stack();  // Return the schedule as a stack
    private:
    std::vector<TransactionOperation> _schedule;
};

class TransactionList{
    public:
    TransactionList(){}
    void ingest();
    std::vector<Schedule>& schedules();
    void print_round_robin(std::ostream& out = std::cout);   // Write the round-robin schedule to out
    private:
    std::vector<Schedule> _schedules;
};