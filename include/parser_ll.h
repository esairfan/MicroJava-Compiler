#ifndef PARSER_LL_H
#define PARSER_LL_H

#include "lexer.h"
#include "symbol_table.h"
#include "tables.h"
#include <stack>
#include <string>

class LL1Parser {
private:
    Lexer& lexer;
    SymbolTable& symTable;
    Token currentToken;
    int errorCount;
    std::stack<std::string> parseStack;
    std::vector<Token> allTokens;
    size_t tokenIndex;

    void advance();
    void reportError(const std::string& message);
    std::string tokenToGrammarSymbol(const Token& t);

public:
    LL1Parser(Lexer& lex, SymbolTable& st);
    void parse();
    int getErrorCount() const { return errorCount; }
};

#endif // PARSER_LL_H
