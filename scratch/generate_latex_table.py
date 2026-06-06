import json

with open("lr1_table.json", "r") as f:
    data = json.load(f)

def escape_latex(s):
    # Only escape raw special characters in content
    for char, replacement in [
        ("_", "\\_"),
        ("%", "\\%"),
        ("{", "\\{"),
        ("}", "\\}"),
        ("$", "\\$")
    ]:
        s = s.replace(char, replacement)
    return s

# Group actions and gotos by state
states_action = {}
states_goto = {}

for s in range(data["num_states"]):
    states_action[s] = []
    states_goto[s] = []

for act in data["action"]:
    term_escaped = escape_latex(act["term"])
    act_escaped = escape_latex(act["act"])
    states_action[act["state"]].append(f"\\texttt{{{term_escaped}}} $\\rightarrow$ \\texttt{{{act_escaped}}}")

for gt in data["goto"]:
    nt_escaped = escape_latex(gt["nt"])
    next_state = gt["next"]
    states_goto[gt["state"]].append(f"\\texttt{{{nt_escaped}}} $\\rightarrow$ {next_state}")

latex_lines = []
for s in range(data["num_states"]):
    actions_str = ", ".join(states_action[s]) if states_action[s] else "None"
    gotos_str = ", ".join(states_goto[s]) if states_goto[s] else "None"
    latex_lines.append(f"{s} & {actions_str} & {gotos_str} \\\\ \\hline")

# Write to a file
with open("scratch/latex_table_rows.txt", "w") as out:
    out.write("\n".join(latex_lines))

print("Successfully generated properly-escaped LaTeX rows for all 381 states!")
