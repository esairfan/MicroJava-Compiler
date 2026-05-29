#include "../include/parser_rd.h"
#include <iostream>

RecursiveDescentParser::RecursiveDescentParser(Lexer& lex, SymbolTable& st) 
    : lexer(lex), symTable(st), errorCount(0), indentLevel(0) {
    advance();
}

void RecursiveDescentParser::printNode(const std::string& nodeName) {
    for (int i = 0; i < indentLevel; ++i) {
        std::cout << "│  ";
    }
    std::cout << "├── \033[1;33m" << nodeName << "\033[0m (token: \"" << currentToken.lexeme << "\")\n";
}

void RecursiveDescentParser::advance() {
    currentToken = lexer.getNextToken();
    while (currentToken.type == TokenType::ERROR) {
        reportError("Lexical error: " + currentToken.lexeme);
        currentToken = lexer.getNextToken();
    }
}

void RecursiveDescentParser::match(TokenType expected) {
    if (currentToken.type == expected) {
        advance();
    } else {
        reportError("Expected " + tokenTypeToString(expected) + " but found " + tokenTypeToString(currentToken.type));
    }
}

void RecursiveDescentParser::reportError(const std::string& message) {
    std::cerr << "Error at line " << currentToken.line << ", col " << currentToken.column << ": " << message << "\n";
    errorCount++;
    
    // Simple panic mode: advance until we find a semicolon, brace, or EOF
    if (currentToken.type != TokenType::END_OF_FILE) {
        advance();
        while (currentToken.type != TokenType::SEMICOLON && 
               currentToken.type != TokenType::RBRACE &&
               currentToken.type != TokenType::END_OF_FILE) {
            advance();
        }
        if (currentToken.type == TokenType::SEMICOLON) {
            advance();
        }
    }
}

void RecursiveDescentParser::parse() {
    parseProgram();
    if (currentToken.type != TokenType::END_OF_FILE) {
        reportError("Extra tokens at end of file");
    }
}

void RecursiveDescentParser::parseProgram() {
    printNode("Program");
    indentLevel++;
    match(TokenType::KW_PROGRAM);
    match(TokenType::IDENTIFIER);
    
    while (currentToken.type == TokenType::KW_FINAL || 
           currentToken.type == TokenType::IDENTIFIER || 
           currentToken.type == TokenType::KW_CLASS) {
        if (currentToken.type == TokenType::KW_FINAL) {
            parseConstDecl();
        } else if (currentToken.type == TokenType::IDENTIFIER) {
            parseVarDecl();
        } else if (currentToken.type == TokenType::KW_CLASS) {
            parseClassDecl();
        }
    }
    
    match(TokenType::LBRACE);
    while (currentToken.type == TokenType::IDENTIFIER || currentToken.type == TokenType::KW_VOID) {
        parseMethodDecl();
    }
    match(TokenType::RBRACE);
    indentLevel--;
}

void RecursiveDescentParser::parseConstDecl() {
    printNode("ConstDecl");
    indentLevel++;
    match(TokenType::KW_FINAL);
    parseType();
    match(TokenType::IDENTIFIER);
    match(TokenType::ASSIGN);
    if (currentToken.type == TokenType::NUMBER || currentToken.type == TokenType::CHAR_CONST) {
        advance();
    } else {
        reportError("Expected number or char constant");
    }
    match(TokenType::SEMICOLON);
    indentLevel--;
}

void RecursiveDescentParser::parseVarDecl() {
    printNode("VarDecl");
    indentLevel++;
    parseType();
    match(TokenType::IDENTIFIER);
    while (currentToken.type == TokenType::COMMA) {
        advance();
        match(TokenType::IDENTIFIER);
    }
    match(TokenType::SEMICOLON);
    indentLevel--;
}

void RecursiveDescentParser::parseClassDecl() {
    printNode("ClassDecl");
    indentLevel++;
    match(TokenType::KW_CLASS);
    match(TokenType::IDENTIFIER);
    match(TokenType::LBRACE);
    while (currentToken.type == TokenType::IDENTIFIER) {
        parseVarDecl();
    }
    match(TokenType::RBRACE);
    indentLevel--;
}

