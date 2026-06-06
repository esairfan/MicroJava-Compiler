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
    if (!currentToken.lexeme.empty() && currentToken.type != TokenType::END_OF_FILE && currentToken.type != TokenType::ERROR) {
        for (int i = 0; i < indentLevel; ++i) {
            std::cout << "│  ";
        }
        std::cout << "├── " << tokenTypeToString(currentToken.type) << " (token: \"" << currentToken.lexeme << "\")\n";
    }
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
    
    std::string progName = currentToken.lexeme;
    int line = currentToken.line;
    match(TokenType::IDENTIFIER);
    
    symTable.insert(progName, SymbolKind::CLASS, DataType::VOID, "", line);
    
    symTable.enterScope(); // Enter global program scope
    
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
    
    symTable.exitScope(); // Exit global program scope
    indentLevel--;
}

void RecursiveDescentParser::parseConstDecl() {
    printNode("ConstDecl");
    indentLevel++;
    match(TokenType::KW_FINAL);
    
    parseType();
    DataType constType = lastParsedType;
    std::string constUserType = lastParsedUserType;
    
    std::string constName = currentToken.lexeme;
    int line = currentToken.line;
    match(TokenType::IDENTIFIER);
    
    match(TokenType::ASSIGN);
    if (currentToken.type == TokenType::NUMBER || currentToken.type == TokenType::CHAR_CONST) {
        advance();
    } else {
        reportError("Expected number or char constant");
    }
    match(TokenType::SEMICOLON);
    
    symTable.insert(constName, SymbolKind::CONST, constType, constUserType, line);
    indentLevel--;
}

void RecursiveDescentParser::parseVarDecl() {
    printNode("VarDecl");
    indentLevel++;
    
    parseType();
    DataType varType = lastParsedType;
    std::string varUserType = lastParsedUserType;
    bool isArray = lastParsedIsArray;
    
    std::string varName = currentToken.lexeme;
    int line = currentToken.line;
    match(TokenType::IDENTIFIER);
    
    if (isArray) {
        symTable.insert(varName, SymbolKind::ARRAY, DataType::NONE, "", line, varType, varUserType);
    } else {
        symTable.insert(varName, SymbolKind::VAR, varType, varUserType, line);
    }
    
    while (currentToken.type == TokenType::COMMA) {
        advance();
        std::string nextVarName = currentToken.lexeme;
        int nextLine = currentToken.line;
        match(TokenType::IDENTIFIER);
        
        if (isArray) {
            symTable.insert(nextVarName, SymbolKind::ARRAY, DataType::NONE, "", nextLine, varType, varUserType);
        } else {
            symTable.insert(nextVarName, SymbolKind::VAR, varType, varUserType, nextLine);
        }
    }
    match(TokenType::SEMICOLON);
    indentLevel--;
}

void RecursiveDescentParser::parseClassDecl() {
    printNode("ClassDecl");
    indentLevel++;
    match(TokenType::KW_CLASS);
    
    std::string className = currentToken.lexeme;
    int line = currentToken.line;
    match(TokenType::IDENTIFIER);
    
    symTable.insert(className, SymbolKind::CLASS, DataType::USER_DEFINED, className, line);
    
    symTable.enterScope(); // Enter class scope
    
    match(TokenType::LBRACE);
    while (currentToken.type == TokenType::IDENTIFIER) {
        parseVarDecl();
    }
    match(TokenType::RBRACE);
    
    symTable.exitScope(); // Exit class scope
    indentLevel--;
}

void RecursiveDescentParser::parseMethodDecl() {
    printNode("MethodDecl");
    indentLevel++;
    
    DataType retType = DataType::VOID;
    std::string retTypeName = "";
    if (currentToken.type == TokenType::KW_VOID) {
        advance();
        retType = DataType::VOID;
    } else {
        parseType();
        retTypeName = lastParsedUserType;
        if (lastParsedIsArray) {
            retType = DataType::NONE;
        } else if (lastParsedType == DataType::INT) {
            retType = DataType::INT;
        } else if (lastParsedType == DataType::CHAR) {
            retType = DataType::CHAR;
        } else {
            retType = DataType::USER_DEFINED;
        }
    }
    
    std::string methodName = currentToken.lexeme;
    int line = currentToken.line;
    match(TokenType::IDENTIFIER);
    
    symTable.insert(methodName, SymbolKind::METHOD, retType, (retType == DataType::USER_DEFINED ? retTypeName : ""), line);
    
    symTable.enterScope(); // Enter method scope
    
    match(TokenType::LPAREN);
    if (currentToken.type == TokenType::IDENTIFIER) {
        parseFormPars();
    }
    match(TokenType::RPAREN);
    
    while (currentToken.type == TokenType::IDENTIFIER) {
        parseVarDecl();
    }
    
    parseBlock();
    
    symTable.exitScope(); // Exit method scope
    indentLevel--;
}

void RecursiveDescentParser::parseFormPars() {
    printNode("FormPars");
    indentLevel++;
    
    parseType();
    DataType paramType = lastParsedType;
    std::string paramUserType = lastParsedUserType;
    bool isArray = lastParsedIsArray;
    
    std::string paramName = currentToken.lexeme;
    int line = currentToken.line;
    match(TokenType::IDENTIFIER);
    
    if (isArray) {
        symTable.insert(paramName, SymbolKind::ARRAY, DataType::NONE, "", line, paramType, paramUserType);
    } else {
        symTable.insert(paramName, SymbolKind::VAR, paramType, paramUserType, line);
    }
    
    while (currentToken.type == TokenType::COMMA) {
        advance();
        parseType();
        DataType nextParamType = lastParsedType;
        std::string nextParamUserType = lastParsedUserType;
        bool nextIsArray = lastParsedIsArray;
        
        std::string nextParamName = currentToken.lexeme;
        int nextLine = currentToken.line;
        match(TokenType::IDENTIFIER);
        
        if (nextIsArray) {
            symTable.insert(nextParamName, SymbolKind::ARRAY, DataType::NONE, "", nextLine, nextParamType, nextParamUserType);
        } else {
            symTable.insert(nextParamName, SymbolKind::VAR, nextParamType, nextParamUserType, nextLine);
        }
    }
    indentLevel--;
}

void RecursiveDescentParser::parseType() {
    printNode("Type");
    indentLevel++;
    
    std::string typeName = currentToken.lexeme;
    match(TokenType::IDENTIFIER);
    
    if (typeName == "int") {
        lastParsedType = DataType::INT;
        lastParsedUserType = "";
    } else if (typeName == "char") {
        lastParsedType = DataType::CHAR;
        lastParsedUserType = "";
    } else {
        lastParsedType = DataType::USER_DEFINED;
        lastParsedUserType = typeName;
    }
    
    if (currentToken.type == TokenType::LBRACKET) {
        advance();
        match(TokenType::RBRACKET);
        lastParsedIsArray = true;
    } else {
        lastParsedIsArray = false;
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
