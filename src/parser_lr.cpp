#include "../include/parser_lr.h"
#include <iostream>

LR1Parser::LR1Parser(Lexer& lex, SymbolTable& st)
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

void LR1Parser::advance() {
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

void LR1Parser::reportError(const std::string& message) {
    std::cerr << "LR(1) Error at line " << currentToken.line << ", col " << currentToken.column << ": " << message << "\n";
    errorCount++;
}

std::string LR1Parser::tokenToGrammarSymbol(const Token& t) {
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

void LR1Parser::parse() {
    stateStack.push_back(0); // Start state
    
    while (true) {
        int state = stateStack.back();
        std::string sym = tokenToGrammarSymbol(currentToken);
        
        // Build stateStack string:
        std::string stackStr = "";
        for (size_t i = 0; i < stateStack.size(); ++i) {
            stackStr += std::to_string(stateStack[i]) + (i + 1 < stateStack.size() ? " " : "");
        }
        
        // Build inputStr:
        std::string inputStr = "";
        for (size_t i = tokenIndex; i < allTokens.size(); ++i) {
            inputStr += allTokens[i].lexeme + (i + 1 < allTokens.size() ? " " : "");
        }
        
        if (lr1_action.find(state) != lr1_action.end() && lr1_action[state].find(sym) != lr1_action[state].end()) {
            std::string action = lr1_action[state][sym];
            
            if (action == "acc") {
                std::cout << "[LR_STEP] Stack: " << stackStr << " | Input: " << inputStr << " | Action: Accept\n";
                break;
            } else if (action[0] == 'S') {
                int next_state = std::stoi(action.substr(1));
                std::cout << "[LR_STEP] Stack: " << stackStr << " | Input: " << inputStr << " | Action: Shift " << next_state << "\n";
                symbolStack.push_back(sym);
                stateStack.push_back(next_state);
                advance();
            } else if (action[0] == 'R') {
                int rule_idx = std::stoi(action.substr(1));
                const auto& rule = lr1_rules[rule_idx];
                
                int top_state_before = stateStack[stateStack.size() - 1 - rule.len];
                
                int next_goto = -1;
                if (lr1_goto.find(top_state_before) != lr1_goto.end() && lr1_goto[top_state_before].find(rule.lhs) != lr1_goto[top_state_before].end()) {
                    next_goto = lr1_goto[top_state_before][rule.lhs];
                }
                
                std::string actionMsg = "Reduce (" + std::to_string(rule_idx) + ") " + rule.lhs + ". Pop " + std::to_string(rule.len) + " state" + (rule.len == 1 ? "" : "s") + ". Push GOTO(" + std::to_string(top_state_before) + ", " + rule.lhs + ") = " + std::to_string(next_goto) + ".";
                
                std::cout << "[LR_STEP] Stack: " << stackStr << " | Input: " << inputStr << " | Action: " << actionMsg << "\n";
                
                for (int i = 0; i < rule.len; ++i) {
                    symbolStack.pop_back();
                    stateStack.pop_back();
                }
                
                symbolStack.push_back(rule.lhs);
                int top_state = stateStack.back();
                
                if (next_goto != -1) {
                    stateStack.push_back(next_goto);
                } else {
                    reportError("GOTO table missing entry after reduction");
                    break;
                }
            }
        } else {
            std::cout << "[LR_STEP] Stack: " << stackStr << " | Input: " << inputStr << " | Action: Syntax error while parsing " << sym << "\n";
            reportError("Syntax error, unexpected token '" + sym + "'");
            if (currentToken.type == TokenType::END_OF_FILE) {
                break;
            }
            advance();
        }
    }
}
