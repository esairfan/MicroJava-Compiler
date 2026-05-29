#include "../include/parser_ll.h"
#include <iostream>

LL1Parser::LL1Parser(Lexer& lex, SymbolTable& st)
    : lexer(lex), symTable(st), errorCount(0) {
    // Initialize tables
    init_tables();
    advance();
}

void LL1Parser::advance() {
    currentToken = lexer.getNextToken();
    while (currentToken.type == TokenType::ERROR) {
        reportError("Lexical error: " + currentToken.lexeme);
        currentToken = lexer.getNextToken();
    }
}

void LL1Parser::reportError(const std::string& message) {
    std::cerr << "LL(1) Error at line " << currentToken.line << ", col " << currentToken.column << ": " << message << "\n";
    errorCount++;
}

std::string LL1Parser::tokenToGrammarSymbol(const Token& t) {
    switch (t.type) {
        case TokenType::KW_PROGRAM: return "program";
        case TokenType::KW_CLASS: return "class";
        case TokenType::KW_IF: return "if";
        case TokenType::KW_ELSE: return "else";
        case TokenType::KW_WHILE: return "while";
        case TokenType::KW_READ: return "read";
        case TokenType::KW_PRINT: return "print";
        case TokenType::KW_RETURN: return "return";
        case TokenType::KW_VOID: return "void";
        case TokenType::KW_FINAL: return "final";
        case TokenType::KW_NEW: return "new";
        case TokenType::PLUS: return "+";
        case TokenType::MINUS: return "-";
        case TokenType::STAR: return "*";
        case TokenType::SLASH: return "/";
        case TokenType::PERCENT: return "%";
        case TokenType::EQ: return "==";
        case TokenType::NEQ: return "!=";
        case TokenType::GT: return ">";
        case TokenType::GTE: return ">=";
        case TokenType::LT: return "<";
        case TokenType::LTE: return "<=";
        case TokenType::LPAREN: return "(";
        case TokenType::RPAREN: return ")";
        case TokenType::LBRACKET: return "[";
        case TokenType::RBRACKET: return "]";
        case TokenType::LBRACE: return "{";
        case TokenType::RBRACE: return "}";
        case TokenType::ASSIGN: return "=";
        case TokenType::SEMICOLON: return ";";
        case TokenType::COMMA: return ",";
        case TokenType::DOT: return ".";
        case TokenType::IDENTIFIER: return "ident";
        case TokenType::NUMBER: return "number";
        case TokenType::CHAR_CONST: return "charConst";
        case TokenType::END_OF_FILE: return "$";
        default: return "";
    }
}

void LL1Parser::parse() {
    parseStack.push("$");
    parseStack.push("Program");

    while (!parseStack.empty()) {
        std::string top = parseStack.top();
        std::string sym = tokenToGrammarSymbol(currentToken);

        if (top == sym) {
            parseStack.pop();
            if (sym != "$") {
                advance();
            }
        } else if (top == "$" || top == "ident" || top == "number" || top == "charConst" || 
                   top == "program" || top == "class" || top == "if" || top == "else" || 
                   top == "while" || top == "read" || top == "print" || top == "return" || 
                   top == "void" || top == "final" || top == "new" || top == "+" || 
                   top == "-" || top == "*" || top == "/" || top == "%" || top == "==" || 
                   top == "!=" || top == ">" || top == ">=" || top == "<" || top == "<=" || 
                   top == "(" || top == ")" || top == "[" || top == "]" || top == "{" || 
                   top == "}" || top == "=" || top == ";" || top == "," || top == ".") {
            // It's a terminal but doesn't match
            reportError("Expected '" + top + "' but found '" + sym + "'");
            // Panic mode recovery: pop stack
            parseStack.pop();
        } else {
            // It's a non-terminal
            if (ll1_table.find(top) != ll1_table.end() && ll1_table[top].find(sym) != ll1_table[top].end()) {
                int rule_idx = ll1_table[top][sym];
                parseStack.pop();
                
                const auto& rule = ll1_rules[rule_idx];
                
                std::cout << "Applied Rule: \033[1;35m" << rule.lhs << "\033[0m -> ";
                if (rule.rhs.empty()) {
                    std::cout << "\033[3mepsilon\033[0m";
                } else {
                    for (const auto& r_sym : rule.rhs) {
                        std::cout << r_sym << " ";
                    }
                }
                std::cout << "\n";

                // Push right hand side in reverse order
                for (auto it = rule.rhs.rbegin(); it != rule.rhs.rend(); ++it) {
                    parseStack.push(*it);
                }
            } else {
                reportError("Syntax error, unexpected token '" + sym + "' while parsing " + top);
                // Panic mode recovery: skip token or pop stack if at EOF to avoid infinite loop
                if (sym == "$") {
                    parseStack.pop();
                } else {
                    advance();
                }
            }
        }
    }
}
