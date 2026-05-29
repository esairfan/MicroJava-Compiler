#ifndef TABLES_H
#define TABLES_H

#include <string>
#include <vector>
#include <unordered_map>

// LL(1) Grammar Rule
struct LL1Rule {
    std::string lhs;
    std::vector<std::string> rhs;
};

// LL(1) Parser Data
extern std::vector<LL1Rule> ll1_rules;
extern std::unordered_map<std::string, std::unordered_map<std::string, int>> ll1_table;

// LR(1) Parser Data
struct LR1Rule {
    std::string lhs;
    int len;
};

extern std::vector<LR1Rule> lr1_rules;
extern std::unordered_map<int, std::unordered_map<std::string, std::string>> lr1_action;
extern std::unordered_map<int, std::unordered_map<std::string, int>> lr1_goto;

void init_tables();

#endif // TABLES_H
