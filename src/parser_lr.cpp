#include "../include/parser_lr.h"
#include <iostream>

LR1Parser::LR1Parser(Lexer& lex, SymbolTable& st)
    : lexer(lex), symTable(st), errorCount(0) {
    // Initialize tables
    init_tables();
    advance();
}

void LR1Parser::advance() {
    currentToken = lexer.getNextToken();
    while (currentToken.type == TokenType::ERROR) {
        reportError("Lexical error: " + currentToken.lexeme);
        currentToken = lexer.getNextToken();
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
        
        if (lr1_action.find(state) != lr1_action.end() && lr1_action[state].find(sym) != lr1_action[state].end()) {
            std::string action = lr1_action[state][sym];
            
            if (action == "acc") {
                std::cout << "Action: \033[1;32mACCEPT\033[0m\n";
                break;
            } else if (action[0] == 'S') {
                int next_state = std::stoi(action.substr(1));
                std::cout << "Action: \033[1;34mSHIFT\033[0m to State " << next_state << " (Token: \"" << currentToken.lexeme << "\")\n";
                symbolStack.push_back(sym);
                stateStack.push_back(next_state);
                advance();
            } else if (action[0] == 'R') {
                int rule_idx = std::stoi(action.substr(1));
                const auto& rule = lr1_rules[rule_idx];
                
                std::cout << "Action: \033[1;35mREDUCE\033[0m by rule \033[1;33m" << rule.lhs << "\033[0m (length " << rule.len << ")\n";
                
                for (int i = 0; i < rule.len; ++i) {
                    symbolStack.pop_back();
                    stateStack.pop_back();
                }
                
                symbolStack.push_back(rule.lhs);
                int top_state = stateStack.back();
                
                if (lr1_goto.find(top_state) != lr1_goto.end() && lr1_goto[top_state].find(rule.lhs) != lr1_goto[top_state].end()) {
                    int next_goto = lr1_goto[top_state][rule.lhs];
                    std::cout << "  -> GOTO State " << next_goto << "\n";
                    stateStack.push_back(next_goto);
                } else {
                    reportError("GOTO table missing entry after reduction");
                    break;
                }
            }
        } else {
            // Error
            reportError("Syntax error, unexpected token '" + sym + "'");
            
            // Panic mode recovery for LR: pop states until we find one that can shift the current token
            // In a real compiler we'd look for a state that can shift "error" and then pop tokens.
            // Since we didn't add the "error" non-terminal to the grammar script, we'll just skip the token
            if (currentToken.type == TokenType::END_OF_FILE) {
                break;
            }
            advance();
        }
    }
}
