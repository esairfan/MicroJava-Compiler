#include "../include/token.h"

std::string tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::KW_PROGRAM: return "KW_PROGRAM";
        case TokenType::KW_CLASS: return "KW_CLASS";
        case TokenType::KW_IF: return "KW_IF";
        case TokenType::KW_ELSE: return "KW_ELSE";
        case TokenType::KW_WHILE: return "KW_WHILE";
        case TokenType::KW_READ: return "KW_READ";
        case TokenType::KW_PRINT: return "KW_PRINT";
        case TokenType::KW_RETURN: return "KW_RETURN";
        case TokenType::KW_VOID: return "KW_VOID";
        case TokenType::KW_FINAL: return "KW_FINAL";
        case TokenType::KW_NEW: return "KW_NEW";
        case TokenType::PLUS: return "PLUS";
        case TokenType::MINUS: return "MINUS";
        case TokenType::STAR: return "STAR";
        case TokenType::SLASH: return "SLASH";
        case TokenType::PERCENT: return "PERCENT";
        case TokenType::EQ: return "EQ";
        case TokenType::NEQ: return "NEQ";
        case TokenType::GT: return "GT";
        case TokenType::GTE: return "GTE";
        case TokenType::LT: return "LT";
        case TokenType::LTE: return "LTE";
        case TokenType::LPAREN: return "LPAREN";
        case TokenType::RPAREN: return "RPAREN";
        case TokenType::LBRACKET: return "LBRACKET";
        case TokenType::RBRACKET: return "RBRACKET";
        case TokenType::LBRACE: return "LBRACE";
        case TokenType::RBRACE: return "RBRACE";
        case TokenType::ASSIGN: return "ASSIGN";
        case TokenType::SEMICOLON: return "SEMICOLON";
        case TokenType::COMMA: return "COMMA";
        case TokenType::DOT: return "DOT";
        case TokenType::IDENTIFIER: return "IDENTIFIER";
        case TokenType::NUMBER: return "NUMBER";
        case TokenType::CHAR_CONST: return "CHAR_CONST";
        case TokenType::END_OF_FILE: return "EOF";
        case TokenType::ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}