void RecursiveDescentParser::parseMethodDecl() {
    printNode("MethodDecl");
    indentLevel++;
    if (currentToken.type == TokenType::KW_VOID) {
        advance();
    } else {
        parseType();
    }
    
    match(TokenType::IDENTIFIER);
    match(TokenType::LPAREN);
    if (currentToken.type == TokenType::IDENTIFIER) {
        parseFormPars();
    }
    match(TokenType::RPAREN);
    
    while (currentToken.type == TokenType::IDENTIFIER) {
        parseVarDecl();
    }
    
    parseBlock();
    indentLevel--;
}

void RecursiveDescentParser::parseFormPars() {
    printNode("FormPars");
    indentLevel++;
    parseType();
    match(TokenType::IDENTIFIER);
    while (currentToken.type == TokenType::COMMA) {
        advance();
        parseType();
        match(TokenType::IDENTIFIER);
    }
    indentLevel--;
}

void RecursiveDescentParser::parseType() {
    printNode("Type");
    indentLevel++;
    match(TokenType::IDENTIFIER);
    if (currentToken.type == TokenType::LBRACKET) {
        advance();
        match(TokenType::RBRACKET);
    }
    indentLevel--;
}

void RecursiveDescentParser::parseBlock() {
    printNode("Block");
    indentLevel++;
    match(TokenType::LBRACE);
    while (currentToken.type != TokenType::RBRACE && currentToken.type != TokenType::END_OF_FILE) {
        parseStatement();
    }
    match(TokenType::RBRACE);
    indentLevel--;
}

void RecursiveDescentParser::parseStatement() {
    printNode("Statement");
    indentLevel++;
    if (currentToken.type == TokenType::IDENTIFIER) {
        parseDesignator();
        if (currentToken.type == TokenType::ASSIGN) {
            advance();
            parseExpr();
            match(TokenType::SEMICOLON);
        } else if (currentToken.type == TokenType::LPAREN) {
            parseActPars();
            match(TokenType::SEMICOLON);
        } else {
            reportError("Expected '=' or '(' after designator");
        }
    } else if (currentToken.type == TokenType::KW_IF) {
        advance();
        match(TokenType::LPAREN);
        parseCondition();
        match(TokenType::RPAREN);
        parseStatement();
        if (currentToken.type == TokenType::KW_ELSE) {
            advance();
            parseStatement();
        }
    } else if (currentToken.type == TokenType::KW_WHILE) {
        advance();
        match(TokenType::LPAREN);
        parseCondition();
        match(TokenType::RPAREN);
        parseStatement();
    } else if (currentToken.type == TokenType::KW_RETURN) {
        advance();
        if (currentToken.type == TokenType::MINUS || currentToken.type == TokenType::IDENTIFIER || 
            currentToken.type == TokenType::NUMBER || currentToken.type == TokenType::CHAR_CONST || 
            currentToken.type == TokenType::KW_NEW || currentToken.type == TokenType::LPAREN) {
            parseExpr();
        }
        match(TokenType::SEMICOLON);
    } else if (currentToken.type == TokenType::KW_READ) {
        advance();
        match(TokenType::LPAREN);
        parseDesignator();
        match(TokenType::RPAREN);
        match(TokenType::SEMICOLON);
    } else if (currentToken.type == TokenType::KW_PRINT) {
        advance();
        match(TokenType::LPAREN);
        parseExpr();
        if (currentToken.type == TokenType::COMMA) {
            advance();
            match(TokenType::NUMBER);
        }
        match(TokenType::RPAREN);
        match(TokenType::SEMICOLON);
    } else if (currentToken.type == TokenType::LBRACE) {
        parseBlock();
    } else if (currentToken.type == TokenType::SEMICOLON) {
        advance();
    } else {
        reportError("Invalid statement start");
    }
    indentLevel--;
}

