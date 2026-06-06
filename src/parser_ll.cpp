#include "../include/parser_ll.h"
#include <iostream>

LL1Parser::LL1Parser(Lexer& lex, SymbolTable& st)
    : lexer(lex), symTable(st), errorCount(0) {
    init_tables();
    Token t;
    do {
        t = lexer.getNextToken();
        allTokens.push_back(t);
    } while (t.type != TokenType::END_OF_FILE);
    tokenIndex = 0;
    currentToken = allTokens[0];
}

void LL1Parser::advance() {
    if (tokenIndex < allTokens.size() - 1) {
        tokenIndex++;
        currentToken = allTokens[tokenIndex];
        while (currentToken.type == TokenType::ERROR && tokenIndex < allTokens.size() - 1) {
            reportError("Lexical error: " + currentToken.lexeme);
            tokenIndex++;
            currentToken = allTokens[tokenIndex];
        }
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

        // Get Stack Representation
        std::stack<std::string> tempStack = parseStack;
        std::vector<std::string> stackElems;
        while (!tempStack.empty()) {
            stackElems.push_back(tempStack.top());
            tempStack.pop();
        }
        std::string stackStr = "";
        for (auto it = stackElems.rbegin(); it != stackElems.rend(); ++it) {
            stackStr += *it + (it + 1 != stackElems.rend() ? " " : "");
        }

        // Get Input Buffer Representation
        std::string inputStr = "";
        for (size_t i = tokenIndex; i < allTokens.size(); ++i) {
            inputStr += allTokens[i].lexeme + (i + 1 < allTokens.size() ? " " : "");
        }

        if (top == sym) {
            std::cout << "[LL_STEP] Stack: " << stackStr << " | Input: " << inputStr << " | Action: Match token '" << sym << "'\n";
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
            std::cout << "[LL_STEP] Stack: " << stackStr << " | Input: " << inputStr << " | Action: Mismatch Error: Expected '" << top << "' but found '" << sym << "'\n";
            reportError("Expected '" + top + "' but found '" + sym + "'");
            parseStack.pop();
        } else {
            if (ll1_table.find(top) != ll1_table.end() && ll1_table[top].find(sym) != ll1_table[top].end()) {
                int rule_idx = ll1_table[top][sym];
                const auto& rule = ll1_rules[rule_idx];
                
                std::string ruleStr = rule.lhs + " -> ";
                if (rule.rhs.empty()) {
                    ruleStr += "epsilon";
                } else {
                    for (const auto& r_sym : rule.rhs) {
                        ruleStr += r_sym + " ";
                    }
                }
                std::cout << "[LL_STEP] Stack: " << stackStr << " | Input: " << inputStr << " | Action: Applied Rule " << ruleStr << "\n";
                
                parseStack.pop();
                for (auto it = rule.rhs.rbegin(); it != rule.rhs.rend(); ++it) {
                    parseStack.push(*it);
                }
            } else {
                std::cout << "[LL_STEP] Stack: " << stackStr << " | Input: " << inputStr << " | Action: Syntax error while parsing " << top << "\n";
                reportError("Syntax error, unexpected token '" + sym + "' while parsing " + top);
                if (sym == "$") {
                    parseStack.pop();
                } else {
                    advance();
                }
            }
        }
    }
}
