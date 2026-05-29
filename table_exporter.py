import json

# Read LL(1) table
with open("ll1_table.json", "r") as f:
    ll1_data = json.load(f)

# Read LR(1) table
with open("lr1_table.json", "r") as f:
    lr1_data = json.load(f)

header_content = """#ifndef TABLES_H
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
"""

with open("include/tables.h", "w") as f:
    f.write(header_content)

cpp_content = """#include "../include/tables.h"

std::vector<LL1Rule> ll1_rules;
std::unordered_map<std::string, std::unordered_map<std::string, int>> ll1_table;

std::vector<LR1Rule> lr1_rules;
std::unordered_map<int, std::unordered_map<std::string, std::string>> lr1_action;
std::unordered_map<int, std::unordered_map<std::string, int>> lr1_goto;

void init_tables() {
"""

# Output LL(1) rules
ll1_rules = []
for nt, rules in ll1_data["grammar"].items():
    for rule in rules:
        ll1_rules.append((nt, rule))

for i, (nt, rule) in enumerate(ll1_rules):
    cpp_content += f'    ll1_rules.push_back({{"{nt}", {{'
    cpp_content += ', '.join(f'"{sym}"' for sym in rule)
    cpp_content += '}});\n'

cpp_content += '\n'

# Output LL(1) table
# To map from ll1_data["table"] rule index to our flattened list
# We need to map (nt, rule_idx) -> flat_idx
flat_idx = 0
nt_to_flat = {}
for nt, rules in ll1_data["grammar"].items():
    for i in range(len(rules)):
        nt_to_flat[(nt, i)] = flat_idx
        flat_idx += 1

for nt, lookaheads in ll1_data["table"].items():
    for t, rule_idx in lookaheads.items():
        if t == "": t = "$"
        flat_id = nt_to_flat[(nt, rule_idx)]
        cpp_content += f'    ll1_table["{nt}"]["{t}"] = {flat_id};\n'

cpp_content += '\n'

# Output LR(1) rules
for r in lr1_data["rules"]:
    cpp_content += f'    lr1_rules.push_back({{"{r["lhs"]}", {r["len"]}}});\n'

cpp_content += '\n'

# Output LR(1) Action table
for a in lr1_data["action"]:
    cpp_content += f'    lr1_action[{a["state"]}]["{a["term"]}"] = "{a["act"]}";\n'

cpp_content += '\n'

# Output LR(1) Goto table
for g in lr1_data["goto"]:
    cpp_content += f'    lr1_goto[{g["state"]}]["{g["nt"]}"] = {g["next"]};\n'

cpp_content += "}\n"

with open("src/tables.cpp", "w") as f:
    f.write(cpp_content)

print("Exported C++ tables to include/tables.h and src/tables.cpp")
