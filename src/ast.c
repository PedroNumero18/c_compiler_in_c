/** Abstract Syntax Tree Implementation 
 * Implements functions for creating, manipulating, and freeing
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

// Initial capacity for child nodes
#define INITIAL_CHILDREN_CAPACITY 4

// Create a new AST node of the specified type
ASTNode* create_ast_node(ASTNodeType type) {
    ASTNode *node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->type = type;
    node->children = NULL;
    node->num_children = 0;
    node->children_capacity = 0;
    
    // Initialize data based on node type
    switch (type) {
        case AST_PROGRAM:
        case AST_PARAM_LIST:
        case AST_COMPOUND_STMT:
        case AST_EXPR_STMT:
        case AST_ARG_LIST:
            // These nodes can have multiple children, allocate array
            node->children = (ASTNode**)malloc(INITIAL_CHILDREN_CAPACITY * sizeof(ASTNode*));
            if (!node->children) {
                free(node);
                return NULL;
            }
            node->children_capacity = INITIAL_CHILDREN_CAPACITY;
            break;
        
        case AST_FUNCTION:
            node->data.function.name = NULL;
            node->data.function.parameters = NULL;
            node->data.function.body = NULL;
            break;
        
        case AST_PARAMETER:
            node->data.parameter.name = NULL;
            node->data.parameter.is_array = 0;
            break;
        
        case AST_VARIABLE_DECL:
            node->data.variable_decl.name = NULL;
            node->data.variable_decl.is_array = 0;
            node->data.variable_decl.array_size = 0;
            node->data.variable_decl.initializer = NULL;
            break;
        
        case AST_IF_STMT:
            node->data.if_stmt.condition = NULL;
            node->data.if_stmt.if_branch = NULL;
            node->data.if_stmt.else_branch = NULL;
            break;
        
        case AST_WHILE_STMT:
            node->data.while_stmt.condition = NULL;
            node->data.while_stmt.body = NULL;
            break;
        
        case AST_RETURN_STMT:
            node->data.return_stmt.value = NULL;
            break;
        
        case AST_BINARY_EXPR:
        case AST_ASSIGN_EXPR:
            node->data.binary_expr.left = NULL;
            node->data.binary_expr.right = NULL;
            break;
        
        case AST_UNARY_EXPR:
            node->data.unary_expr.operand = NULL;
            break;
        
        case AST_CALL_EXPR:
            node->data.call_expr.function = NULL;
            node->data.call_expr.arguments = NULL;
            break;
        
        case AST_SUBSCRIPT_EXPR:
            node->data.subscript_expr.array = NULL;
            node->data.subscript_expr.index = NULL;
            break;
        
        case AST_IDENTIFIER:
            node->data.identifier.name = NULL;
            break;
        
        case AST_STRING:
            node->data.string.value = NULL;
            break;
        
        default:
            // Other node types don't need special initialization
            break;
    }
    
    return node;
}

// Free an AST node and all its children
void free_ast(ASTNode *node) {
    if (!node) return;
    
    // Free children first
    if (node->children) {
        for (int i = 0; i < node->num_children; i++) {
            free_ast(node->children[i]);
        }
        free(node->children);
    }
    
    // Free node-specific data
    switch (node->type) {
        case AST_FUNCTION:
            if (node->data.function.name) free(node->data.function.name);
            if (node->data.function.parameters) free_ast(node->data.function.parameters);
            if (node->data.function.body) free_ast(node->data.function.body);
            break;
        
        case AST_PARAMETER:
            if (node->data.parameter.name) free(node->data.parameter.name);
            break;
        
        case AST_VARIABLE_DECL:
            if (node->data.variable_decl.name) free(node->data.variable_decl.name);
            if (node->data.variable_decl.initializer) free_ast(node->data.variable_decl.initializer);
            break;
        
        case AST_IF_STMT:
            if (node->data.if_stmt.condition) free_ast(node->data.if_stmt.condition);
            if (node->data.if_stmt.if_branch) free_ast(node->data.if_stmt.if_branch);
            if (node->data.if_stmt.else_branch) free_ast(node->data.if_stmt.else_branch);
            break;
        
        case AST_WHILE_STMT:
            if (node->data.while_stmt.condition) free_ast(node->data.while_stmt.condition);
            if (node->data.while_stmt.body) free_ast(node->data.while_stmt.body);
            break;
        
        case AST_RETURN_STMT:
            if (node->data.return_stmt.value) free_ast(node->data.return_stmt.value);
            break;
        
        case AST_BINARY_EXPR:
        case AST_ASSIGN_EXPR:
            if (node->data.binary_expr.left) free_ast(node->data.binary_expr.left);
            if (node->data.binary_expr.right) free_ast(node->data.binary_expr.right);
            break;
        
        case AST_UNARY_EXPR:
            if (node->data.unary_expr.operand) free_ast(node->data.unary_expr.operand);
            break;
        
        case AST_CALL_EXPR:
            if (node->data.call_expr.function) free_ast(node->data.call_expr.function);
            if (node->data.call_expr.arguments) free_ast(node->data.call_expr.arguments);
            break;
        
        case AST_SUBSCRIPT_EXPR:
            if (node->data.subscript_expr.array) free_ast(node->data.subscript_expr.array);
            if (node->data.subscript_expr.index) free_ast(node->data.subscript_expr.index);
            break;
        
        case AST_IDENTIFIER:
            if (node->data.identifier.name) free(node->data.identifier.name);
            break;
        
        case AST_STRING:
            if (node->data.string.value) free(node->data.string.value);
            break;
        
        default:
            // Other node types don't have pointers to free
            break;
    }
    
    // Free the node itself
    free(node);
}

// Add a child node to a parent node
void add_child(ASTNode *parent, ASTNode *child) {
    if (!parent || !child) return;
    
    // Check if we need to resize the children array
    if (parent->num_children >= parent->children_capacity) {
        int new_capacity = parent->children_capacity * 2;
        if (new_capacity == 0) new_capacity = INITIAL_CHILDREN_CAPACITY;
        
        ASTNode **new_children = (ASTNode**)realloc(parent->children, 
                                                   new_capacity * sizeof(ASTNode*));
        if (!new_children) return;
        
        parent->children = new_children;
        parent->children_capacity = new_capacity;
    }
    
    // Add the child
    parent->children[parent->num_children++] = child;
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

// Get string representation of binary operator
static const char* binary_op_str(BinaryOp op) {
    switch (op) {
        case OP_ADD: return "+";
        case OP_SUBTRACT: return "-";
        case OP_MULTIPLY: return "*";
        case OP_DIVIDE: return "/";
        case OP_MODULO: return "%";
        case OP_EQ: return "==";
        case OP_NEQ: return "!=";
        case OP_LT: return "<";
        case OP_GT: return ">";
        case OP_LTE: return "<=";
        case OP_GTE: return ">=";
        case OP_LOGICAL_AND: return "&&";
        case OP_LOGICAL_OR: return "||";
        case OP_BITWISE_AND: return "&";
        case OP_BITWISE_OR: return "|";
        case OP_BITWISE_XOR: return "^";
        case OP_SHL: return "<<";
        case OP_SHR: return ">>";
        default: return "unknown";
    }
}

// Get string representation of unary operator
static const char* unary_op_str(UnaryOp op) {
    switch (op) {
        case OP_NEGATE: return "-";
        case OP_NOT: return "!";
        case OP_BITWISE_NOT: return "~";
        case OP_PRE_INC: return "++";
        case OP_PRE_DEC: return "--";
        case OP_POST_INC: return "++ (post)";
        case OP_POST_DEC: return "-- (post)";
        default: return "unknown";
    }
}

// Print indentation for AST visualization
static void print_indent(int indent) {
    for (int i = 0; i < indent; i++) {
        printf("  ");
    }
}

// Print AST for debugging
void print_ast(ASTNode *node, int indent) {
    if (!node) {
        print_indent(indent);
        printf("NULL\n");
        return;
    }
    
    print_indent(indent);
    
    switch (node->type) {
        case AST_PROGRAM:
            printf("Program (%d children)\n", node->num_children);
            for (int i = 0; i < node->num_children; i++) {
                print_ast(node->children[i], indent + 1);
            }
            break;
        
        case AST_FUNCTION:
            printf("Function: %s, Return Type: %s\n", 
                   node->data.function.name, 
                   data_type_str(node->data.function.return_type));
            
            print_indent(indent + 1);
            printf("Parameters:\n");
            if (node->data.function.parameters) {
                print_ast(node->data.function.parameters, indent + 2);
            } else {
                print_indent(indent + 2);
                printf("(none)\n");
            }
            
            print_indent(indent + 1);
            printf("Body:\n");
            if (node->data.function.body) {
                print_ast(node->data.function.body, indent + 2);
            } else {
                print_indent(indent + 2);
                printf("(none - function declaration only)\n");
            }
            break;
        
        case AST_PARAM_LIST:
            printf("Parameter List (%d parameters)\n", node->num_children);
            for (int i = 0; i < node->num_children; i++) {
                print_ast(node->children[i], indent + 1);
            }
            break;
        
        case AST_PARAMETER:
            printf("Parameter: %s, Type: %s%s\n", 
                   node->data.parameter.name, 
                   data_type_str(node->data.parameter.type),
                   node->data.parameter.is_array ? "[]" : "");
            break;
        
        case AST_COMPOUND_STMT:
            printf("Compound Statement (%d statements)\n", node->num_children);
            for (int i = 0; i < node->num_children; i++) {
                print_ast(node->children[i], indent + 1);
            }
            break;
        
        case AST_VARIABLE_DECL:
            printf("Variable Declaration: %s, Type: %s%s", 
                   node->data.variable_decl.name, 
                   data_type_str(node->data.variable_decl.type),
                   node->data.variable_decl.is_array ? "[]" : "");
            
            if (node->data.variable_decl.is_array && node->data.variable_decl.array_size > 0) {
                printf("[%d]", node->data.variable_decl.array_size);
            }
            
            printf("\n");
            
            if (node->data.variable_decl.initializer) {
                print_indent(indent + 1);
                printf("Initializer:\n");
                print_ast(node->data.variable_decl.initializer, indent + 2);
            }
            break;
        
        case AST_IF_STMT:
            printf("If Statement\n");
            
            print_indent(indent + 1);
            printf("Condition:\n");
            print_ast(node->data.if_stmt.condition, indent + 2);
            
            print_indent(indent + 1);
            printf("If Branch:\n");
            print_ast(node->data.if_stmt.if_branch, indent + 2);
            
            if (node->data.if_stmt.else_branch) {
                print_indent(indent + 1);
                printf("Else Branch:\n");
                print_ast(node->data.if_stmt.else_branch, indent + 2);
            }
            break;
        
        case AST_WHILE_STMT:
            printf("While Statement\n");
            
            print_indent(indent + 1);
            printf("Condition:\n");
            print_ast(node->data.while_stmt.condition, indent + 2);
            
            print_indent(indent + 1);
            printf("Body:\n");
            print_ast(node->data.while_stmt.body, indent + 2);
            break;
        
        case AST_RETURN_STMT:
            printf("Return Statement\n");
            
            if (node->data.return_stmt.value) {
                print_indent(indent + 1);
                printf("Value:\n");
                print_ast(node->data.return_stmt.value, indent + 2);
            }
            break;
        
        case AST_EXPR_STMT:
            printf("Expression Statement\n");
            
            if (node->num_children > 0) {
                print_ast(node->children[0], indent + 1);
            }
            break;
        
        case AST_BINARY_EXPR:
            printf("Binary Expression: %s\n", binary_op_str(node->data.binary_expr.op));
            
            print_indent(indent + 1);
            printf("Left:\n");
            print_ast(node->data.binary_expr.left, indent + 2);
            
            print_indent(indent + 1);
            printf("Right:\n");
            print_ast(node->data.binary_expr.right, indent + 2);
            break;
        
        case AST_ASSIGN_EXPR:
            printf("Assignment Expression\n");
            
            print_indent(indent + 1);
            printf("Left (target):\n");
            print_ast(node->data.binary_expr.left, indent + 2);
            
            print_indent(indent + 1);
            printf("Right (value):\n");
            print_ast(node->data.binary_expr.right, indent + 2);
            break;
        
        case AST_UNARY_EXPR:
            printf("Unary Expression: %s\n", unary_op_str(node->data.unary_expr.op));
            
            print_indent(indent + 1);
            printf("Operand:\n");
            print_ast(node->data.unary_expr.operand, indent + 2);
            break;
        
        case AST_CALL_EXPR:
            printf("Function Call\n");
            
            print_indent(indent + 1);
            printf("Function:\n");
            print_ast(node->data.call_expr.function, indent + 2);
            
            print_indent(indent + 1);
            printf("Arguments:\n");
            if (node->data.call_expr.arguments) {
                print_ast(node->data.call_expr.arguments, indent + 2);
            } else {
                print_indent(indent + 2);
                printf("(none)\n");
            }
            break;
        
        case AST_ARG_LIST:
            printf("Argument List (%d arguments)\n", node->num_children);
            for (int i = 0; i < node->num_children; i++) {
                print_ast(node->children[i], indent + 1);
            }
            break;
        
        case AST_SUBSCRIPT_EXPR:
            printf("Array Subscript\n");
            
            print_indent(indent + 1);
            printf("Array:\n");
            print_ast(node->data.subscript_expr.array, indent + 2);
            
            print_indent(indent + 1);
            printf("Index:\n");
            print_ast(node->data.subscript_expr.index, indent + 2);
            break;
        
        case AST_IDENTIFIER:
            printf("Identifier: %s\n", node->data.identifier.name);
            break;
        
        case AST_INTEGER:
            printf("Integer: %d\n", node->data.integer.value);
            break;
        
        case AST_CHARACTER:
            if (node->data.character.value >= 32 && node->data.character.value <= 126) {
                printf("Character: '%c'\n", node->data.character.value);
            } else {
                printf("Character: '\\x%02X'\n", (unsigned char)node->data.character.value);
            }
            break;
        
        case AST_STRING:
            printf("String: \"%s\"\n", node->data.string.value);
            break;
        
        default:
            printf("Unknown AST node type: %d\n", node->type);
            break;
    }
}
