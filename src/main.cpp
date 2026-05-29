#include <iostream>
#include <iomanip>
#include "../include/lexer.h"
#include "../include/symbol_table.h"
#include "../include/parser_rd.h"
#include "../include/parser_ll.h"
#include "../include/parser_lr.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <source_file>\n";
        return 1;
    }

    std::string filename = argv[1];

    // ==========================================
    // PHASE 1: LEXICAL ANALYSIS
    // ==========================================
    std::cout << "\033[1;36m==============================================================================\033[0m\n";
    std::cout << "\033[1;36m                      PHASE 1: LEXICAL ANALYSIS (LEXER)                       \033[0m\n";
    std::cout << "\033[1;36m==============================================================================\033[0m\n";
    std::cout << "Scanning source file: \033[1;33m" << filename << "\033[0m...\n\n";
    std::cout << "+----------------------+----------------------------+---------+---------+\n";
    std::cout << "| Token Type           | Lexeme                     | Line    | Column  |\n";
    std::cout << "+----------------------+----------------------------+---------+---------+\n";

    Lexer lexer_demo(filename);
    Token t;
    int tokenCount = 0;
    int errors_lex = 0;
    do {
        t = lexer_demo.getNextToken();
        std::string typeStr = tokenTypeToString(t.type);
        if (t.type == TokenType::ERROR) {
            errors_lex++;
            // Print error in red
            printf("| \033[1;31m%-20s\033[0m | \033[1;31m%-26s\033[0m | %-7d | %-7d |\n", 
                   typeStr.c_str(), t.lexeme.c_str(), t.line, t.column);
        } else {
            printf("| %-20s | %-26s | %-7d | %-7d |\n", 
                   typeStr.c_str(), t.lexeme.c_str(), t.line, t.column);
        }
        tokenCount++;
    } while (t.type != TokenType::END_OF_FILE);
    std::cout << "+----------------------+----------------------------+---------+---------+\n";
    if (errors_lex == 0) {
        std::cout << "\033[1;32m[SUCCESS] Lexical scan completed with 0 lexical errors. Total tokens: " << tokenCount << "\033[0m\n\n";
    } else {
        std::cout << "\033[1;31m[FAILED] Lexical scan completed with " << errors_lex << " errors. Total tokens: " << tokenCount << "\033[0m\n\n";
    }

    // ==========================================
    // PHASE 2: RECURSIVE DESCENT PARSING (TREE)
    // ==========================================
    std::cout << "\033[1;36m==============================================================================\033[0m\n";
    std::cout << "\033[1;36m                   PHASE 2: RECURSIVE DESCENT PARSE TREE                      \033[0m\n";
    std::cout << "\033[1;36m==============================================================================\033[0m\n";
    Lexer lexer_rd(filename);
    SymbolTable symTable_rd;
    RecursiveDescentParser rdParser(lexer_rd, symTable_rd);
    rdParser.parse();
    int errors_rd = rdParser.getErrorCount();
    std::cout << "------------------------------------------------------------------------------\n";
    if (errors_rd == 0) {
        std::cout << "\033[1;32m[SUCCESS] Recursive Descent Parsing completed with 0 errors.\033[0m\n\n";
    } else {
        std::cout << "\033[1;31m[FAILED] Recursive Descent Parsing finished with " << errors_rd << " errors.\033[0m\n\n";
    }

    // ==========================================
    // PHASE 3: SYMBOL TABLE DUMP
    // ==========================================
    symTable_rd.printTable();

    // ==========================================
    // PHASE 4: LL(1) PREDICTIVE PARSING
    // ==========================================
    std::cout << "\033[1;36m==============================================================================\033[0m\n";
    std::cout << "\033[1;36m                   PHASE 4: LL(1) PREDICTIVE PARSER STEPS                     \033[0m\n";
    std::cout << "\033[1;36m==============================================================================\033[0m\n";
    Lexer lexer_ll(filename);
    SymbolTable symTable_ll;
    LL1Parser llParser(lexer_ll, symTable_ll);
    llParser.parse();
    int errors_ll = llParser.getErrorCount();
    std::cout << "------------------------------------------------------------------------------\n";
    if (errors_ll == 0) {
        std::cout << "\033[1;32m[SUCCESS] LL(1) Parser completed with 0 errors.\033[0m\n\n";
    } else {
        std::cout << "\033[1;31m[FAILED] LL(1) Parser finished with " << errors_ll << " errors.\033[0m\n\n";
    }

    // ==========================================
    // PHASE 5: CANONICAL LR(1) PARSING
    // ==========================================
    std::cout << "\033[1;36m==============================================================================\033[0m\n";
    std::cout << "\033[1;36m                   PHASE 5: CANONICAL LR(1) PARSER ACTIONS                    \033[0m\n";
    std::cout << "\033[1;36m==============================================================================\033[0m\n";
    Lexer lexer_lr(filename);
    SymbolTable symTable_lr;
    LR1Parser lrParser(lexer_lr, symTable_lr);
    lrParser.parse();
    int errors_lr = lrParser.getErrorCount();
    std::cout << "------------------------------------------------------------------------------\n";
    if (errors_lr == 0) {
        std::cout << "\033[1;32m[SUCCESS] Canonical LR(1) Parser completed with 0 errors.\033[0m\n\n";
    } else {
        std::cout << "\033[1;31m[FAILED] Canonical LR(1) Parser finished with " << errors_lr << " errors.\033[0m\n\n";
    }

    return 0;
}
