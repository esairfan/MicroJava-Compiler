#ifndef LEXER_H
#define LEXER_H

#include "token.h"
#include <fstream>
#include <string>

class Lexer {
private:
    static const int BUFFER_SIZE = 4096;
    char buffer1[BUFFER_SIZE + 1]; // +1 for EOF sentinel
    char buffer2[BUFFER_SIZE + 1];
    char* currentBuffer;
    
    int forward;  // index of current character in the active buffer
    int line;
    int column;
    int tokenStartLine;
    int tokenStartColumn;

    std::ifstream file;
    bool eofReached;

    void fillBuffer(char* buffer);
    char getNextChar();
    void retract();

    bool isWhitespace(char c);
    bool isLetter(char c);
    bool isDigit(char c);
    
    void skipWhitespaceAndComments();
    TokenType checkKeyword(const std::string& lexeme);

public:
    Lexer(const std::string& filename);
    ~Lexer();

    Token getNextToken();
};

#endif // LEXER_H
