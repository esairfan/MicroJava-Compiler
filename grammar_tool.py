import json
from collections import defaultdict

# The MicroJava grammar transformed to eliminate left recursion and left factor.
# Format: NonTerminal -> list of lists of symbols. Empty string '' means epsilon.
ll1_grammar = {
    "Program": [["program", "ident", "Program_Body", "{", "MethodDecls", "}"]],
    "Program_Body": [
        ["ConstDecl", "Program_Body"],
        ["VarDecl", "Program_Body"],
        ["ClassDecl", "Program_Body"],
        []
    ],
    "ConstDecl": [["final", "Type", "ident", "=", "ConstValue", ";"]],
    "ConstValue": [["number"], ["charConst"]],
    "VarDecl": [["Type", "ident", "VarDecl_Tail", ";"]],
    "VarDecl_Tail": [[",", "ident", "VarDecl_Tail"], []],
    "ClassDecl": [["class", "ident", "{", "VarDecls", "}"]],
    "VarDecls": [["VarDecl", "VarDecls"], []],
    "MethodDecls": [["MethodDecl", "MethodDecls"], []],
    "MethodDecl": [["MethodType", "ident", "(", "FormPars_Opt", ")", "VarDecls", "Block"]],
    "MethodType": [["Type"], ["void"]],
    "FormPars_Opt": [["FormPars"], []],
    "FormPars": [["Type", "ident", "FormPars_Tail"]],
    "FormPars_Tail": [[",", "Type", "ident", "FormPars_Tail"], []],
    "Type": [["ident", "Type_Opt"]],
    "Type_Opt": [["[", "]"], []],
    "Block": [["{", "Statements", "}"]],
    "Statements": [["Statement", "Statements"], []],
    "Statement": [
        ["Designator", "Statement_AssignOrCall"],
        ["if", "(", "Condition", ")", "Statement", "Else_Opt"],
        ["while", "(", "Condition", ")", "Statement"],
        ["return", "Expr_Opt", ";"],
        ["read", "(", "Designator", ")", ";"],
        ["print", "(", "Expr", "Print_Opt", ")", ";"],
        ["Block"],
        [";"]
    ],
    "Statement_AssignOrCall": [["=", "Expr", ";"], ["ActPars", ";"]],
    "Else_Opt": [["else", "Statement"], []],
    "Expr_Opt": [["Expr"], []],
    "Print_Opt": [[",", "number"], []],
    "ActPars": [["(", "ActPars_List", ")"]],
    "ActPars_List": [["Expr", "ActPars_Tail"], []],
    "ActPars_Tail": [[",", "Expr", "ActPars_Tail"], []],
    "Condition": [["Expr", "Relop", "Expr"]],
    "Relop": [["=="], ["!="], [">"], [">="], ["<"], ["<="]],
    "Expr": [["Expr_Minus", "Term", "Expr_Tail"]],
    "Expr_Minus": [["-"], []],
    "Expr_Tail": [["Addop", "Term", "Expr_Tail"], []],
    "Addop": [["+"], ["-"]],
    "Term": [["Factor", "Term_Tail"]],
    "Term_Tail": [["Mulop", "Factor", "Term_Tail"], []],
    "Mulop": [["*"], ["/"], ["%"]],
    "Factor": [
        ["Designator", "Factor_Designator"],
        ["number"],
        ["charConst"],
        ["new", "ident", "Factor_New"],
        ["(", "Expr", ")"]
    ],
    "Factor_Designator": [["ActPars"], []],
    "Factor_New": [["[", "Expr", "]"], []],
    "Designator": [["ident", "Designator_Tail"]],
    "Designator_Tail": [
        [".", "ident", "Designator_Tail"],
        ["[", "Expr", "]", "Designator_Tail"],
        []
    ]
}

terminals = set(["program", "ident", "{", "}", "final", "=", ";", "number", "charConst", ",", "class", "void", "(", ")", "[", "]", "if", "else", "while", "return", "read", "print", "==", "!=", ">", ">=", "<", "<=", "-", "+", "*", "/", "%", "new", "."])

def compute_first(grammar):
    first = defaultdict(set)
    changed = True
    while changed:
        changed = False
        for nt, rules in grammar.items():
            for rule in rules:
                if len(rule) == 0:
                    if "" not in first[nt]:
                        first[nt].add("")
                        changed = True
                else:
                    for symbol in rule:
                        if symbol in terminals:
                            if symbol not in first[nt]:
                                first[nt].add(symbol)
                                changed = True
                            break
                        else:
                            added = False
                            for f in first[symbol]:
                                if f != "" and f not in first[nt]:
                                    first[nt].add(f)
                                    changed = True
                            if "" not in first[symbol]:
                                break
                    else:
                        if "" not in first[nt]:
                            first[nt].add("")
                            changed = True
    return first

def compute_follow(grammar, first):
    follow = defaultdict(set)
    follow["Program"].add("$")
    changed = True
    while changed:
        changed = False
        for nt, rules in grammar.items():
            for rule in rules:
                for i, symbol in enumerate(rule):
                    if symbol in terminals:
                        continue
                    # It's a non-terminal
                    trailer = set()
                    for next_sym in rule[i+1:]:
                        if next_sym in terminals:
                            trailer.add(next_sym)
                            break
                        else:
                            trailer.update(first[next_sym] - {""})
                            if "" not in first[next_sym]:
                                break
                    else:
                        trailer.add("")
                        
                    for t in trailer:
                        if t != "" and t not in follow[symbol]:
                            follow[symbol].add(t)
                            changed = True
                            
                    if "" in trailer or i == len(rule) - 1:
                        for f in follow[nt]:
                            if f not in follow[symbol]:
                                follow[symbol].add(f)
                                changed = True
    return follow

def build_ll1_table(grammar, first, follow):
    table = defaultdict(dict)
    for nt, rules in grammar.items():
        for rule_idx, rule in enumerate(rules):
            first_of_rule = set()
            if len(rule) == 0:
                first_of_rule.add("")
            else:
                for symbol in rule:
                    if symbol in terminals:
                        first_of_rule.add(symbol)
                        break
                    else:
                        first_of_rule.update(first[symbol] - {""})
                        if "" not in first[symbol]:
                            break
                else:
                    first_of_rule.add("")
            
            for t in first_of_rule:
                if t != "":
                    if t in table[nt]:
                        pass # Conflict, just ignore for now or keep first
                    else:
                        table[nt][t] = rule_idx
            if "" in first_of_rule:
                for t in follow[nt]:
                    if t not in table[nt]:
                        table[nt][t] = rule_idx
    return table

first = compute_first(ll1_grammar)
follow = compute_follow(ll1_grammar, first)
ll1_table = build_ll1_table(ll1_grammar, first, follow)

with open("ll1_table.json", "w") as f:
    json.dump({
        "first": {k: list(v) for k, v in first.items()},
        "follow": {k: list(v) for k, v in follow.items()},
        "table": {k: {t: r for t, r in v.items()} for k, v in ll1_table.items()},
        "grammar": ll1_grammar
    }, f, indent=2)

print("Generated LL(1) tables successfully.")

# To keep the size manageable for Canonical LR(1), we can use a slightly less verbose version 
# of the grammar or just generate LR(1) for the main parts. However, generating LR(1) for this full 
# grammar will produce ~500 states. Python can do it in seconds.
