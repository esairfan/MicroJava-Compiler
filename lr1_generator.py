import json
from collections import defaultdict

# EBNF to BNF without left recursion wasn't needed for LR, but we'll use the same BNF for simplicity
# However, LR(1) handles left recursion perfectly fine. Let's just use the BNF we defined.
from grammar_tool import ll1_grammar, terminals, compute_first

grammar = []
non_terminals = list(ll1_grammar.keys())
nt_to_id = {nt: i for i, nt in enumerate(non_terminals)}
terminals = list(terminals)
terminals.append("$")
term_to_id = {t: i for i, t in enumerate(terminals)}

rules = []
for nt, rhs_list in ll1_grammar.items():
    for rhs in rhs_list:
        rules.append({"lhs": nt, "rhs": rhs})

# We need FIRST sets for sequences
first_sets = compute_first(ll1_grammar)

def first_of_seq(seq, lookahead):
    res = set()
    for sym in seq:
        if sym in terminals:
            res.add(sym)
            break
        else:
            res.update(first_sets[sym] - {""})
            if "" not in first_sets[sym]:
                break
    else:
        res.add(lookahead)
    return res

# Item format: (rule_idx, dot_pos, lookahead)
def closure(items):
    closure_set = set(items)
    changed = True
    while changed:
        changed = False
        new_items = set()
        for rule_idx, dot_pos, lookahead in closure_set:
            rule = rules[rule_idx]
            if dot_pos < len(rule["rhs"]):
                sym = rule["rhs"][dot_pos]
                if sym in non_terminals:
                    lookaheads = first_of_seq(rule["rhs"][dot_pos+1:], lookahead)
                    for r_idx, r in enumerate(rules):
                        if r["lhs"] == sym:
                            for la in lookaheads:
                                if (r_idx, 0, la) not in closure_set and (r_idx, 0, la) not in new_items:
                                    new_items.add((r_idx, 0, la))
                                    changed = True
        closure_set.update(new_items)
    return frozenset(closure_set)

def goto(items, symbol):
    next_items = set()
    for rule_idx, dot_pos, lookahead in items:
        rule = rules[rule_idx]
        if dot_pos < len(rule["rhs"]) and rule["rhs"][dot_pos] == symbol:
            next_items.add((rule_idx, dot_pos + 1, lookahead))
    if not next_items:
        return frozenset()
    return closure(next_items)

# Generate Canonical LR(1) states
start_rule = {"lhs": "S'", "rhs": ["Program"]}
rules.insert(0, start_rule)
# Recalculate nt_to_id
non_terminals.insert(0, "S'")

start_item = (0, 0, "$")
states = [closure([start_item])]
state_to_id = {states[0]: 0}
transitions = {} # (state_id, symbol) -> state_id

queue = [0]
while queue:
    state_id = queue.pop(0)
    state = states[state_id]
    
    symbols_to_process = set()
    for rule_idx, dot_pos, lookahead in state:
        rule = rules[rule_idx]
        if dot_pos < len(rule["rhs"]):
            symbols_to_process.add(rule["rhs"][dot_pos])
            
    for sym in symbols_to_process:
        next_state = goto(state, sym)
        if not next_state:
            continue
        if next_state not in state_to_id:
            state_to_id[next_state] = len(states)
            states.append(next_state)
            queue.append(state_to_id[next_state])
        transitions[(state_id, sym)] = state_to_id[next_state]

# Generate Action and Goto tables
action_table = {} # (state_id, term) -> "S#", "R#", "acc"
goto_table = {} # (state_id, nt) -> state_id

for state_id, state in enumerate(states):
    for rule_idx, dot_pos, lookahead in state:
        rule = rules[rule_idx]
        if dot_pos == len(rule["rhs"]):
            if rule["lhs"] == "S'":
                if lookahead == "$":
                    action_table[(state_id, "$")] = "acc"
            else:
                action = f"R{rule_idx}"
                if (state_id, lookahead) in action_table:
                    if action_table[(state_id, lookahead)] != action:
                        print(f"Reduce-Reduce or Shift-Reduce conflict in state {state_id} on {lookahead}!")
                action_table[(state_id, lookahead)] = action
        else:
            sym = rule["rhs"][dot_pos]
            if sym in terminals:
                if (state_id, sym) in transitions:
                    next_id = transitions[(state_id, sym)]
                    action = f"S{next_id}"
                    if (state_id, sym) in action_table and action_table[(state_id, sym)].startswith("R"):
                        print(f"Shift-Reduce conflict in state {state_id} on {sym}!")
                    action_table[(state_id, sym)] = action

for (state_id, sym), next_id in transitions.items():
    if sym in non_terminals:
        goto_table[(state_id, sym)] = next_id

# Export to JSON
export_data = {
    "num_states": len(states),
    "terminals": terminals,
    "non_terminals": non_terminals,
    "rules": [{"lhs": r["lhs"], "len": len(r["rhs"])} for r in rules],
    "action": [{"state": s, "term": t, "act": a} for (s, t), a in action_table.items()],
    "goto": [{"state": s, "nt": n, "next": g} for (s, n), g in goto_table.items()]
}

with open("lr1_table.json", "w") as f:
    json.dump(export_data, f)

print(f"Generated Canonical LR(1) with {len(states)} states.")
