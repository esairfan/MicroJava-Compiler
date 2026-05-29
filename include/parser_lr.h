#ifndef PARSER_LR_H
#define PARSER_LR_H

#include "lexer.h"
#include "symbol_table.h"
#include "tables.h"
#include <vector>
#include <string>

class LR1Parser {
private:
    Lexer& lexer;
    SymbolTable& symTable;
    Token currentToken;
    int errorCount;
    std::vector<int> stateStack;
    std::vector<std::string> symbolStack;

    void advance();
    void reportError(const std::string& message);
    std::string tokenToGrammarSymbol(const Token& t);

public:
    LR1Parser(Lexer& lex, SymbolTable& st);
    void parse();
    int getErrorCount() const { return errorCount; }
};

#endif // PARSER_LR_H
