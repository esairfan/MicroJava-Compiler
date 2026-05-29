#ifndef TOKEN_H
#define TOKEN_H

#include <string>

enum class TokenType {
    // Keywords
    KW_PROGRAM, KW_CLASS, KW_IF, KW_ELSE, KW_WHILE,
    KW_READ, KW_PRINT, KW_RETURN, KW_VOID, KW_FINAL, KW_NEW,

    // Operators and Punctuation
    PLUS, MINUS, STAR, SLASH, PERCENT,          // + - * / %
    EQ, NEQ, GT, GTE, LT, LTE,                  // == != > >= < <=
    LPAREN, RPAREN, LBRACKET, RBRACKET,         // ( ) [ ]
    LBRACE, RBRACE,                             // { }
    ASSIGN, SEMICOLON, COMMA, DOT,              // = ; , .

    // Identifiers and Constants
    IDENTIFIER, NUMBER, CHAR_CONST,

    // Special
    END_OF_FILE, ERROR
};

struct Token {
    TokenType type;
    std::string lexeme;
    int line;
    int column;

    Token() : type(TokenType::ERROR), lexeme(""), line(0), column(0) {}
    Token(TokenType t, std::string l, int ln, int col)
        : type(t), lexeme(l), line(ln), column(col) {}
};

// Helper to get string representation of TokenType
std::string tokenTypeToString(TokenType type);

#endif // TOKEN_H
