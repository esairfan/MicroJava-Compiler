#include "../include/lexer.h"
#include <iostream>

Lexer::Lexer(const std::string& filename) {
    file.open(filename, std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << "\n";
    }
    
    // Initialize buffers with EOF
    buffer1[BUFFER_SIZE] = EOF;
    buffer2[BUFFER_SIZE] = EOF;
    
    line = 1;
    column = 1;
    tokenStartLine = 1;
    tokenStartColumn = 1;
    forward = 0;
    eofReached = false;
    
    currentBuffer = buffer1;
    fillBuffer(buffer1);
}

Lexer::~Lexer() {
    if (file.is_open()) {
        file.close();
    }
}

void Lexer::fillBuffer(char* buffer) {
    if (eofReached) {
        buffer[0] = EOF;
        return;
    }
    
    file.read(buffer, BUFFER_SIZE);
    std::streamsize bytesRead = file.gcount();
    
    if (bytesRead < BUFFER_SIZE) {
        buffer[bytesRead] = EOF;
        eofReached = true;
    } else {
        buffer[BUFFER_SIZE] = EOF;
    }
}

char Lexer::getNextChar() {
    char c = currentBuffer[forward++];
    
    if (c == EOF) {
        if (forward == BUFFER_SIZE + 1) { // Hit end of buffer
            // Switch buffers
            if (currentBuffer == buffer1) {
                fillBuffer(buffer2);
                currentBuffer = buffer2;
            } else {
                fillBuffer(buffer1);
                currentBuffer = buffer1;
            }
            forward = 0;
            c = currentBuffer[forward++];
            
            if (c == EOF) {
                // The newly filled buffer started with EOF (real EOF)
                forward--;
            }
        } else {
            // Hit real EOF sentinel in the middle/start of the buffer
            forward--;
        }
    }
    
    if (c != EOF) {
        column++;
        if (c == '\n') {
            line++;
            column = 1;
        }
    }
    
    return c;
}

void Lexer::retract() {
    forward--;
    column--;
    if (forward < 0) {
        // Retracting across buffer boundary
        if (currentBuffer == buffer1) {
            currentBuffer = buffer2;
        } else {
            currentBuffer = buffer1;
        }
        forward = BUFFER_SIZE - 1;
    }
}

