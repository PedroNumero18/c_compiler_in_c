/**
 * Symbol Table Header
 * 
 * Defines structures and functions for the symbol table,
 * which keeps track of variables, functions, and their scopes.
*/

#ifndef SYMTAB_H
#define SYMTAB_H
 
#include "ast.h"
 
// Symbol types
typedef enum {
    SYMBOL_VARIABLE,
    SYMBOL_FUNCTION,
    SYMBOL_PARAMETER
} SymbolType;
 
// Symbol entry
typedef struct SymbolEntry {
    char *name;           // Symbol name
    SymbolType type;      // Type of symbol
    DataType data_type;   // Data type (int, char, void)
    int is_array;         // Is it an array?
    int array_size;       // Size of array (if array)
    int scope_level;      // Nesting level of scope
    int offset;           // Memory offset (for code generation)
    struct SymbolEntry *next;  // Next symbol in hash table bucket
} SymbolEntry;
 
// Symbol table
typedef struct {
    SymbolEntry **buckets;  // Hash table buckets
    int size;               // Number of buckets
    int scope_level;        // Current scope level
} SymbolTable;
 
// Function prototypes
SymbolTable* init_symbol_table(int size);
void free_symbol_table(SymbolTable *table);
 
void enter_scope(SymbolTable *table);
void exit_scope(SymbolTable *table);
 
SymbolEntry* add_symbol(SymbolTable *table, const char *name, SymbolType type, 
                         DataType data_type, int is_array, int array_size);
SymbolEntry* lookup_symbol(SymbolTable *table, const char *name);
SymbolEntry* lookup_symbol_current_scope(SymbolTable *table, const char *name);
 
void dump_symbol_table(SymbolTable *table);
 
#endif // SYMTAB_H