/**
 * Code Generation Header
 * 
 * Defines structures and functions for generating code from the AST.
*/

#ifndef CODEGEN_H
#define CODEGEN_H
 
#include <stdio.h>
#include "ast.h"
#include "symtab.h"
 
 
// Intermediate representation instruction types
typedef enum {
    IR_LABEL,        // Label
    IR_LOAD,         // Load value
    IR_STORE,        // Store value
    IR_ADD,          // Addition
    IR_SUB,          // Subtraction
    IR_MUL,          // Multiplication
    IR_DIV,          // Division
    IR_MOD,          // Modulo
    IR_NEG,          // Negation
    IR_AND,          // Logical AND
    IR_OR,           // Logical OR
    IR_NOT,          // Logical NOT
    IR_EQ,           // Equality
    IR_NEQ,          // Inequality
    IR_LT,           // Less than
    IR_GT,           // Greater than
    IR_LTE,          // Less than or equal
    IR_GTE,          // Greater than or equal
    IR_JMP,          // Unconditional jump
    IR_JMPZ,         // Jump if zero
    IR_JMPNZ,        // Jump if not zero
    IR_CALL,         // Function call
    IR_RET,          // Return
    IR_ARG,          // Argument
    IR_ALLOC         // Allocate local variable
} IRType;
 
// IR operand structure
typedef struct {
    enum {
        OP_NONE,     // No operand
        OP_INTEGER,  // Integer constant
        OP_STRING,   // String constant
        OP_VARIABLE, // Variable
        OP_LABEL,    // Label
        OP_REGISTER  // Register
    } type;
     
    union {
        int integer;
        char *string;
        int reg;
    } value;
} IROperand;
 
// IR instruction structure
 typedef struct IRInst {
     IRType type;           // Instruction type
     IROperand dest;        // Destination operand
     IROperand src1;        // Source operand 1
     IROperand src2;        // Source operand 2
     struct IRInst *next;   // Next instruction
 } IRInst;
 
// Code generator structure
 typedef struct {
     SymbolTable *symtab;   // Symbol table
     IRInst *ir_head;       // Head of IR instruction list
     IRInst *ir_tail;       // Tail of IR instruction list
     int label_count;       // Counter for generating unique labels
     int temp_reg_count;    // Counter for generating temporary registers
} CodeGenerator;
 
// Function prototypes
 CodeGenerator* init_code_generator(SymbolTable *symtab);
 void free_code_generator(CodeGenerator *generator);
 
 int generate_code(ASTNode *ast, FILE *output, int generate_assembly);
 IRInst* generate_ir(CodeGenerator *generator, ASTNode *ast);
 int generate_assembly(CodeGenerator *generator, FILE *output);
 int generate_machine_code(CodeGenerator *generator, FILE *output);
 
#endif // CODEGEN_H