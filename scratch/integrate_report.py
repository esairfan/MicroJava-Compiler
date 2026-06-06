import re

with open("scratch/compiler_stripped.txt", "r", encoding="utf-8") as f:
    text = f.read()

# Let's find index positions of headers
p1_match = re.search(r"PHASE 1:[^\n]+", text)
p2_match = re.search(r"PHASE 2:[^\n]+", text)
p3_match = re.search(r"PHASE 3:[^\n]+", text)
p4_match = re.search(r"PHASE 4:[^\n]+", text)
p5_match = re.search(r"PHASE 5:[^\n]+", text)

# Helper to clean up any trailing border lines (like equal signs or dashes) from the end of a section
def clean_section(sect_text):
    lines = sect_text.strip().splitlines()
    # Remove lines from the end if they are just separators
    while lines:
        last = lines[-1].strip()
        if not last or all(c in "=-" for c in last):
            lines.pop()
        else:
            break
    # Remove lines from the beginning if they are just separators
    while lines:
        first = lines[0].strip()
        if not first or all(c in "=-" for c in first) or first.startswith("Scanning source file"):
            lines.pop(0)
        else:
            break
    return "\n".join(lines).strip()

# Now let's extract
# Phase 1: starts after the === line under p1 header, ends before p2 header block
p1_start = text.find("\n", p1_match.end())
p1_start = text.find("\n", p1_start + 1)
p2_start_header = text.rfind("===", 0, p2_match.start())
p1_text = clean_section(text[p1_start:p2_start_header])

# Phase 2: starts after the === line under p2 header, ends before p3 header block
p2_start = text.find("\n", p2_match.end())
p2_start = text.find("\n", p2_start + 1)
p3_start_header = text.rfind("===", 0, p3_match.start())
p2_text = clean_section(text[p2_start:p3_start_header])

# Phase 3: starts after the === line under p3 header, ends before p4 header block
p3_start = text.find("\n", p3_match.end())
p3_start = text.find("\n", p3_start + 1)
p4_start_header = text.rfind("===", 0, p4_match.start())
p3_text = clean_section(text[p3_start:p4_start_header])

# Phase 4: starts after the === line under p4 header, ends before p5 header block
p4_start = text.find("\n", p4_match.end())
p4_start = text.find("\n", p4_start + 1)
p5_start_header = text.rfind("===", 0, p5_match.start())
p4_text = clean_section(text[p4_start:p5_start_header])

# Phase 5: starts after the === line under p5 header, ends before the end of the file or SUCCESS line
p5_start = text.find("\n", p5_match.end())
p5_start = text.find("\n", p5_start + 1)
p5_text = clean_section(text[p5_start:])

print(f"P1 len: {len(p1_text)}")
print(f"P2 len: {len(p2_text)}")
print(f"P3 len: {len(p3_text)}")
print(f"P4 len: {len(p4_text)}")
print(f"P5 len: {len(p5_text)}")

# Read report.tex
with open("docs/report.tex", "r", encoding="utf-8") as f:
    report = f.read()

# 1. Remove TikZ flow diagram if present
tikz_pattern = re.compile(
    r"\\subsection\{File Dependencies and Linkage Map\}.*?\\newpage",
    re.DOTALL
)
if tikz_pattern.search(report):
    report = tikz_pattern.sub(r"\\newpage", report)
    print("Removed File Dependencies and Linkage Map flowchart section.")
else:
    print("Flowchart section pattern not found or already removed.")

# Also remove TikZ libraries and styles from preamble if present
report = report.replace("\\usepackage{tikz}", "% \\usepackage{tikz}")
report = report.replace("\\usetikzlibrary{shapes.geometric, arrows, positioning}", "% \\usetikzlibrary{shapes.geometric, arrows, positioning}")

tikz_styles = [
    r"\tikzstyle{startstop}",
    r"\tikzstyle{io}",
    r"\tikzstyle{process}",
    r"\tikzstyle{decision}",
    r"\tikzstyle{arrow}"
]
for style in tikz_styles:
    report = report.replace(style, "% " + style)

# 2. Re-structure Section 7
sect7_start = report.find("\\section{Unified End-to-End Multiphase Execution Trace}")
sect8_start = report.find("\\section{Compiler Error Handling and Halt-on-Error Semantics}")

if sect7_start == -1 or sect8_start == -1:
    raise ValueError("Could not find Section 7 or Section 8 markers in report.tex")

# Construct the new Section 7 content
new_sect7 = f"""\\section{{Unified End-to-End Multiphase Execution Trace}}
This section displays the exact, raw compiler validation outputs for a single valid MicroJava program, demonstrating the integration of all five phases:

\\subsection{{Example MicroJava Source Program}}
Below is the complete MicroJava program (\\texttt{{temp.mj}}) used for generating the execution traces:
\\begin{{lstlisting}}[language=Java, caption=Example MicroJava Program (ArrayMax)]
 program ArrayMax
  int[] numbers;
  int size;
{{
  void main()
    int i, max, val;
  {{
    size = 5;
    numbers = new int[size];
    
    // Read 5 numbers from user input
    i = 0;
    while (i < size) {{
      read(val);
      numbers[i] = val;
      i = i + 1;
    }}
    
    // Find maximum element
    max = numbers[0];
    i = 1;
    while (i < size) {{
      if (numbers[i] > max) {{
        max = numbers[i];
      }}
      i = i + 1;
    }}
    
    print(max);
  }}
}}
\\end{{lstlisting}}

\\subsection{{Phase 1: Lexical Analyzer Output Stream}}
\\begin{{verbatim}}
{p1_text}
\\end{{verbatim}}

\\subsection{{Phase 2: Recursive Descent AST / Parse Tree}}
\\begin{{verbatim}}
{p2_text}
\\end{{verbatim}}

\\subsection{{Phase 3: Symbol Table Registry}}
\\begin{{verbatim}}
{p3_text}
\\end{{verbatim}}

\\subsection{{Phase 4: LL(1) Parsing Rules}}
\\begin{{verbatim}}
{p4_text}
\\end{{verbatim}}

\\subsection{{Phase 5: Canonical LR(1) Actions}}
\\begin{{verbatim}}
{p5_text}
\\end{{verbatim}}

\\newpage
"""

# Replace in report
report_updated = report[:sect7_start] + new_sect7 + report[sect8_start:]

with open("docs/report.tex", "w", encoding="utf-8") as f:
    f.write(report_updated)

print("Successfully updated docs/report.tex with example code and all phase outputs!")