bool Lexer::isWhitespace(char c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

bool Lexer::isLetter(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool Lexer::isDigit(char c) {
    return c >= '0' && c <= '9';
}

void Lexer::skipWhitespaceAndComments() {
    while (true) {
        char c = getNextChar();
        
        if (isWhitespace(c)) {
            continue;
        } else if (c == '/') {
            char next = getNextChar();
            if (next == '/') {
                // Line comment, skip until \n or EOF
                while (true) {
                    char cc = getNextChar();
                    if (cc == '\n' || cc == EOF) break;
                }
            } else {
                // Not a comment, retract both
                retract(); // retract next
                retract(); // retract c
                break;
            }
        } else {
            if (c != EOF) {
                retract();
            }
            break;
        }
    }
}

TokenType Lexer::checkKeyword(const std::string& lexeme) {
    if (lexeme == "program") return TokenType::KW_PROGRAM;
    if (lexeme == "class") return TokenType::KW_CLASS;
    if (lexeme == "if") return TokenType::KW_IF;
    if (lexeme == "else") return TokenType::KW_ELSE;
    if (lexeme == "while") return TokenType::KW_WHILE;
    if (lexeme == "read") return TokenType::KW_READ;
    if (lexeme == "print") return TokenType::KW_PRINT;
    if (lexeme == "return") return TokenType::KW_RETURN;
    if (lexeme == "void") return TokenType::KW_VOID;
    if (lexeme == "final") return TokenType::KW_FINAL;
    if (lexeme == "new") return TokenType::KW_NEW;
    return TokenType::IDENTIFIER;
}

Token Lexer::getNextToken() {
    skipWhitespaceAndComments();
    
    tokenStartLine = line;
    tokenStartColumn = column;
    
    char c = getNextChar();
    
    if (c == EOF) {
        return Token(TokenType::END_OF_FILE, "EOF", tokenStartLine, tokenStartColumn);
    }
    
    // Identifiers and Keywords
    if (isLetter(c)) {
        std::string lexeme = "";
        lexeme += c;
        while (true) {
            c = getNextChar();
            if (isLetter(c) || isDigit(c)) {
                lexeme += c;
            } else {
                if (c != EOF) retract();
                break;
            }
        }
        return Token(checkKeyword(lexeme), lexeme, tokenStartLine, tokenStartColumn);
    }
    
    // Numbers
    if (isDigit(c)) {
        std::string lexeme = "";
        lexeme += c;
        while (true) {
            c = getNextChar();
            if (isDigit(c)) {
                lexeme += c;
            } else {
                if (c != EOF) retract();
                break;
            }
        }
        return Token(TokenType::NUMBER, lexeme, tokenStartLine, tokenStartColumn);
    }
    
    // Character constants e.g. 'x', '\n', '\r', '\t'
    if (c == '\'') {
        std::string lexeme = "'";
        char next = getNextChar();
        lexeme += next;
        
        if (next == '\\') {
            char escape = getNextChar();
            lexeme += escape;
        }
        
        char quote = getNextChar();
        lexeme += quote;
        
        if (quote != '\'') {
            return Token(TokenType::ERROR, "Malformed char constant: " + lexeme, tokenStartLine, tokenStartColumn);
        }
        
        return Token(TokenType::CHAR_CONST, lexeme, tokenStartLine, tokenStartColumn);
    }
    
    // Operators and Punctuation
    switch (c) {
        case '+': return Token(TokenType::PLUS, "+", tokenStartLine, tokenStartColumn);
        case '-': return Token(TokenType::MINUS, "-", tokenStartLine, tokenStartColumn);
        case '*': return Token(TokenType::STAR, "*", tokenStartLine, tokenStartColumn);
        case '/': return Token(TokenType::SLASH, "/", tokenStartLine, tokenStartColumn);
        case '%': return Token(TokenType::PERCENT, "%", tokenStartLine, tokenStartColumn);
        case '(': return Token(TokenType::LPAREN, "(", tokenStartLine, tokenStartColumn);
        case ')': return Token(TokenType::RPAREN, ")", tokenStartLine, tokenStartColumn);
        case '[': return Token(TokenType::LBRACKET, "[", tokenStartLine, tokenStartColumn);
        case ']': return Token(TokenType::RBRACKET, "]", tokenStartLine, tokenStartColumn);
        case '{': return Token(TokenType::LBRACE, "{", tokenStartLine, tokenStartColumn);
        case '}': return Token(TokenType::RBRACE, "}", tokenStartLine, tokenStartColumn);
        case ';': return Token(TokenType::SEMICOLON, ";", tokenStartLine, tokenStartColumn);
        case ',': return Token(TokenType::COMMA, ",", tokenStartLine, tokenStartColumn);
        case '.': return Token(TokenType::DOT, ".", tokenStartLine, tokenStartColumn);
        
        case '=':
            c = getNextChar();
            if (c == '=') return Token(TokenType::EQ, "==", tokenStartLine, tokenStartColumn);
            retract();
            return Token(TokenType::ASSIGN, "=", tokenStartLine, tokenStartColumn);
            
        case '!':
            c = getNextChar();
            if (c == '=') return Token(TokenType::NEQ, "!=", tokenStartLine, tokenStartColumn);
            retract();
            return Token(TokenType::ERROR, "!", tokenStartLine, tokenStartColumn); // '!' alone is invalid in MicroJava
            
        case '>':
            c = getNextChar();
            if (c == '=') return Token(TokenType::GTE, ">=", tokenStartLine, tokenStartColumn);
            retract();
            return Token(TokenType::GT, ">", tokenStartLine, tokenStartColumn);
            
        case '<':
            c = getNextChar();
            if (c == '=') return Token(TokenType::LTE, "<=", tokenStartLine, tokenStartColumn);
            retract();
            return Token(TokenType::LT, "<", tokenStartLine, tokenStartColumn);
    }
    
    // Unknown character
    std::string unk = ""; unk += c;
    return Token(TokenType::ERROR, "Unknown char: " + unk, tokenStartLine, tokenStartColumn);
}
