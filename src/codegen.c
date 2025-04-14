/**
 * Code Generation Implementation
 * 
 * Implements the code generation phase, which translates the AST
 * into intermediate representation and then to ARM assembly code.
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include "../Include/common.h"
 #include "../Include/codegen.h"
 #include "../Include/error.h"
 
 // Forward declarations for static functions
 static void gen_ir(CodeGenerator *generator, ASTNode *node);
 static int gen_expr(CodeGenerator *generator, ASTNode *node);
 static void gen_if_stmt(CodeGenerator *generator, ASTNode *node);
 static void gen_while_stmt(CodeGenerator *generator, ASTNode *node);
 static void gen_return_stmt(CodeGenerator *generator, ASTNode *node);
 static void gen_variable_decl(CodeGenerator *generator, ASTNode *node);
 static void gen_function(CodeGenerator *generator, ASTNode *node);
 static void generate_arm_asm(FILE *output, IRInst *ir);
 
 // Implementation of the function declared in codegen.h
 IRInst* generate_ir(CodeGenerator *generator, ASTNode *ast) {
     if (!generator || !ast) return NULL;
     
     // Generate IR code by traversing the AST
     gen_ir(generator, ast);
     
     // Return the head of the generated IR instruction list
     return generator->ir_head;
 }
 
 // Generate code for a node in the AST - Helper function
 static void gen_ir(CodeGenerator *generator, ASTNode *node) {
     if (!generator || !node) return;
     
     switch (node->type) {
         case AST_PROGRAM:
         case AST_COMPOUND_STMT:
             // Generate code for each child node
             for (int i = 0; i < node->num_children; i++) {
                 gen_ir(generator, node->children[i]);
             }
             break;
         
         case AST_FUNCTION:
             gen_function(generator, node);
             break;
         
         case AST_VARIABLE_DECL:
             gen_variable_decl(generator, node);
             break;
         
         case AST_IF_STMT:
             gen_if_stmt(generator, node);
             break;
         
         case AST_WHILE_STMT:
             gen_while_stmt(generator, node);
             break;
         
         case AST_RETURN_STMT:
             gen_return_stmt(generator, node);
             break;
         
         case AST_EXPR_STMT:
             // Generate code for expression if it exists
             if (node->num_children > 0) {
                 gen_expr(generator, node->children[0]);
             }
             break;
         
         default:
             fprintf(stderr, "Unsupported node type for code generation: %d\n", node->type);
             break;
     }
 }
 
 // Initialize code generator
 CodeGenerator* init_code_generator(SymbolTable *symtab) {
     CodeGenerator *generator = (CodeGenerator*)malloc(sizeof(CodeGenerator));
     if (!generator) return NULL;
     
     generator->symtab = symtab;
     generator->ir_head = NULL;
     generator->ir_tail = NULL;
     generator->label_count = 0;
     generator->temp_reg_count = 0;
     
     return generator;
 }
 
 // Free code generator resources
 void free_code_generator(CodeGenerator *generator) {
     if (!generator) return;
     
     // Free IR instruction list
     IRInst *inst = generator->ir_head;
     while (inst) {
         IRInst *next = inst->next;
         
         // Free string operands if any
         if (inst->dest.type == OP_STRING && inst->dest.value.string) {
             free(inst->dest.value.string);
         }
         if (inst->src1.type == OP_STRING && inst->src1.value.string) {
             free(inst->src1.value.string);
         }
         if (inst->src2.type == OP_STRING && inst->src2.value.string) {
             free(inst->src2.value.string);
         }
         
         free(inst);
         inst = next;
     }
     
     free(generator);
 }
 
 // Add a new IR instruction to the list
 static IRInst* add_ir_inst(CodeGenerator *generator, IRType type) {
     IRInst *inst = (IRInst*)malloc(sizeof(IRInst));
     if (!inst) return NULL;
     
     inst->type = type;
     inst->next = NULL;
     
     // Initialize operands
     inst->dest.type = OP_NONE;
     inst->src1.type = OP_NONE;
     inst->src2.type = OP_NONE;
     
     // Add to the list
     if (generator->ir_tail) {
         generator->ir_tail->next = inst;
     } else {
         generator->ir_head = inst;
     }
     generator->ir_tail = inst;
     
     return inst;
 }
 
 // Generate a new unique label
 static char* new_label(CodeGenerator *generator) {
     char *label = (char*)malloc(32);
     if (!label) return NULL;
     
     snprintf(label, 32, "L%d", generator->label_count++);
     return label;
 }
 
 // Generate a new temporary register
 static int new_temp_reg(CodeGenerator *generator) {
     return generator->temp_reg_count++;
 }
 
 // Create an integer operand
 static IROperand make_int_operand(int value) {
     IROperand op;
     op.type = OP_INTEGER;
     op.value.integer = value;
     return op;
 }
 
 // Create a string operand
 static IROperand make_string_operand(const char *value) {
     IROperand op;
     op.type = OP_STRING;
     op.value.string = strdup(value);
     return op;
 }
 
 // Create a variable operand
 static IROperand make_var_operand(const char *name) {
     IROperand op;
     op.type = OP_VARIABLE;
     op.value.string = strdup(name);
     return op;
 }
 
 // Create a label operand
 static IROperand make_label_operand(const char *label) {
     IROperand op;
     op.type = OP_LABEL;
     op.value.string = strdup(label);
     return op;
 }
 
 // Create a register operand
 static IROperand make_reg_operand(int reg) {
     IROperand op;
     op.type = OP_REGISTER;
     op.value.reg = reg;
     return op;
 }
 
 // Generate code to evaluate an expression (returns register holding result)
 static int gen_expr(CodeGenerator *generator, ASTNode *node) {
     if (!node) return -1;
     
     int result_reg = -1;
     
     switch (node->type) {
         case AST_INTEGER: {
             // Load integer constant into a register
             int reg = new_temp_reg(generator);
             IRInst *inst = add_ir_inst(generator, IR_LOAD);
             inst->dest = make_reg_operand(reg);
             inst->src1 = make_int_operand(node->data.integer.value);
             result_reg = reg;
             break;
         }
         
         case AST_CHARACTER: {
             // Load character constant into a register
             int reg = new_temp_reg(generator);
             IRInst *inst = add_ir_inst(generator, IR_LOAD);
             inst->dest = make_reg_operand(reg);
             inst->src1 = make_int_operand(node->data.character.value);
             result_reg = reg;
             break;
         }
         
         case AST_IDENTIFIER: {
             // Load variable value into a register
             int reg = new_temp_reg(generator);
             IRInst *inst = add_ir_inst(generator, IR_LOAD);
             inst->dest = make_reg_operand(reg);
             inst->src1 = make_var_operand(node->data.identifier.name);
             result_reg = reg;
             break;
         }
         
         case AST_STRING: {
             // Load address of string constant into a register
             int reg = new_temp_reg(generator);
             IRInst *inst = add_ir_inst(generator, IR_LOAD);
             inst->dest = make_reg_operand(reg);
             inst->src1 = make_string_operand(node->data.string.value);
             result_reg = reg;
             break;
         }
         
         case AST_BINARY_EXPR: {
             int left_reg = gen_expr(generator, node->data.binary_expr.left);
             int right_reg = gen_expr(generator, node->data.binary_expr.right);
             
             if (left_reg < 0 || right_reg < 0) {
                 return -1;
             }
             
             int result = new_temp_reg(generator);
             IRInst *inst = NULL;
             
             // Generate appropriate instruction based on operation
             switch (node->data.binary_expr.op) {
                 case OP_ADD:
                     inst = add_ir_inst(generator, IR_ADD);
                     break;
                 case OP_SUBTRACT:
                     inst = add_ir_inst(generator, IR_SUB);
                     break;
                 case OP_MULTIPLY:
                     inst = add_ir_inst(generator, IR_MUL);
                     break;
                 case OP_DIVIDE:
                     inst = add_ir_inst(generator, IR_DIV);
                     break;
                 case OP_MODULO:
                     inst = add_ir_inst(generator, IR_MOD);
                     break;
                 case OP_EQ:
                     inst = add_ir_inst(generator, IR_EQ);
                     break;
                 case OP_NEQ:
                     inst = add_ir_inst(generator, IR_NEQ);
                     break;
                 case OP_LT:
                     inst = add_ir_inst(generator, IR_LT);
                     break;
                 case OP_GT:
                     inst = add_ir_inst(generator, IR_GT);
                     break;
                 case OP_LTE:
                     inst = add_ir_inst(generator, IR_LTE);
                     break;
                 case OP_GTE:
                     inst = add_ir_inst(generator, IR_GTE);
                     break;
                 case OP_LOGICAL_AND:
                     inst = add_ir_inst(generator, IR_AND);
                     break;
                 case OP_LOGICAL_OR:
                     inst = add_ir_inst(generator, IR_OR);
                     break;
                 default:
                     fprintf(stderr, "Unsupported binary operator: %d\n", node->data.binary_expr.op);
                     return -1;
             }
             
             if (inst) {
                 inst->dest = make_reg_operand(result);
                 inst->src1 = make_reg_operand(left_reg);
                 inst->src2 = make_reg_operand(right_reg);
             }
             
             result_reg = result;
             break;
         }
         
         case AST_UNARY_EXPR: {
             int operand_reg = gen_expr(generator, node->data.unary_expr.operand);
             
             if (operand_reg < 0) {
                 return -1;
             }
             
             int result = new_temp_reg(generator);
             IRInst *inst = NULL;
             
             // Generate appropriate instruction based on operation
             switch (node->data.unary_expr.op) {
                 case OP_NEGATE:
                     inst = add_ir_inst(generator, IR_NEG);
                     break;
                 case OP_NOT:
                     inst = add_ir_inst(generator, IR_NOT);
                     break;
                 default:
                     fprintf(stderr, "Unsupported unary operator: %d\n", node->data.unary_expr.op);
                     return -1;
             }
             
             if (inst) {
                 inst->dest = make_reg_operand(result);
                 inst->src1 = make_reg_operand(operand_reg);
             }
             
             result_reg = result;
             break;
         }
         
         case AST_ASSIGN_EXPR: {
             // Check if left side is an identifier
             if (node->data.binary_expr.left->type != AST_IDENTIFIER) {
                 fprintf(stderr, "Left side of assignment must be an identifier\n");
                 return -1;
             }
             
             // Evaluate right side expression
             int right_reg = gen_expr(generator, node->data.binary_expr.right);
             
             if (right_reg < 0) {
                 return -1;
             }
             
             // Store result into the variable
             IRInst *inst = add_ir_inst(generator, IR_STORE);
             inst->dest = make_var_operand(node->data.binary_expr.left->data.identifier.name);
             inst->src1 = make_reg_operand(right_reg);
             
             // Assignment also returns the assigned value
             result_reg = right_reg;
             break;
         }
         
         case AST_CALL_EXPR: {
             // Check if function is an identifier
             if (node->data.call_expr.function->type != AST_IDENTIFIER) {
                 fprintf(stderr, "Function call target must be an identifier\n");
                 return -1;
             }
             
             const char *func_name = node->data.call_expr.function->data.identifier.name;
             
             // Process arguments if any
             if (node->data.call_expr.arguments) {
                 for (int i = 0; i < node->data.call_expr.arguments->num_children; i++) {
                     ASTNode *arg = node->data.call_expr.arguments->children[i];
                     int arg_reg = gen_expr(generator, arg);
                     
                     if (arg_reg < 0) {
                         return -1;
                     }
                     
                     // Generate argument instruction
                     IRInst *arg_inst = add_ir_inst(generator, IR_ARG);
                     arg_inst->dest = make_int_operand(i);  // Argument index
                     arg_inst->src1 = make_reg_operand(arg_reg);
                 }
             }
             
             // Generate call instruction
             int result = new_temp_reg(generator);
             IRInst *call_inst = add_ir_inst(generator, IR_CALL);
             call_inst->dest = make_reg_operand(result);
             call_inst->src1 = make_string_operand(func_name);
             
             result_reg = result;
             break;
         }
         
         default:
             fprintf(stderr, "Unsupported expression type: %d\n", node->type);
             return -1;
     }
     
     return result_reg;
 }
 
 // Generate code for an if statement
 static void gen_if_stmt(CodeGenerator *generator, ASTNode *node) {
     if (!node || node->type != AST_IF_STMT) return;
     
     char *else_label = new_label(generator);
     char *end_label = new_label(generator);
     
     // Generate condition code
     int cond_reg = gen_expr(generator, node->data.if_stmt.condition);
     
     if (cond_reg < 0) {
         free(else_label);
         free(end_label);
         return;
     }
     
     // Branch if condition is false
     IRInst *branch = add_ir_inst(generator, IR_JMPZ);
     branch->src1 = make_reg_operand(cond_reg);
     branch->src2 = make_label_operand(else_label);
     
     // Generate 'if' branch code
     gen_ir(generator, node->data.if_stmt.if_branch);
     
     // Jump to end after 'if' branch
     IRInst *jmp_end = add_ir_inst(generator, IR_JMP);
     jmp_end->src1 = make_label_operand(end_label);
     
     // 'else' label
     IRInst *else_inst = add_ir_inst(generator, IR_LABEL);
     else_inst->src1 = make_label_operand(else_label);
     
     // Generate 'else' branch code if it exists
     if (node->data.if_stmt.else_branch) {
         gen_ir(generator, node->data.if_stmt.else_branch);
     }
     
     // End label
     IRInst *end_inst = add_ir_inst(generator, IR_LABEL);
     end_inst->src1 = make_label_operand(end_label);
     
     free(else_label);
     free(end_label);
 }
 
 // Generate code for a while statement
 static void gen_while_stmt(CodeGenerator *generator, ASTNode *node) {
     if (!node || node->type != AST_WHILE_STMT) return;
     
     char *start_label = new_label(generator);
     char *end_label = new_label(generator);
     
     // Start label
     IRInst *start_inst = add_ir_inst(generator, IR_LABEL);
     start_inst->src1 = make_label_operand(start_label);
     
     // Generate condition code
     int cond_reg = gen_expr(generator, node->data.while_stmt.condition);
     
     if (cond_reg < 0) {
         free(start_label);
         free(end_label);
         return;
     }
     
     // Branch if condition is false
     IRInst *branch = add_ir_inst(generator, IR_JMPZ);
     branch->src1 = make_reg_operand(cond_reg);
     branch->src2 = make_label_operand(end_label);
     
     // Generate loop body
     gen_ir(generator, node->data.while_stmt.body);
     
     // Jump back to condition
     IRInst *jmp_start = add_ir_inst(generator, IR_JMP);
     jmp_start->src1 = make_label_operand(start_label);
     
     // End label
     IRInst *end_inst = add_ir_inst(generator, IR_LABEL);
     end_inst->src1 = make_label_operand(end_label);
     
     free(start_label);
     free(end_label);
 }
 
 // Generate code for a return statement
 static void gen_return_stmt(CodeGenerator *generator, ASTNode *node) {
     if (!node || node->type != AST_RETURN_STMT) return;
     
     // Generate expression code if return has a value
     if (node->data.return_stmt.value) {
         int value_reg = gen_expr(generator, node->data.return_stmt.value);
         
         if (value_reg < 0) {
             return;
         }
         
         // Return value
         IRInst *ret_inst = add_ir_inst(generator, IR_RET);
         ret_inst->src1 = make_reg_operand(value_reg);
     } else {
         // Return without value
         add_ir_inst(generator, IR_RET);
     }
 }
 
 // Generate code for a variable declaration
 static void gen_variable_decl(CodeGenerator *generator, ASTNode *node) {
     if (!node || node->type != AST_VARIABLE_DECL) return;
     
     const char *var_name = node->data.variable_decl.name;
     
     // Allocate variable
     IRInst *alloc_inst = add_ir_inst(generator, IR_ALLOC);
     alloc_inst->dest = make_var_operand(var_name);
     
     // Initialize variable if there's an initializer
     if (node->data.variable_decl.initializer) {
         int init_reg = gen_expr(generator, node->data.variable_decl.initializer);
         
         if (init_reg < 0) {
             return;
         }
         
         // Store initializer value
         IRInst *store_inst = add_ir_inst(generator, IR_STORE);
         store_inst->dest = make_var_operand(var_name);
         store_inst->src1 = make_reg_operand(init_reg);
     }
 }
 
 // Generate code for a function
 static void gen_function(CodeGenerator *generator, ASTNode *node) {
     if (!node || node->type != AST_FUNCTION) return;
     
     const char *func_name = node->data.function.name;
     
     // Add function label
     IRInst *label_inst = add_ir_inst(generator, IR_LABEL);
     label_inst->src1 = make_string_operand(func_name);
     
     // Generate code for parameters (not needed for IR, handled in symbol table)
     
     // Generate code for function body
     gen_ir(generator, node->data.function.body);
 }
 
 // Print IR instruction as text
 static void print_ir_inst(FILE *output, IRInst *inst) {
     if (!output || !inst) return;
     
     // Helper function to print an operand
     void print_operand(IROperand op) {
         switch (op.type) {
             case OP_NONE:
                 fprintf(output, "<none>");
                 break;
             case OP_INTEGER:
                 fprintf(output, "%d", op.value.integer);
                 break;
             case OP_STRING:
             case OP_VARIABLE:
             case OP_LABEL:
                 fprintf(output, "%s", op.value.string);
                 break;
             case OP_REGISTER:
                 fprintf(output, "r%d", op.value.reg);
                 break;
         }
     }
     
    // Print instruction based on type
     switch (inst->type) {
         case IR_LABEL:
             fprintf(output, "%s:\n", inst->src1.value.string);
             break;
             
         case IR_LOAD:
             fprintf(output, "    ");
             print_operand(inst->dest);
             fprintf(output, " = ");
             print_operand(inst->src1);
             fprintf(output, "\n");
             break;
             
         case IR_STORE:
             fprintf(output, "    ");
             print_operand(inst->dest);
             fprintf(output, " = ");
             print_operand(inst->src1);
             fprintf(output, "\n");
             break;
             
         case IR_ADD:
             fprintf(output, "    ");
             print_operand(inst->dest);
             fprintf(output, " = ");
             print_operand(inst->src1);
             fprintf(output, " + ");
             print_operand(inst->src2);
             fprintf(output, "\n");
             break;
             
         case IR_SUB:
             fprintf(output, "    ");
             print_operand(inst->dest);
             fprintf(output, " = ");
             print_operand(inst->src1);
             fprintf(output, " - ");
             print_operand(inst->src2);
             fprintf(output, "\n");
             break;
             
         case IR_MUL:
             fprintf(output, "    ");
             print_operand(inst->dest);
             fprintf(output, " = ");
             print_operand(inst->src1);
             fprintf(output, " * ");
             print_operand(inst->src2);
             fprintf(output, "\n");
             break;
             
         case IR_DIV:
             fprintf(output, "    ");
             print_operand(inst->dest);
             fprintf(output, " = ");
             print_operand(inst->src1);
             fprintf(output, " / ");
             print_operand(inst->src2);
             fprintf(output, "\n");
             break;
             
         case IR_MOD:
             fprintf(output, "    ");
             print_operand(inst->dest);
             fprintf(output, " = ");
             print_operand(inst->src1);
             fprintf(output, " %% ");
             print_operand(inst->src2);
             fprintf(output, "\n");
             break;
             
         case IR_NEG:
             fprintf(output, "    ");
             print_operand(inst->dest);
             fprintf(output, " = -");
             print_operand(inst->src1);
             fprintf(output, "\n");
             break;
             
         case IR_AND:
             fprintf(output, "    ");
             print_operand(inst->dest);
             fprintf(output, " = ");
             print_operand(inst->src1);
             fprintf(output, " && ");
             print_operand(inst->src2);
             fprintf(output, "\n");
             break;
             
         case IR_OR:
             fprintf(output, "    ");
             print_operand(inst->dest);
             fprintf(output, " = ");
             print_operand(inst->src1);
             fprintf(output, " || ");
             print_operand(inst->src2);
             fprintf(output, "\n");
             break;
             
         case IR_NOT:
             fprintf(output, "    ");
             print_operand(inst->dest);
             fprintf(output, " = !");
             print_operand(inst->src1);
             fprintf(output, "\n");
             break;
             
         case IR_EQ:
             fprintf(output, "    ");
             print_operand(inst->dest);
             fprintf(output, " = ");
             print_operand(inst->src1);
             fprintf(output, " == ");
             print_operand(inst->src2);
             fprintf(output, "\n");
             break;
             
         case IR_NEQ:
             fprintf(output, "    ");
             print_operand(inst->dest);
             fprintf(output, " = ");
             print_operand(inst->src1);
             fprintf(output, " != ");
             print_operand(inst->src2);
             fprintf(output, "\n");
             break;
             
         case IR_LT:
             fprintf(output, "    ");
             print_operand(inst->dest);
             fprintf(output, " = ");
             print_operand(inst->src1);
             fprintf(output, " < ");
             print_operand(inst->src2);
             fprintf(output, "\n");
             break;
             
         case IR_GT:
             fprintf(output, "    ");
             print_operand(inst->dest);
             fprintf(output, " = ");
             print_operand(inst->src1);
             fprintf(output, " > ");
             print_operand(inst->src2);
             fprintf(output, "\n");
             break;
             
         case IR_LTE:
             fprintf(output, "    ");
             print_operand(inst->dest);
             fprintf(output, " = ");
             print_operand(inst->src1);
             fprintf(output, " <= ");
             print_operand(inst->src2);
             fprintf(output, "\n");
             break;
             
         case IR_GTE:
             fprintf(output, "    ");
             print_operand(inst->dest);
             fprintf(output, " = ");
             print_operand(inst->src1);
             fprintf(output, " >= ");
             print_operand(inst->src2);
             fprintf(output, "\n");
             break;
             
         case IR_JMP:
             fprintf(output, "    goto ");
             print_operand(inst->src1);
             fprintf(output, "\n");
             break;
             
         case IR_JMPZ:
             fprintf(output, "    if (");
             print_operand(inst->src1);
             fprintf(output, " == 0) goto ");
             print_operand(inst->src2);
             fprintf(output, "\n");
             break;
             
         case IR_JMPNZ:
             fprintf(output, "    if (");
             print_operand(inst->src1);
             fprintf(output, " != 0) goto ");
             print_operand(inst->src2);
             fprintf(output, "\n");
             break;
             
         case IR_CALL:
             fprintf(output, "    ");
             print_operand(inst->dest);
             fprintf(output, " = call ");
             print_operand(inst->src1);
             fprintf(output, "\n");
             break;
             
         case IR_RET:
             fprintf(output, "    return ");
             if (inst->src1.type != OP_NONE) {
                 print_operand(inst->src1);
             }
             fprintf(output, "\n");
             break;
             
         case IR_ARG:
             fprintf(output, "    arg[");
             print_operand(inst->dest);
             fprintf(output, "] = ");
             print_operand(inst->src1);
             fprintf(output, "\n");
             break;
             
         case IR_ALLOC:
             fprintf(output, "    alloc ");
             print_operand(inst->dest);
             fprintf(output, "\n");
             break;
             
         default:
             fprintf(output, "    <unknown instruction>\n");
             break;
     }
 }
 
 // Print IR instructions
 void print_ir(FILE *output, IRInst *ir) {
     if (!output) return;
     
     fprintf(output, "# Intermediate Representation\n");
     
     for (IRInst *inst = ir; inst != NULL; inst = inst->next) {
         print_ir_inst(output, inst);
     }
 }
 
 // Generate ARM assembly code
 static void generate_arm_asm(FILE *output, IRInst *ir) {
     if (!output || !ir) return;
     
     // Emit assembly header
     fprintf(output, "# Generated ARM 32-bit assembly code\n");
     fprintf(output, ".text\n");
     fprintf(output, ".syntax unified\n");
     fprintf(output, ".arm\n"); // Use ARM instruction set, not Thumb
     
     // Collect information about variables and function parameters
     // In a real compiler, we'd use the symbol table for this
     const char *var_names[] = {"n", "a", "b", "i", "result", "x", "y"};
     int var_count = sizeof(var_names) / sizeof(var_names[0]);
     
     // Process IR code
     for (IRInst *inst = ir; inst; inst = inst->next) {
         switch (inst->type) {
             case IR_LABEL:
                 fprintf(output, "%s:\n", inst->src1.value.string);
                 break;
                 
             case IR_LOAD:
                 if (inst->src1.type == OP_INTEGER) {
                     // Load immediate value
                     fprintf(output, "    mov r%d, #%d\n", 
                            inst->dest.value.reg % 10, 
                            inst->src1.value.integer);
                 } else if (inst->src1.type == OP_VARIABLE) {
                     // Load from memory
                     fprintf(output, "    ldr r%d, =%s\n", 
                            inst->dest.value.reg % 10, 
                            inst->src1.value.string);
                     fprintf(output, "    ldr r%d, [r%d]\n", 
                            inst->dest.value.reg % 10,
                            inst->dest.value.reg % 10);
                 }
                 break;
                 
             case IR_STORE:
                 if (inst->src1.type == OP_REGISTER) {
                     // Store to memory
                     fprintf(output, "    ldr r12, =%s\n", inst->dest.value.string);
                     fprintf(output, "    str r%d, [r12]\n", 
                            inst->src1.value.reg % 10);
                 }
                 break;
                 
             case IR_ADD:
                 fprintf(output, "    add r%d, r%d, r%d\n", 
                        inst->dest.value.reg % 10,
                        inst->src1.value.reg % 10, 
                        inst->src2.value.reg % 10);
                 break;
                 
             case IR_SUB:
                 fprintf(output, "    sub r%d, r%d, r%d\n", 
                        inst->dest.value.reg % 10,
                        inst->src1.value.reg % 10, 
                        inst->src2.value.reg % 10);
                 break;
                 
             case IR_MUL:
                 fprintf(output, "    mul r%d, r%d, r%d\n", 
                        inst->dest.value.reg % 10,
                        inst->src1.value.reg % 10, 
                        inst->src2.value.reg % 10);
                 break;
                 
             case IR_DIV:
                 fprintf(output, "    # Division in ARM requires library calls\n");
                 fprintf(output, "    # Using simplified DIV for demo\n");
                 fprintf(output, "    mov r0, r%d\n", inst->src1.value.reg % 10);
                 fprintf(output, "    mov r1, r%d\n", inst->src2.value.reg % 10);
                 fprintf(output, "    bl __aeabi_idiv\n");
                 fprintf(output, "    mov r%d, r0\n", inst->dest.value.reg % 10);
                 break;
                 
             case IR_MOD:
                 fprintf(output, "    # Modulo in ARM requires library calls\n");
                 fprintf(output, "    # Using simplified MOD for demo\n");
                 fprintf(output, "    mov r0, r%d\n", inst->src1.value.reg % 10);
                 fprintf(output, "    mov r1, r%d\n", inst->src2.value.reg % 10);
                 fprintf(output, "    bl __aeabi_idivmod\n");
                 fprintf(output, "    mov r%d, r1\n", inst->dest.value.reg % 10);
                 break;
                 
             case IR_NEG:
                 fprintf(output, "    rsb r%d, r%d, #0\n", 
                        inst->dest.value.reg % 10,
                        inst->src1.value.reg % 10);
                 break;
                 
             case IR_EQ:
                 fprintf(output, "    cmp r%d, r%d\n", 
                        inst->src1.value.reg % 10,
                        inst->src2.value.reg % 10);
                 fprintf(output, "    moveq r%d, #1\n", inst->dest.value.reg % 10);
                 fprintf(output, "    movne r%d, #0\n", inst->dest.value.reg % 10);
                 break;
                 
             case IR_NEQ:
                 fprintf(output, "    cmp r%d, r%d\n", 
                        inst->src1.value.reg % 10,
                        inst->src2.value.reg % 10);
                 fprintf(output, "    movne r%d, #1\n", inst->dest.value.reg % 10);
                 fprintf(output, "    moveq r%d, #0\n", inst->dest.value.reg % 10);
                 break;
                 
             case IR_LT:
                 fprintf(output, "    cmp r%d, r%d\n", 
                        inst->src1.value.reg % 10,
                        inst->src2.value.reg % 10);
                 fprintf(output, "    movlt r%d, #1\n", inst->dest.value.reg % 10);
                 fprintf(output, "    movge r%d, #0\n", inst->dest.value.reg % 10);
                 break;
                 
             case IR_GT:
                 fprintf(output, "    cmp r%d, r%d\n", 
                        inst->src1.value.reg % 10,
                        inst->src2.value.reg % 10);
                 fprintf(output, "    movgt r%d, #1\n", inst->dest.value.reg % 10);
                 fprintf(output, "    movle r%d, #0\n", inst->dest.value.reg % 10);
                 break;
                 
             case IR_LTE:
                 fprintf(output, "    cmp r%d, r%d\n", 
                        inst->src1.value.reg % 10,
                        inst->src2.value.reg % 10);
                 fprintf(output, "    movle r%d, #1\n", inst->dest.value.reg % 10);
                 fprintf(output, "    movgt r%d, #0\n", inst->dest.value.reg % 10);
                 break;
                 
             case IR_GTE:
                 fprintf(output, "    cmp r%d, r%d\n", 
                        inst->src1.value.reg % 10,
                        inst->src2.value.reg % 10);
                 fprintf(output, "    movge r%d, #1\n", inst->dest.value.reg % 10);
                 fprintf(output, "    movlt r%d, #0\n", inst->dest.value.reg % 10);
                 break;
                 
             case IR_JMP:
                 fprintf(output, "    b %s\n", inst->src1.value.string);
                 break;
                 
             case IR_JMPZ:
                 fprintf(output, "    cmp r%d, #0\n", inst->src1.value.reg % 10);
                 fprintf(output, "    beq %s\n", inst->src2.value.string);
                 break;
                 
             case IR_JMPNZ:
                 fprintf(output, "    cmp r%d, #0\n", inst->src1.value.reg % 10);
                 fprintf(output, "    bne %s\n", inst->src2.value.string);
                 break;
                 
             case IR_CALL:
                 fprintf(output, "    bl %s\n", inst->src1.value.string);
                 fprintf(output, "    mov r%d, r0\n", inst->dest.value.reg % 10);
                 break;
                 
             case IR_ARG:
                 // In ARM EABI, first 4 args in r0-r3, rest on stack
                 if (inst->dest.value.integer < 4) {
                     fprintf(output, "    mov r%d, r%d\n", 
                            inst->dest.value.integer,
                            inst->src1.value.reg % 10);
                 } else {
                     fprintf(output, "    # Stack args not properly implemented\n");
                     fprintf(output, "    push {r%d}\n", inst->src1.value.reg % 10);
                 }
                 break;
                 
             case IR_RET:
                 if (inst->src1.type == OP_REGISTER) {
                     fprintf(output, "    mov r0, r%d\n", inst->src1.value.reg % 10);
                 }
                 fprintf(output, "    bx lr\n");
                 break;
                 
             case IR_ALLOC:
                 fprintf(output, "    # Variable %s is allocated in data section\n", 
                        inst->dest.value.string);
                 break;
                 
             case IR_AND:
                 fprintf(output, "    cmp r%d, #0\n", inst->src1.value.reg % 10);
                 fprintf(output, "    moveq r%d, #0\n", inst->dest.value.reg % 10);
                 fprintf(output, "    bne 1f\n");
                 fprintf(output, "    b 2f\n");
                 fprintf(output, "1:\n");
                 fprintf(output, "    cmp r%d, #0\n", inst->src2.value.reg % 10);
                 fprintf(output, "    movne r%d, #1\n", inst->dest.value.reg % 10);
                 fprintf(output, "    moveq r%d, #0\n", inst->dest.value.reg % 10);
                 fprintf(output, "2:\n");
                 break;
                 
             case IR_OR:
                 fprintf(output, "    cmp r%d, #0\n", inst->src1.value.reg % 10);
                 fprintf(output, "    movne r%d, #1\n", inst->dest.value.reg % 10);
                 fprintf(output, "    beq 1f\n");
                 fprintf(output, "    b 2f\n");
                 fprintf(output, "1:\n");
                 fprintf(output, "    cmp r%d, #0\n", inst->src2.value.reg % 10);
                 fprintf(output, "    movne r%d, #1\n", inst->dest.value.reg % 10);
                 fprintf(output, "    moveq r%d, #0\n", inst->dest.value.reg % 10);
                 fprintf(output, "2:\n");
                 break;
                 
             case IR_NOT:
                 fprintf(output, "    cmp r%d, #0\n", inst->src1.value.reg % 10);
                 fprintf(output, "    moveq r%d, #1\n", inst->dest.value.reg % 10);
                 fprintf(output, "    movne r%d, #0\n", inst->dest.value.reg % 10);
                 break;
                 
             default:
                 fprintf(output, "    # Unimplemented instruction: %d\n", inst->type);
                 break;
         }
     }
     
     // Add data section for variables
     fprintf(output, "\n.data\n");
     
     // Define all variables
     for (int i = 0; i < var_count; i++) {
         fprintf(output, "%s: .word 0\n", var_names[i]);
     }
 }
 
// Main entry point for code generation
 int generate_code(ASTNode *ast, FILE *output, int generate_assembly) {
     // Parameter 'target' is not used since we only support ARM
     if (!ast || !output) return 0;
     
     // Initialize symbol table
     SymbolTable *symtab = init_symbol_table(211);  // Prime number for hash table size
     if (!symtab) {
         error_report(NULL, "Failed to initialize symbol table");
         return 0;
     }
     
     // Initialize code generator with ARM target
     CodeGenerator *generator = init_code_generator(symtab);
     if (!generator) {
         error_report(NULL, "Failed to initialize code generator");
         free_symbol_table(symtab);
         return 0;
     }
     
     // Generate IR code
     generate_ir(generator, ast);
     
     // Generate target code
     int success = 0;
     if (generate_assembly) {
         // Print the IR as assembly
         print_ir(output, generator->ir_head);
         // Generate ARM assembly
         generate_arm_asm(output, generator->ir_head);
         success = 1;
     } else {
         // Generate machine code - not implemented yet
         fprintf(output, "Machine code generation not implemented yet.\n");
         success = 0;
     }
     
     // Clean up
     free_code_generator(generator);
     free_symbol_table(symtab);
     
     return success;
 }
 
 // Generate assembly code (temporary dummy implementation)
 int generate_assembly(CodeGenerator *generator, FILE *output) {
     if (!generator || !output) return 0;
     
     // Print IR as assembly for now
     print_ir(output, generator->ir_head);
     
     return 1;
 }
 
 // Generate machine code (temporary dummy implementation)
 int generate_machine_code(CodeGenerator *generator, FILE *output) {
     if (!generator || !output) return 0;
     
     // Not implemented yet
     fprintf(output, "Machine code generation not implemented yet.\n");
     
     return 0;
 }