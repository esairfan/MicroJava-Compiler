import json

with open("ll1_table.json", "r") as f:
    ll1_data = json.load(f)

first = ll1_data["first"]
follow = ll1_data["follow"]

def escape_latex(s):
    if not s:
        return "$\\epsilon$"
    # Escape LaTeX special chars
    s = s.replace("_", "\\_")
    if s == "{":
        return "\\texttt{\\{}"
    if s == "}":
        return "\\texttt{\\}}"
    return f"\\texttt{{{s}}}"

# Generate LaTeX for FIRST sets
first_latex = "\\begin{tabular}{|l|p{10cm}|}\n\\hline\n\\textbf{Non-Terminal} & \\textbf{FIRST Set} \\\\ \\hline\n"
for nt in sorted(first.keys()):
    items = ", ".join(escape_latex(x) for x in sorted(first[nt]))
    first_latex += f"\\texttt{{{nt.replace('_', '\\_')}}} & {items} \\\\ \\hline\n"
first_latex += "\\end{tabular}"

# Generate LaTeX for FOLLOW sets
follow_latex = "\\begin{tabular}{|l|p{10cm}|}\n\\hline\n\\textbf{Non-Terminal} & \\textbf{FOLLOW Set} \\\\ \\hline\n"
for nt in sorted(follow.keys()):
    items = ", ".join(escape_latex(x) for x in sorted(follow[nt]))
    follow_latex += f"\\texttt{{{nt.replace('_', '\\_')}}} & {items} \\\\ \\hline\n"
follow_latex += "\\end{tabular}"

# Generate representative LL(1) table entries (first 10 non-terminals to avoid huge file)
ll1_table_data = ll1_data["table"]
ll1_latex = "\\begin{tabular}{|l|l|p{8cm}|}\n\\hline\n\\textbf{Non-Terminal} & \\textbf{Input Token} & \\textbf{Applied Rule} \\\\ \\hline\n"

count = 0
for nt in sorted(ll1_table_data.keys()):
    if count >= 15:
        break
    count += 1
    for t in sorted(ll1_table_data[nt].keys()):
        rule_idx = ll1_table_data[nt][t]
        rule = ll1_data["grammar"][nt][rule_idx]
        rule_str = f"{nt} \\rightarrow " + (" ".join(rule) if rule else "\\epsilon")
        rule_str = rule_str.replace("_", "\\_").replace("{", "\\{").replace("}", "\\}")
        ll1_latex += f"\\texttt{{{nt.replace('_', '\\_')}}} & \\texttt{{{t}}} & ${rule_str}$ \\\\ \\hline\n"
ll1_latex += "\\end{tabular}"

# Save to output file
with open("scratch/report_inserts.txt", "w") as f:
    f.write("=== FIRST SETS ===\n")
    f.write(first_latex)
    f.write("\n\n=== FOLLOW SETS ===\n")
    f.write(follow_latex)
    f.write("\n\n=== LL1 TABLE ===\n")
    f.write(ll1_latex)

print("Generated inserts successfully.")