void RecursiveDescentParser::parseActPars() {
    printNode("ActPars");
    indentLevel++;
    match(TokenType::LPAREN);
    if (currentToken.type == TokenType::MINUS || currentToken.type == TokenType::IDENTIFIER || 
        currentToken.type == TokenType::NUMBER || currentToken.type == TokenType::CHAR_CONST || 
        currentToken.type == TokenType::KW_NEW || currentToken.type == TokenType::LPAREN) {
        parseExpr();
        while (currentToken.type == TokenType::COMMA) {
            advance();
            parseExpr();
        }
    }
    match(TokenType::RPAREN);
    indentLevel--;
}

void RecursiveDescentParser::parseCondition() {
    printNode("Condition");
    indentLevel++;
    parseExpr();
    parseRelop();
    parseExpr();
    indentLevel--;
}

void RecursiveDescentParser::parseRelop() {
    printNode("Relop");
    indentLevel++;
    if (currentToken.type == TokenType::EQ || currentToken.type == TokenType::NEQ ||
        currentToken.type == TokenType::GT || currentToken.type == TokenType::GTE ||
        currentToken.type == TokenType::LT || currentToken.type == TokenType::LTE) {
        advance();
    } else {
        reportError("Expected relational operator");
    }
    indentLevel--;
}

void RecursiveDescentParser::parseExpr() {
    printNode("Expr");
    indentLevel++;
    if (currentToken.type == TokenType::MINUS) {
        advance();
    }
    parseTerm();
    while (currentToken.type == TokenType::PLUS || currentToken.type == TokenType::MINUS) {
        parseAddop();
        parseTerm();
    }
    indentLevel--;
}

void RecursiveDescentParser::parseTerm() {
    printNode("Term");
    indentLevel++;
    parseFactor();
    while (currentToken.type == TokenType::STAR || currentToken.type == TokenType::SLASH || currentToken.type == TokenType::PERCENT) {
        parseMulop();
        parseFactor();
    }
    indentLevel--;
}

void RecursiveDescentParser::parseFactor() {
    printNode("Factor");
    indentLevel++;
    if (currentToken.type == TokenType::IDENTIFIER) {
        parseDesignator();
        if (currentToken.type == TokenType::LPAREN) {
            parseActPars();
        }
    } else if (currentToken.type == TokenType::NUMBER) {
        advance();
    } else if (currentToken.type == TokenType::CHAR_CONST) {
        advance();
    } else if (currentToken.type == TokenType::KW_NEW) {
        advance();
        match(TokenType::IDENTIFIER);
        if (currentToken.type == TokenType::LBRACKET) {
            advance();
            parseExpr();
            match(TokenType::RBRACKET);
        }
    } else if (currentToken.type == TokenType::LPAREN) {
        advance();
        parseExpr();
        match(TokenType::RPAREN);
    } else {
        reportError("Invalid factor");
    }
    indentLevel--;
}

void RecursiveDescentParser::parseDesignator() {
    printNode("Designator");
    indentLevel++;
    match(TokenType::IDENTIFIER);
    while (currentToken.type == TokenType::DOT || currentToken.type == TokenType::LBRACKET) {
        if (currentToken.type == TokenType::DOT) {
            advance();
            match(TokenType::IDENTIFIER);
        } else if (currentToken.type == TokenType::LBRACKET) {
            advance();
            parseExpr();
            match(TokenType::RBRACKET);
        }
    }
    indentLevel--;
}

void RecursiveDescentParser::parseAddop() {
    printNode("Addop");
    indentLevel++;
    if (currentToken.type == TokenType::PLUS || currentToken.type == TokenType::MINUS) {
        advance();
    } else {
        reportError("Expected '+' or '-'");
    }
    indentLevel--;
}

void RecursiveDescentParser::parseMulop() {
    printNode("Mulop");
    indentLevel++;
    if (currentToken.type == TokenType::STAR || currentToken.type == TokenType::SLASH || currentToken.type == TokenType::PERCENT) {
        advance();
    } else {
        reportError("Expected '*', '/', or '%'");
    }
    indentLevel--;
}
