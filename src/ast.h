#ifndef AST_H
#define AST_H

// AST node types
typedef enum {
    AST_PROGRAM,
    AST_FUNCTION,
    AST_PARAM_LIST,
    AST_PARAMETER,
    AST_COMPOUND_STMT,
    AST_VARIABLE_DECL,
    AST_ASSIGN_EXPR,
    AST_IF_STMT,
    AST_WHILE_STMT,
    AST_RETURN_STMT,
    AST_EXPR_STMT,
    AST_BINARY_EXPR,
    AST_UNARY_EXPR,
    AST_CALL_EXPR,
    AST_ARG_LIST,
    AST_SUBSCRIPT_EXPR,
    AST_IDENTIFIER,
    AST_INTEGER,
    AST_CHARACTER,
    AST_STRING
} ASTNodeType;

// Data types
typedef enum {
    TYPE_VOID,
    TYPE_INT,
    TYPE_CHAR
} DataType;

// Binary operators
typedef enum {
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_MODULO,
    OP_EQ,
    OP_NEQ,
    OP_LT,
    OP_GT,
    OP_LTE,
    OP_GTE,
    OP_LOGICAL_AND,
    OP_LOGICAL_OR,
    OP_BITWISE_AND,
    OP_BITWISE_OR,
    OP_BITWISE_XOR,
    OP_SHL,
    OP_SHR
} BinaryOp;

// Unary operators
typedef enum {
    OP_NEGATE,
    OP_NOT,
    OP_BITWISE_NOT,
    OP_PRE_INC,
    OP_PRE_DEC,
    OP_POST_INC,
    OP_POST_DEC
} UnaryOp;

typedef struct ASTNode ASTNode;

// Program node
typedef struct {
    // Children are top-level declarations and functions
} ProgramData;

// Function node
typedef struct {
    char *name;
    DataType return_type;
    ASTNode *parameters;  // AST_PARAM_LIST
    ASTNode *body;        // AST_COMPOUND_STMT
} FunctionData;

// Parameter list node
typedef struct {
    // Children are parameters
} ParamListData;

// Parameter node
typedef struct {
    char *name;
    DataType type;
    int is_array;
} ParameterData;

// Compound statement node
typedef struct {
    // Children are statements
} CompoundStmtData;

// Variable declaration node
typedef struct {
    char *name;
    DataType type;
    int is_array;
    int array_size;
    ASTNode *initializer;
} VariableDeclData;

// If statement node
typedef struct {
    ASTNode *condition;
    ASTNode *if_branch;
    ASTNode *else_branch;  // Can be NULL
} IfStmtData;

// While statement node
typedef struct {
    ASTNode *condition;
    ASTNode *body;
} WhileStmtData;

// Return statement node
typedef struct {
    ASTNode *value;  // Can be NULL for void return
} ReturnStmtData;

// Expression statement node
typedef struct {
    // Child is the expression
} ExprStmtData;

// Binary expression node
typedef struct {
    BinaryOp op;
    ASTNode *left;
    ASTNode *right;
} BinaryExprData;

// Unary expression node
typedef struct {
    UnaryOp op;
    ASTNode *operand;
} UnaryExprData;

// Function call node
typedef struct {
    ASTNode *function;   // Function expression (usually identifier)
    ASTNode *arguments;  // AST_ARG_LIST
} CallExprData;

// Argument list node
typedef struct {
    // Children are expressions
} ArgListData;

// Array subscript node
typedef struct {
    ASTNode *array;
    ASTNode *index;
} SubscriptExprData;

// Identifier node
typedef struct {
    char *name;
} IdentifierData;

// Integer literal node
typedef struct {
    int value;
} IntegerData;

// Character literal node
typedef struct {
    char value;
} CharacterData;

// String literal node
typedef struct {
    char *value;
} StringData;

// AST node structure
struct ASTNode {
    ASTNodeType type;
    union {
        ProgramData program;
        FunctionData function;
        ParamListData param_list;
        ParameterData parameter;
        CompoundStmtData compound_stmt;
        VariableDeclData variable_decl;
        IfStmtData if_stmt;
        WhileStmtData while_stmt;
        ReturnStmtData return_stmt;
        ExprStmtData expr_stmt;
        BinaryExprData binary_expr;
        UnaryExprData unary_expr;
        CallExprData call_expr;
        ArgListData arg_list;
        SubscriptExprData subscript_expr;
        IdentifierData identifier;
        IntegerData integer;
        CharacterData character;
        StringData string;
    } data;
    ASTNode **children;
    int num_children;
    int children_capacity;
};

// Function prototypes
ASTNode* create_ast_node(ASTNodeType type);
void free_ast(ASTNode *node);
void add_child(ASTNode *parent, ASTNode *child);
void print_ast(ASTNode *node, int indent);

#endif