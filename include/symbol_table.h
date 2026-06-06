#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <string>
#include <unordered_map>
#include <vector>
#include <iostream>

enum class SymbolKind {
    VAR, CONST, METHOD, ARRAY, CLASS
};

enum class DataType {
    INT, CHAR, VOID, USER_DEFINED, NULL_TYPE, NONE
};

struct SymbolInfo {
    std::string name;
    SymbolKind kind;
    DataType type;
    std::string userType; // Name of the class if DataType::USER_DEFINED
    int scopeLevel;
    int line;

    // For arrays
    DataType elementType;
    std::string elementUserType;
    
    // For methods
    int numParams;
    
    // Constructor
    SymbolInfo() : kind(SymbolKind::VAR), type(DataType::NONE), scopeLevel(0), line(0), elementType(DataType::NONE), numParams(0) {}
};

class Scope {
public:
    std::unordered_map<std::string, SymbolInfo> table;
    int level;
    
    Scope(int l) : level(l) {}
};

class SymbolTable {
private:
    std::vector<Scope> scopes;
    int currentLevel;
    std::vector<Scope> allScopes;

public:
    SymbolTable();
    ~SymbolTable();

    void enterScope();
    void exitScope();

    bool insert(const std::string& name, SymbolKind kind, DataType type, const std::string& userType, int line, DataType elementType = DataType::NONE, std::string elementUserType = "");
    
    // Look up in all scopes from inner to outer
    SymbolInfo* lookup(const std::string& name);
    
    // Look up only in current scope
    SymbolInfo* lookupLocal(const std::string& name);

    // Delete a symbol from the current scope
    bool remove(const std::string& name);

    void printTable() const;
    
    int getCurrentLevel() const { return currentLevel; }
};

std::string kindToString(SymbolKind kind);

#endif // SYMBOL_TABLE_H
