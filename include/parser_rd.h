#ifndef PARSER_RD_H
#define PARSER_RD_H

#include "lexer.h"
#include "symbol_table.h"

class RecursiveDescentParser {
private:
    Lexer& lexer;
    SymbolTable& symTable;
    Token currentToken;
    int errorCount;

    int indentLevel;
    DataType lastParsedType;
    std::string lastParsedUserType;
    bool lastParsedIsArray;

    void printNode(const std::string& nodeName);
    void advance();
    void match(TokenType expected);
    void reportError(const std::string& message);
void reportSemanticError(const std::string& message); // Add this

    // MicroJava Grammar Methods
    void parseProgram();
    void parseConstDecl();
    void parseVarDecl();
    void parseClassDecl();
    void parseMethodDecl();
    void parseFormPars();
    void parseType();
    void parseBlock();
    void parseStatement();
    void parseActPars();
    void parseCondition();
    void parseRelop();
    void parseExpr();
    void parseTerm();
    void parseFactor();
    void parseDesignator();
    void parseAddop();
    void parseMulop();

public:
    RecursiveDescentParser(Lexer& lex, SymbolTable& st);
    void parse();
    int getErrorCount() const { return errorCount; }
};

#endif // PARSER_RD_H
