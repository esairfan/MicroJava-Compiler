with open("docs/report.tex", "r") as f:
    tex = f.read()

# 1. Add \usepackage{longtable} to the preamble
if "\\usepackage{longtable}" not in tex:
    tex = tex.replace("\\usepackage{amsmath}", "\\usepackage{amsmath}\n\\usepackage{longtable}")

# 2. Read the generated rows
with open("scratch/latex_table_rows.txt", "r") as f:
    rows = f.read()

# 3. Locate the table subset to replace
start_marker1 = "\\subsection{Action and Goto Tables (States 0--29)}"
start_marker2 = "\\subsection{Action and Goto Tables (All 381 States)}"
end_marker = "\\subsection{Trace Segment for Declaration Statement: \\texttt{int x;}}"

start_idx = tex.find(start_marker1)
if start_idx == -1:
    start_idx = tex.find(start_marker2)

end_idx = tex.find(end_marker)

if start_idx != -1 and end_idx != -1:
    # Build the new section with longtable
    new_section = (
        "\\subsection{Action and Goto Tables (All 381 States)}\n"
        "\\begin{longtable}{|c|p{7.5cm}|p{6.5cm}|}\n"
        "\\hline\n"
        "\\textbf{State} & \\textbf{Action Transitions (Terminal $\\rightarrow$ Action)} & \\textbf{GOTO Transitions (Non-Terminal $\\rightarrow$ State)} \\\\ \\hline\n"
        "\\endfirsthead\n"
        "\\hline\n"
        "\\textbf{State} & \\textbf{Action Transitions (Terminal $\\rightarrow$ Action)} & \\textbf{GOTO Transitions (Non-Terminal $\\rightarrow$ State)} \\\\ \\hline\n"
        "\\endhead\n"
        "\\hline\n"
        "\\endfoot\n"
        "\\hline\n"
        "\\endlastfoot\n"
        f"{rows}\n"
        "\\end{longtable}\n\n"
    )
    
    # Replace the segment
    updated_tex = tex[:start_idx] + new_section + tex[end_idx:]
    
    with open("docs/report.tex", "w") as f:
        f.write(updated_tex)
    print("Successfully updated docs/report.tex with all 381 states in longtable format!")
else:
    print("Error: Could not locate markers in docs/report.tex")
