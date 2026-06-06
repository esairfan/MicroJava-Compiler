#include "../include/symbol_table.h"

std::string kindToString(SymbolKind kind) {
    switch(kind) {
        case SymbolKind::VAR: return "VAR";
        case SymbolKind::CONST: return "CONST";
        case SymbolKind::METHOD: return "METHOD";
        case SymbolKind::ARRAY: return "ARRAY";
        case SymbolKind::CLASS: return "CLASS";
        default: return "UNKNOWN";
    }
}

std::string typeToString(DataType type, const std::string& userType) {
    switch(type) {
        case DataType::INT: return "int";
        case DataType::CHAR: return "char";
        case DataType::VOID: return "void";
        case DataType::USER_DEFINED: return userType;
        case DataType::NULL_TYPE: return "null";
        default: return "none";
    }
}

SymbolTable::SymbolTable() {
    currentLevel = -1;
    // Push the universe scope
    enterScope();
    
    // Insert predeclared standard methods
    insert("chr", SymbolKind::METHOD, DataType::CHAR, "", 0);
    if (auto info = lookupLocal("chr")) info->numParams = 1;

    insert("ord", SymbolKind::METHOD, DataType::INT, "", 0);
    if (auto info = lookupLocal("ord")) info->numParams = 1;
    
    insert("len", SymbolKind::METHOD, DataType::INT, "", 0);
    if (auto info = lookupLocal("len")) info->numParams = 1;
}

SymbolTable::~SymbolTable() {}

void SymbolTable::enterScope() {
    currentLevel++;
    scopes.push_back(Scope(currentLevel));
}

void SymbolTable::exitScope() {
    if (!scopes.empty()) {
        allScopes.push_back(scopes.back());
        scopes.pop_back();
        currentLevel--;
    }
}

bool SymbolTable::insert(const std::string& name, SymbolKind kind, DataType type, const std::string& userType, int line, DataType elementType, std::string elementUserType) {
    if (scopes.empty()) return false;
    
    auto& currentScope = scopes.back().table;
    if (currentScope.find(name) != currentScope.end()) {
        // Already exists in current scope
        return false;
    }
    
    SymbolInfo info;
    info.name = name;
    info.kind = kind;
    info.type = type;
    info.userType = userType;
    info.scopeLevel = currentLevel;
    info.line = line;
    info.elementType = elementType;
    info.elementUserType = elementUserType;
    
    currentScope[name] = info;
    return true;
}

SymbolInfo* SymbolTable::lookup(const std::string& name) {
    for (int i = currentLevel; i >= 0; --i) {
        auto& scopeTable = scopes[i].table;
        auto it = scopeTable.find(name);
        if (it != scopeTable.end()) {
            return &(it->second);
        }
    }
    return nullptr;
}

SymbolInfo* SymbolTable::lookupLocal(const std::string& name) {
    if (scopes.empty()) return nullptr;
    
    auto& scopeTable = scopes.back().table;
    auto it = scopeTable.find(name);
    if (it != scopeTable.end()) {
        return &(it->second);
    }
    return nullptr;
}

bool SymbolTable::remove(const std::string& name) {
    if (scopes.empty()) return false;
    auto& scopeTable = scopes.back().table;
    auto it = scopeTable.find(name);
    if (it != scopeTable.end()) {
        scopeTable.erase(it);
        return true;
    }
    return false;
}

void SymbolTable::printTable() const {
    std::cout << "\033[1;36m==============================================================================\033[0m\n";
    std::cout << "\033[1;36m                         PHASE 3: SYMBOL TABLE DUMP                           \033[0m\n";
    std::cout << "\033[1;36m==============================================================================\033[0m\n";
    std::cout << "+-------+----------------------+------------+----------------------------+------+\n";
    std::cout << "| Scope | Symbol Name          | Kind       | Data Type                  | Line |\n";
    std::cout << "+-------+----------------------+------------+----------------------------+------+\n";
    
    for (const auto& scope : allScopes) {
        for (const auto& pair : scope.table) {
            const SymbolInfo& info = pair.second;
            std::string dt = typeToString(info.type, info.userType);
            if (info.kind == SymbolKind::ARRAY) {
                dt += " of " + typeToString(info.elementType, info.elementUserType);
            }
            printf("| %-5d | %-20s | %-10s | %-26s | %-4d |\n",
                   scope.level,
                   info.name.c_str(),
                   kindToString(info.kind).c_str(),
                   dt.c_str(),
                   info.line);
        }
    }
    
    for (const auto& scope : scopes) {
        for (const auto& pair : scope.table) {
            const SymbolInfo& info = pair.second;
            std::string dt = typeToString(info.type, info.userType);
            if (info.kind == SymbolKind::ARRAY) {
                dt += " of " + typeToString(info.elementType, info.elementUserType);
            }
            printf("| %-5d | %-20s | %-10s | %-26s | %-4d |\n",
                   scope.level,
                   info.name.c_str(),
                   kindToString(info.kind).c_str(),
                   dt.c_str(),
                   info.line);
        }
    }
    std::cout << "+-------+----------------------+------------+----------------------------+------+\n\n";
}
