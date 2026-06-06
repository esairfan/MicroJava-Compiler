import json

with open("lr1_table.json", "r") as f:
    data = json.load(f)

action_map = {}
for entry in data["action"]:
    state = entry["state"]
    term = entry["term"]
    act = entry["act"]
    if state not in action_map:
        action_map[state] = []
    action_map[state].append((term, act))

goto_map = {}
for entry in data["goto"]:
    state = entry["state"]
    nt = entry["nt"]
    next_state = entry["next"]
    if state not in goto_map:
        goto_map[state] = []
    goto_map[state].append((nt, next_state))

# Generate LaTeX table for the first 30 states
latex_str = "\\begin{tabular}{|c|p{7cm}|p{6cm}|}\n\\hline\n"
latex_str += "\\textbf{State} & \\textbf{Action Transitions (Terminal $\\rightarrow$ Action)} & \\textbf{GOTO Transitions (Non-Terminal $\\rightarrow$ State)} \\\\ \\hline\n"

for state in range(30):
    actions = action_map.get(state, [])
    gotos = goto_map.get(state, [])
    
    act_parts = []
    for term, act in sorted(actions):
        # Escape LaTeX special chars
        term_esc = term.replace("_", "\\_").replace("$", "\\$")
        act_parts.append(f"\\texttt{{{term_esc}}} $\\rightarrow$ \\texttt{{{act}}}")
    act_str = ", ".join(act_parts) if act_parts else "None"
    
    goto_parts = []
    for nt, next_state in sorted(gotos):
        nt_esc = nt.replace("_", "\\_")
        goto_parts.append(f"\\texttt{{{nt_esc}}} $\\rightarrow$ {next_state}")
    goto_str = ", ".join(goto_parts) if goto_parts else "None"
    
    latex_str += f"{state} & {act_str} & {goto_str} \\\\ \\hline\n"

latex_str += "\\end{tabular}\n"

with open("scratch/lr_tables_insert.txt", "w") as f:
    f.write(latex_str)

print("Generated LR tables insert successfully.")
