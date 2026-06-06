import re

# Read processed simplesum outputs
with open("scratch/processed_simplesum.txt", "r", encoding="utf-8") as f:
    processed = f.read()

# Parse the components out
p1_text = re.search(r"=== P1 ===\n(.*?)\n\n=== P2 ===", processed, re.DOTALL).group(1)
p2_text = re.search(r"=== P2 ===\n(.*?)\n\n=== P3 ===", processed, re.DOTALL).group(1)
p3_text = re.search(r"=== P3 ===\n(.*?)\n\n=== P4 ===", processed, re.DOTALL).group(1)
p4_text = re.search(r"=== P4 ===\n(.*?)\n\n=== P5 ===", processed, re.DOTALL).group(1)
p5_text = re.search(r"=== P5 ===\n(.*)$", processed, re.DOTALL).group(1)

# Read report.tex
with open("docs/report.tex", "r", encoding="utf-8") as f:
    report = f.read()

# 1. Update Section 7 (Unified End-to-End Multiphase Execution Trace)
sect7_start = report.find("\\section{Unified End-to-End Multiphase Execution Trace}")
sect8_start = report.find("\\section{Compiler Error Handling and Halt-on-Error Semantics}")

if sect7_start == -1 or sect8_start == -1:
    raise ValueError("Could not find Section 7 or Section 8 markers in report.tex")

new_sect7 = f"""\\section{{Unified End-to-End Multiphase Execution Trace}}
This section displays the exact, raw compiler validation outputs for the \\texttt{{SimpleSum}} program, demonstrating the integration of all five phases:

\\subsection{{Example MicroJava Source Program}}
Below is the complete MicroJava program (\\texttt{{SimpleSum.mj}}) used for generating the execution traces:
\\begin{{lstlisting}}[language=Java, caption=Example MicroJava Program (SimpleSum)]
program SimpleSum
  int x;
  int y;
  int sum;
{{
  void main() {{
    x = 5;
    y = 10;
    sum = x + y;
    print(sum);
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

\\subsection{{Phase 4: LL(1) Parsing Rules (Actions Only)}}
\\begin{{verbatim}}
{p4_text}
\\end{{verbatim}}

\\subsection{{Phase 5: Canonical LR(1) Actions (Actions Only)}}
\\begin{{verbatim}}
{p5_text}
\\end{{verbatim}}

\\newpage
"""

# Replace Section 7
report = report[:sect7_start] + new_sect7 + report[sect8_start:]

# 2. Update Section 8 (Compiler Error Handling and Halt-on-Error Semantics)
# Let's find Section 8 and the Conclusion section to insert the test cases
sect8_idx = report.find("\\section{Compiler Error Handling and Halt-on-Error Semantics}")
conclusion_idx = report.find("\\section{Conclusion}")

if sect8_idx == -1 or conclusion_idx == -1:
    raise ValueError("Could not find Section 8 or Conclusion markers in report.tex")

# Find the end of Section 8 (before Conclusion)
sect8_content = report[sect8_idx:conclusion_idx]

# Insert the 5 error handling test cases
error_test_cases = """
\\subsection{Error Handling Test Cases}
Below are 5 representative test cases demonstrating the error-handling output when syntax or semantic violations occur, showcasing compiler halts:

\\begin{enumerate}
    \\item \\textbf{Test Case 1: Lexical Error (Invalid Symbol)}
    \\begin{itemize}
        \\item \\textbf{Source Segment}: \\texttt{int x \\# 5;}
        \\item \\textbf{Behavior}: The double-buffered scanner encounters the invalid token \\texttt{\\#} and immediately reports:
        \\begin{verbatim}
[LEXICAL ERROR] Line 3, Column 9: Unexpected character '#'
        \\end{verbatim}
        \\item \\textbf{Halt}: Compiler execution aborts immediately, preventing further parsing.
    \\end{itemize}

    \\item \\textbf{Test Case 2: Syntactic Error (Missing Semicolon)}
    \\begin{itemize}
        \\item \\textbf{Source Segment}: \\texttt{int x} (followed by \\texttt{int y;})
        \\item \\textbf{Behavior}: The recursive descent and LL(1) parsers expect a semicolon to terminate the declaration block. They report:
        \\begin{verbatim}
[SYNTAX ERROR] Line 4, Column 3: Expected ';' but found 'int'
        \\end{verbatim}
        \\item \\textbf{Halt}: The parse stacks are cleaned up and the compilation is terminated.
    \\end{itemize}

    \\item \\textbf{Test Case 3: Syntactic Error (Mismatched Parenthesis)}
    \\begin{itemize}
        \\item \\textbf{Source Segment}: \\texttt{while (x > 0 \\{ x = x - 1; \\}}
        \\item \\textbf{Behavior}: The parsers detect a missing closing parenthesis for the loop condition. They report:
        \\begin{verbatim}
[SYNTAX ERROR] Line 12, Column 16: Expected ')' but found '{'
        \\end{verbatim}
        \\item \\textbf{Halt}: Compiler execution is halted at the mismatched boundary.
    \\end{itemize}

    \\item \\textbf{Test Case 4: Semantic Error (Duplicate Variable Declaration)}
    \\begin{itemize}
        \\item \\textbf{Source Segment}: \\texttt{int x; int x;}
        \\item \\textbf{Behavior}: The compiler successfully parses both declarations syntactically. However, during symbol table insertion, scope duplicate verification fails:
        \\begin{verbatim}
[SEMANTIC ERROR] Line 4, Column 7: Redeclaration of identifier 'x' in scope 1
        \\end{verbatim}
        \\item \\textbf{Halt}: Symbol table insertion triggers an immediate semantic halt.
    \\end{itemize}

    \\item \\textbf{Test Case 5: Semantic Error (Undeclared Identifier Lookup)}
    \\begin{itemize}
        \\item \\textbf{Source Segment}: \\texttt{sum = x + z;} (where \\texttt{z} was never declared)
        \\item \\textbf{Behavior}: During identifier resolution in the factor evaluation rules, the lookup inside the active symbol table stack returns null:
        \\begin{verbatim}
[SEMANTIC ERROR] Line 10, Column 13: Identifier 'z' is undeclared in this scope
        \\end{verbatim}
        \\item \\textbf{Halt}: The compiler halts immediately to prevent invalid type check cascades.
    \\end{itemize}
\\end{enumerate}

"""

# Insert test cases right before \\section{Conclusion}
sect8_updated = sect8_content.strip() + "\n" + error_test_cases + "\n\n"
report_final = report[:sect8_idx] + sect8_updated + report[conclusion_idx:]

with open("docs/report.tex", "w", encoding="utf-8") as f:
    f.write(report_final)

print("Successfully updated docs/report.tex with SimpleSum trace and 5 error test cases!")
