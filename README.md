# MicroJava Compiler & Interactive Web Playground

A complete, multi-stage compiler playground for the MicroJava language. It features a robust C++17 syntax validator backend, a multithreaded Python server, and a sleek, interactive dark-theme browser dashboard featuring a real-time MicroJava terminal runner.

---

## Key Features

1. **5-Phase C++ Compiler Engine**:
   * **Lexer**: Uses double-buffering for speed, tracks lines/columns, and generates clean token streams.
   * **Recursive Descent Parser**: Evaluates inputs recursively and dumps a graphical ASCII tree of the grammar nesting.
   * **Symbol Table**: Manages linked scope levels, tracking declarations, types, and variables.
   * **LL(1) Top-Down Parser**: Validates grammar predicting transitions from preloaded tables.
   * **Canonical LR(1) Bottom-Up Parser**: Performs state-machine shift/reduce actions (over 380 states).
2. **Interactive Terminal Playground**:
   * If compilation succeeds, the client-side JavaScript engine transpiles the compiler token stream into modern asynchronous JavaScript on-the-fly.
   * Runs the transpiled code inside a simulated, full-width Linux-style console that pauses dynamically for input on `read()` and prints values inline on `print()`.
   * Safeguarded against infinite loops via iteration-yielding (`yieldCheck`), preventing window freeze.
3. **Responsive Glassmorphism GUI**:
   * Dual-panel design: Code editor (with matching sidebar line numbering) on the left, compilation results tabs on the right.
   * Sliding error console providing custom guides and context-aware bug-fixing hints.

---

## Directory Structure

* `src/` - C++ compiler source files (`main.cpp`, `lexer.cpp`, `parser_rd.cpp`, `parser_ll.cpp`, `parser_lr.cpp`, `symbol_table.cpp`, `tables.cpp`).
* `include/` - C++ compiler header definitions.
* `gui/` - Web playground assets (`index.html`, `style.css`, `app.js`).
* `docs/` - Academic reporting directory, including the comprehensive 500+ line project details report (`report.tex`).
* `test/` - Sample MicroJava test programs (`sample.mj`, `temp.mj`, `simplesum.mj`).
* `server.py` - Threaded HTTP server handler.
* `grammar_tool.py` - Script generating LL(1) parse tables.
* `lr1_generator.py` - Script computing LR(1) state transitions.
* `table_exporter.py` - Exporter compiling JSON tables directly to `src/tables.cpp`.

---

## Quick Start & Build Instructions

### Prerequisites
* Python 3.x
* C++17 Compiler (g++, clang++, etc.)
* GNU Make or MinGW (optional)

### Build Steps

1. **Compile the C++ compiler binary**:
   ```bash
   g++ -std=c++17 -Wall -Iinclude src/*.cpp -o minicompiler.exe
   ```
   *(Or run `make` if Makefile support is configured on your environment)*

2. **Start the Web Playground Server**:
   ```bash
   python server.py
   ```

3. **Access the Playground**:
   Open your browser and navigate to:
   👉 **[http://localhost:8000](http://localhost:8000)**

---

## Documentation & Report
For details on compiler component linkages, EBNF schemas, parse table mapping, and end-to-end execution traces of the 12-line `SimpleSum` example program, view the LaTeX documentation source at:
👉 **[docs/report.tex](file:///c:/MAD%20projects/Compiler-Project/compiler report.pdf)**

## Key Bug Fixes & Refinements
* **Parser Recovery**: Fixed an infinite loop in `parser_ll.cpp` at EOF by popping the stack if the lookup table misses at unexpected EOF.
* **Vertical Alignment**: Fixed a sidebar numbering drift in `style.css` by locking both the gutter and textarea line heights to exactly `1.5rem`.
* **Multi-threading**: Upgraded the Python server in `server.py` to be multi-threaded, preventing Keep-Alive browser requests from blocking connections.
* **Action-Only Trace Outputs**: Simplified LL(1) and LR(1) parser traces in the report to display action steps only, making execution traces clear and readable.
* **Halt-on-Error Validation**: Added 5 detailed syntax and semantic error test cases showcasing coordinate recovery and graceful halt-on-error compilation.

