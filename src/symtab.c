/**
 * Symbol Table Implementation
 * 
 * Implements the symbol table for tracking variables,
 * functions, and their scopes during compilation.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../Include/common.h"
#include "../Include/symtab.h"
 
// Simple hash function for strings
 static unsigned int hash(const char *name, int size) {
     unsigned int hash_val = 0;
     while (*name) {
         hash_val = hash_val * 31 + *name++;
     }
     return hash_val % size;
 }
 
// Initialize a new symbol table
 SymbolTable* init_symbol_table(int size) {
     SymbolTable *table = (SymbolTable*)malloc(sizeof(SymbolTable));
     if (!table) return NULL;
     
     table->buckets = (SymbolEntry**)calloc(size, sizeof(SymbolEntry*));
     if (!table->buckets) {
         free(table);
         return NULL;
     }
     
     table->size = size;
     table->scope_level = 0;
     
     return table;
 }
 
// Free all memory used by the symbol table
 void free_symbol_table(SymbolTable *table) {
     if (!table) return;
     
     // Free all symbol entries
     for (int i = 0; i < table->size; i++) {
         SymbolEntry *entry = table->buckets[i];
         while (entry) {
             SymbolEntry *next = entry->next;
             free(entry->name);
             free(entry);
             entry = next;
         }
     }
     
     free(table->buckets);
     free(table);
 }
 
// Enter a new scope
 void enter_scope(SymbolTable *table) {
     if (table) {
         table->scope_level++;
     }
 }
 
// Exit the current scope, removing all symbols at this level
 void exit_scope(SymbolTable *table) {
     if (!table || table->scope_level <= 0) return;
     
     // Remove all symbols in the current scope
     for (int i = 0; i < table->size; i++) {
         SymbolEntry **pp = &table->buckets[i];
         while (*pp) {
             SymbolEntry *entry = *pp;
             if (entry->scope_level == table->scope_level) {
                 *pp = entry->next;
                 free(entry->name);
                 free(entry);
             } else {
                 pp = &entry->next;
             }
         }
     }
     
     table->scope_level--;
 }
  // Add a symbol to the symbol table
 SymbolEntry* add_symbol(SymbolTable *table, const char *name, SymbolType type, 
                         DataType data_type, int is_array, int array_size) {
     if (!table || !name) return NULL;
     
     // Check if symbol already exists in current scope
     SymbolEntry *existing = lookup_symbol_current_scope(table, name);
     if (existing) {
         return NULL;  // Symbol already defined in this scope
     }
     
     // Create new symbol entry
     SymbolEntry *entry = (SymbolEntry*)malloc(sizeof(SymbolEntry));
     if (!entry) return NULL;
     
     entry->name = strdup(name);
     if (!entry->name) {
         free(entry);
         return NULL;
     }
     
     entry->type = type;
     entry->data_type = data_type;
     entry->is_array = is_array;
     entry->array_size = array_size;
     entry->scope_level = table->scope_level;
     entry->offset = 0;  // Will be set during code generation
     
     // Insert at the beginning of the bucket
     unsigned int index = hash(name, table->size);
     entry->next = table->buckets[index];
     table->buckets[index] = entry;
     
     return entry;
 }
 
// Look up a symbol in all visible scopes
 SymbolEntry* lookup_symbol(SymbolTable *table, const char *name) {
     if (!table || !name) return NULL;
     
     unsigned int index = hash(name, table->size);
     SymbolEntry *entry = table->buckets[index];
     
     while (entry) {
         if (strcmp(entry->name, name) == 0) {
             return entry;
         }
         entry = entry->next;
     }
     
     return NULL;  // Symbol not found
 }

 // Look up a symbol in the current scope only
 SymbolEntry* lookup_symbol_current_scope(SymbolTable *table, const char *name) {
     if (!table || !name) return NULL;
     
     unsigned int index = hash(name, table->size);
     SymbolEntry *entry = table->buckets[index];
     
     while (entry) {
         if (entry->scope_level == table->scope_level && 
             strcmp(entry->name, name) == 0) {
             return entry;
         }
         entry = entry->next;
     }
     
     return NULL;  // Symbol not found in current scope
 }
 
// Get string representation of symbol type
 static const char* symbol_type_str(SymbolType type) {
     switch (type) {
         case SYMBOL_VARIABLE: return "variable";
         case SYMBOL_FUNCTION: return "function";
         case SYMBOL_PARAMETER: return "parameter";
         default: return "unknown";
     }
 }
 
// Get string representation of data type
 static const char* data_type_str(DataType type) {
     switch (type) {
         case TYPE_VOID: return "void";
         case TYPE_INT: return "int";
         case TYPE_CHAR: return "char";
         default: return "unknown";
     }
 }
 
// Print the contents of the symbol table (for debugging)
 void dump_symbol_table(SymbolTable *table) {
     if (!table) return;
     
     printf("Symbol Table (scope level: %d)\n", table->scope_level);
     printf("----------------------------------------------------\n");
     printf("Name                 Type       Data Type   Scope   Offset\n");
     printf("----------------------------------------------------\n");
     
     for (int i = 0; i < table->size; i++) {
         SymbolEntry *entry = table->buckets[i];
         while (entry) {
             printf("%-20s %-10s %-10s%s %5d %7d\n",
                    entry->name,
                    symbol_type_str(entry->type),
                    data_type_str(entry->data_type),
                    entry->is_array ? "[]" : "  ",
                    entry->scope_level,
                    entry->offset);
             
             entry = entry->next;
         }
     }
     
     printf("----------------------------------------------------\n");
}
 