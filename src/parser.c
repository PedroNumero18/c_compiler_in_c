/**
 * Parser Implementation
 * 
 * Implements a recursive descent parser for a basic set of C.
 * Translates a token stream into an abstract syntax tree (AST).
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include "common.h"
 #include "parser.h"
 #include "ast.h"
 #include "error.h"
 
 // Initialize parser with a lexer
 Parser* init_parser(Lexer *lexer) {
     Parser *parser = (Parser*)malloc(sizeof(Parser));
     if (!parser) return NULL;
     
     parser->lexer = lexer;
     parser->current_token = peek_token(lexer);
     
     return parser;
 }
 
 // Free parser resources
 void free_parser(Parser *parser) {
     if (parser) {
         free(parser);
     }
 }
 
 // Consume current token and advance to next if it matches expected type
 int expect_token(Parser *parser, TokenType type) {
     if (parser->current_token.type == type) {
         advance_token(parser->lexer);
         parser->current_token = peek_token(parser->lexer);
         return 1;
     }
     
     char error_msg[128];
     snprintf(error_msg, sizeof(error_msg), 
              "Expected token %s, got %s", 
              token_type_str(type), 
              token_type_str(parser->current_token.type));
     parser_error(parser, error_msg);
     return 0;
 }
 
 // Check if current token matches expected type, but don't consume
 int match_token(Parser *parser, TokenType type) {
     return parser->current_token.type == type;
 }
 
 // Report parser error
 void parser_error(Parser *parser, const char *message) {
     error_report_location(
         parser->current_token.filename,
         parser->current_token.line,
         parser->current_token.column,
         message
     );
 }
 
 // Parse entire program
 ASTNode* parse_program(Parser *parser) {
     ASTNode *program = create_ast_node(AST_PROGRAM);
     if (!program) return NULL;
     
     // Parse a sequence of function definitions and global declarations
     while (!match_token(parser, TOKEN_EOF)) {
         // Check for preprocessor directives
         if (match_token(parser, TOKEN_POUND)) {
             // Simple handling of #include and #define
             expect_token(parser, TOKEN_POUND);
             
             if (match_token(parser, TOKEN_IDENTIFIER)) {
                 char *directive = strdup(parser->current_token.value);
                 expect_token(parser, TOKEN_IDENTIFIER);
                 
                 // Skip the rest of the line for now (simplistic approach)
                 while (!match_token(parser, TOKEN_EOF) && 
                        !match_token(parser, TOKEN_SEMICOLON)) {
                     advance_token(parser->lexer);
                     parser->current_token = peek_token(parser->lexer);
                 }
                 
                 if (match_token(parser, TOKEN_SEMICOLON)) {
                     expect_token(parser, TOKEN_SEMICOLON);
                 }
                 
                 free(directive);
             }
             continue;
         }
         
         // Type specifier for function or variable
         if (match_token(parser, TOKEN_INT) || 
             match_token(parser, TOKEN_CHAR) || 
             match_token(parser, TOKEN_VOID)) {
             
             TokenType type_token = parser->current_token.type;
             expect_token(parser, type_token);
             
             // Variable or function name
             if (match_token(parser, TOKEN_IDENTIFIER)) {
                 char *identifier = strdup(parser->current_token.value);
                 expect_token(parser, TOKEN_IDENTIFIER);
                 
                 // Function definition
                 if (match_token(parser, TOKEN_LPAREN)) {
                     ASTNode *function = parse_function(parser);
                     if (function) {
                         // Set function return type
                         function->data.function.return_type = type_token == TOKEN_INT ? TYPE_INT : 
                                                              type_token == TOKEN_CHAR ? TYPE_CHAR : 
                                                              TYPE_VOID;
                         function->data.function.name = identifier;
                         add_child(program, function);
                     } else {
                         free(identifier);
                     }
                 }
                 // Global variable declaration
                 else {
                     ASTNode *variable = create_ast_node(AST_VARIABLE_DECL);
                     if (variable) {
                         variable->data.variable_decl.type = type_token == TOKEN_INT ? TYPE_INT : 
                                                            type_token == TOKEN_CHAR ? TYPE_CHAR : 
                                                            TYPE_VOID;
                         variable->data.variable_decl.name = identifier;
                         
                         // Check for array declaration
                         if (match_token(parser, TOKEN_LBRACKET)) {
                             expect_token(parser, TOKEN_LBRACKET);
                             
                             if (match_token(parser, TOKEN_INTEGER)) {
                                 variable->data.variable_decl.is_array = 1;
                                 variable->data.variable_decl.array_size = atoi(parser->current_token.value);
                                 expect_token(parser, TOKEN_INTEGER);
                             }
                             
                             expect_token(parser, TOKEN_RBRACKET);
                         }
                         
                         // Check for initialization
                         if (match_token(parser, TOKEN_ASSIGN)) {
                             expect_token(parser, TOKEN_ASSIGN);
                             
                             ASTNode *initializer = parse_expression(parser);
                             if (initializer) {
                                 variable->data.variable_decl.initializer = initializer;
                             }
                         }
                         
                         expect_token(parser, TOKEN_SEMICOLON);
                         add_child(program, variable);
                     } else {
                         free(identifier);
                     }
                 }
             } else {
                 parser_error(parser, "Expected identifier after type specifier");
                 // Try to recover by skipping to next semicolon
                 while (!match_token(parser, TOKEN_EOF) && 
                        !match_token(parser, TOKEN_SEMICOLON)) {
                     advance_token(parser->lexer);
                     parser->current_token = peek_token(parser->lexer);
                 }
                 if (match_token(parser, TOKEN_SEMICOLON)) {
                     expect_token(parser, TOKEN_SEMICOLON);
                 }
             }
         } else {
             parser_error(parser, "Expected type specifier");
             // Try to recover by skipping to next semicolon
             while (!match_token(parser, TOKEN_EOF) && 
                   !match_token(parser, TOKEN_SEMICOLON)) {
                 advance_token(parser->lexer);
                 parser->current_token = peek_token(parser->lexer);
             }
             if (match_token(parser, TOKEN_SEMICOLON)) {
                 expect_token(parser, TOKEN_SEMICOLON);
             }
         }
     }
     
     return program;
 }
 
 // Parse function definition
 ASTNode* parse_function(Parser *parser) {
     ASTNode *function = create_ast_node(AST_FUNCTION);
     if (!function) return NULL;
     
     // Parameter list
     expect_token(parser, TOKEN_LPAREN);
     
     if (!match_token(parser, TOKEN_RPAREN)) {
         ASTNode *params = parse_parameter_list(parser);
         if (params) {
             function->data.function.parameters = params;
         }
     }
     
     expect_token(parser, TOKEN_RPAREN);
     
     // Function body
     if (match_token(parser, TOKEN_LBRACE)) {
         ASTNode *body = parse_compound_statement(parser);
         if (body) {
             function->data.function.body = body;
         }
     } else {
         // Function declaration without body, just skip semicolon
         expect_token(parser, TOKEN_SEMICOLON);
     }
     
     return function;
 }
 
 // Parse function parameter list
 ASTNode* parse_parameter_list(Parser *parser) {
     ASTNode *param_list = create_ast_node(AST_PARAM_LIST);
     if (!param_list) return NULL;
     
     // Parse first parameter
     if (match_token(parser, TOKEN_INT) || 
         match_token(parser, TOKEN_CHAR) || 
         match_token(parser, TOKEN_VOID)) {
         
         TokenType type_token = parser->current_token.type;
         expect_token(parser, type_token);
         
         // Special case for void parameter (no other parameters)
         if (type_token == TOKEN_VOID && !match_token(parser, TOKEN_IDENTIFIER)) {
             return param_list;  // Empty parameter list
         }
         
         if (match_token(parser, TOKEN_IDENTIFIER)) {
             ASTNode *param = create_ast_node(AST_PARAMETER);
             if (!param) {
                 free_ast(param_list);
                 return NULL;
             }
             
             param->data.parameter.type = type_token == TOKEN_INT ? TYPE_INT : 
                                          type_token == TOKEN_CHAR ? TYPE_CHAR : 
                                          TYPE_VOID;
             param->data.parameter.name = strdup(parser->current_token.value);
             expect_token(parser, TOKEN_IDENTIFIER);
             
             // Check for array parameter
             if (match_token(parser, TOKEN_LBRACKET)) {
                 expect_token(parser, TOKEN_LBRACKET);
                 expect_token(parser, TOKEN_RBRACKET);
                 param->data.parameter.is_array = 1;
             }
             
             add_child(param_list, param);
         }
     }
     
     // Parse additional parameters
     while (match_token(parser, TOKEN_COMMA)) {
         expect_token(parser, TOKEN_COMMA);
         
         if (match_token(parser, TOKEN_INT) || 
             match_token(parser, TOKEN_CHAR) || 
             match_token(parser, TOKEN_VOID)) {
             
             TokenType type_token = parser->current_token.type;
             expect_token(parser, type_token);
             
             if (match_token(parser, TOKEN_IDENTIFIER)) {
                 ASTNode *param = create_ast_node(AST_PARAMETER);
                 if (!param) {
                     free_ast(param_list);
                     return NULL;
                 }
                 
                 param->data.parameter.type = type_token == TOKEN_INT ? TYPE_INT : 
                                              type_token == TOKEN_CHAR ? TYPE_CHAR : 
                                              TYPE_VOID;
                 param->data.parameter.name = strdup(parser->current_token.value);
                 expect_token(parser, TOKEN_IDENTIFIER);
                 
                 // Check for array parameter
                 if (match_token(parser, TOKEN_LBRACKET)) {
                     expect_token(parser, TOKEN_LBRACKET);
                     expect_token(parser, TOKEN_RBRACKET);
                     param->data.parameter.is_array = 1;
                 }
                 
                 add_child(param_list, param);
             }
         }
     }
     
     return param_list;
 }
 
 // Parse compound statement (block)
 ASTNode* parse_compound_statement(Parser *parser) {
     ASTNode *block = create_ast_node(AST_COMPOUND_STMT);
     if (!block) return NULL;
     
     expect_token(parser, TOKEN_LBRACE);
     
     // Parse statements until closing brace
     while (!match_token(parser, TOKEN_RBRACE) && !match_token(parser, TOKEN_EOF)) {
         ASTNode *statement = parse_statement(parser);
         if (statement) {
             add_child(block, statement);
         } else {
             // Error recovery - skip to semicolon or next statement
             while (!match_token(parser, TOKEN_SEMICOLON) && 
                   !match_token(parser, TOKEN_RBRACE) && 
                   !match_token(parser, TOKEN_EOF)) {
                 advance_token(parser->lexer);
                 parser->current_token = peek_token(parser->lexer);
             }
             if (match_token(parser, TOKEN_SEMICOLON)) {
                 expect_token(parser, TOKEN_SEMICOLON);
             }
         }
     }
     
     expect_token(parser, TOKEN_RBRACE);
     
     return block;
 }
 
 // Parse a statement
 ASTNode* parse_statement(Parser *parser) {
     // Variable declaration
     if (match_token(parser, TOKEN_INT) || 
         match_token(parser, TOKEN_CHAR) || 
         match_token(parser, TOKEN_VOID)) {
         return parse_declaration(parser);
     }
     
     // Control statements
     if (match_token(parser, TOKEN_IF)) {
         return parse_if_statement(parser);
     }
     
     if (match_token(parser, TOKEN_WHILE)) {
         return parse_while_statement(parser);
     }
     
     if (match_token(parser, TOKEN_RETURN)) {
         return parse_return_statement(parser);
     }
     
     // Compound statement
     if (match_token(parser, TOKEN_LBRACE)) {
         return parse_compound_statement(parser);
     }
     
     // Expression statement (possibly empty)
     return parse_expression_statement(parser);
 }
 
 // Parse variable declaration
 ASTNode* parse_declaration(Parser *parser) {
     TokenType type_token = parser->current_token.type;
     expect_token(parser, type_token);
     
     return parse_variable_declaration(parser, type_token);
 }
 
 // Parse variable declaration with known type
 ASTNode* parse_variable_declaration(Parser *parser, TokenType type_token) {
     if (!match_token(parser, TOKEN_IDENTIFIER)) {
         parser_error(parser, "Expected identifier in variable declaration");
         return NULL;
     }
     
     ASTNode *var_decl = create_ast_node(AST_VARIABLE_DECL);
     if (!var_decl) return NULL;
     
     var_decl->data.variable_decl.type = type_token == TOKEN_INT ? TYPE_INT : 
                                         type_token == TOKEN_CHAR ? TYPE_CHAR : 
                                         TYPE_VOID;
     var_decl->data.variable_decl.name = strdup(parser->current_token.value);
     expect_token(parser, TOKEN_IDENTIFIER);
     
     // Check for array declaration
     if (match_token(parser, TOKEN_LBRACKET)) {
         expect_token(parser, TOKEN_LBRACKET);
         
         if (match_token(parser, TOKEN_INTEGER)) {
             var_decl->data.variable_decl.is_array = 1;
             var_decl->data.variable_decl.array_size = atoi(parser->current_token.value);
             expect_token(parser, TOKEN_INTEGER);
         }
         
         expect_token(parser, TOKEN_RBRACKET);
     }
     
     // Check for initialization
     if (match_token(parser, TOKEN_ASSIGN)) {
         expect_token(parser, TOKEN_ASSIGN);
         
         ASTNode *initializer = parse_expression(parser);
         if (initializer) {
             var_decl->data.variable_decl.initializer = initializer;
         }
     }
     
     expect_token(parser, TOKEN_SEMICOLON);
     
     return var_decl;
 }
 
 // Parse expression statement (expression followed by semicolon)
 ASTNode* parse_expression_statement(Parser *parser) {
     ASTNode *expr_stmt = create_ast_node(AST_EXPR_STMT);
     if (!expr_stmt) return NULL;
     
     // Empty statement
     if (match_token(parser, TOKEN_SEMICOLON)) {
         expect_token(parser, TOKEN_SEMICOLON);
         return expr_stmt;
     }
     
     // Expression
     ASTNode *expr = parse_expression(parser);
     if (expr) {
         add_child(expr_stmt, expr);
     }
     
     expect_token(parser, TOKEN_SEMICOLON);
     
     return expr_stmt;
 }
 
 // Parse if statement
 ASTNode* parse_if_statement(Parser *parser) {
     ASTNode *if_stmt = create_ast_node(AST_IF_STMT);
     if (!if_stmt) return NULL;
     
     expect_token(parser, TOKEN_IF);
     expect_token(parser, TOKEN_LPAREN);
     
     // Condition
     ASTNode *condition = parse_expression(parser);
     if (condition) {
         if_stmt->data.if_stmt.condition = condition;
     }
     
     expect_token(parser, TOKEN_RPAREN);
     
     // If branch
     ASTNode *if_branch = parse_statement(parser);
     if (if_branch) {
         if_stmt->data.if_stmt.if_branch = if_branch;
     }
     
     // Optional else branch
     if (match_token(parser, TOKEN_ELSE)) {
         expect_token(parser, TOKEN_ELSE);
         
         ASTNode *else_branch = parse_statement(parser);
         if (else_branch) {
             if_stmt->data.if_stmt.else_branch = else_branch;
         }
     }
     
     return if_stmt;
 }
 
 // Parse while statement
 ASTNode* parse_while_statement(Parser *parser) {
     ASTNode *while_stmt = create_ast_node(AST_WHILE_STMT);
     if (!while_stmt) return NULL;
     
     expect_token(parser, TOKEN_WHILE);
     expect_token(parser, TOKEN_LPAREN);
     
     // Condition
     ASTNode *condition = parse_expression(parser);
     if (condition) {
         while_stmt->data.while_stmt.condition = condition;
     }
     
     expect_token(parser, TOKEN_RPAREN);
     
     // Body
     ASTNode *body = parse_statement(parser);
     if (body) {
         while_stmt->data.while_stmt.body = body;
     }
     
     return while_stmt;
 }
 
 // Parse return statement
 ASTNode* parse_return_statement(Parser *parser) {
     ASTNode *return_stmt = create_ast_node(AST_RETURN_STMT);
     if (!return_stmt) return NULL;
     
     expect_token(parser, TOKEN_RETURN);
     
     // Optional return value
     if (!match_token(parser, TOKEN_SEMICOLON)) {
         ASTNode *value = parse_expression(parser);
         if (value) {
             return_stmt->data.return_stmt.value = value;
         }
     }
     
     expect_token(parser, TOKEN_SEMICOLON);
     
     return return_stmt;
 }
 
 // Parse expression
 ASTNode* parse_expression(Parser *parser) {
     return parse_assignment_expression(parser);
 }
 
 // Parse assignment expression
 ASTNode* parse_assignment_expression(Parser *parser) {
     ASTNode *expr = parse_logical_or_expression(parser);
     
     if (match_token(parser, TOKEN_ASSIGN)) {
         ASTNode *assign = create_ast_node(AST_ASSIGN_EXPR);
         if (!assign) {
             free_ast(expr);
             return NULL;
         }
         
         assign->data.binary_expr.left = expr;
         
         expect_token(parser, TOKEN_ASSIGN);
         
         ASTNode *right = parse_assignment_expression(parser);
         if (right) {
             assign->data.binary_expr.right = right;
         }
         
         return assign;
     }
     
     return expr;
 }
 
 // Parse logical OR expression
 ASTNode* parse_logical_or_expression(Parser *parser) {
     ASTNode *left = parse_logical_and_expression(parser);
     
     while (match_token(parser, TOKEN_OR)) {
         ASTNode *or_expr = create_ast_node(AST_BINARY_EXPR);
         if (!or_expr) {
             free_ast(left);
             return NULL;
         }
         
         or_expr->data.binary_expr.op = OP_LOGICAL_OR;
         or_expr->data.binary_expr.left = left;
         
         expect_token(parser, TOKEN_OR);
         
         ASTNode *right = parse_logical_and_expression(parser);
         if (right) {
             or_expr->data.binary_expr.right = right;
         }
         
         left = or_expr;
     }
     
     return left;
 }
 
 // Parse logical AND expression
 ASTNode* parse_logical_and_expression(Parser *parser) {
     ASTNode *left = parse_equality_expression(parser);
     
     while (match_token(parser, TOKEN_AND)) {
         ASTNode *and_expr = create_ast_node(AST_BINARY_EXPR);
         if (!and_expr) {
             free_ast(left);
             return NULL;
         }
         
         and_expr->data.binary_expr.op = OP_LOGICAL_AND;
         and_expr->data.binary_expr.left = left;
         
         expect_token(parser, TOKEN_AND);
         
         ASTNode *right = parse_equality_expression(parser);
         if (right) {
             and_expr->data.binary_expr.right = right;
         }
         
         left = and_expr;
     }
     
     return left;
 }
 
 // Parse equality expression
 ASTNode* parse_equality_expression(Parser *parser) {
     ASTNode *left = parse_relational_expression(parser);
     
     while (match_token(parser, TOKEN_EQ) || match_token(parser, TOKEN_NEQ)) {
         ASTNode *eq_expr = create_ast_node(AST_BINARY_EXPR);
         if (!eq_expr) {
             free_ast(left);
             return NULL;
         }
         
         if (match_token(parser, TOKEN_EQ)) {
             eq_expr->data.binary_expr.op = OP_EQ;
             expect_token(parser, TOKEN_EQ);
         } else {
             eq_expr->data.binary_expr.op = OP_NEQ;
             expect_token(parser, TOKEN_NEQ);
         }
         
         eq_expr->data.binary_expr.left = left;
         
         ASTNode *right = parse_relational_expression(parser);
         if (right) {
             eq_expr->data.binary_expr.right = right;
         }
         
         left = eq_expr;
     }
     
     return left;
 }
 
 // Parse relational expression
 ASTNode* parse_relational_expression(Parser *parser) {
     ASTNode *left = parse_additive_expression(parser);
     
     while (match_token(parser, TOKEN_LT) || match_token(parser, TOKEN_GT) ||
            match_token(parser, TOKEN_LTE) || match_token(parser, TOKEN_GTE)) {
         ASTNode *rel_expr = create_ast_node(AST_BINARY_EXPR);
         if (!rel_expr) {
             free_ast(left);
             return NULL;
         }
         
         if (match_token(parser, TOKEN_LT)) {
             rel_expr->data.binary_expr.op = OP_LT;
             expect_token(parser, TOKEN_LT);
         } else if (match_token(parser, TOKEN_GT)) {
             rel_expr->data.binary_expr.op = OP_GT;
             expect_token(parser, TOKEN_GT);
         } else if (match_token(parser, TOKEN_LTE)) {
             rel_expr->data.binary_expr.op = OP_LTE;
             expect_token(parser, TOKEN_LTE);
         } else {
             rel_expr->data.binary_expr.op = OP_GTE;
             expect_token(parser, TOKEN_GTE);
         }
         
         rel_expr->data.binary_expr.left = left;
         
         ASTNode *right = parse_additive_expression(parser);
         if (right) {
             rel_expr->data.binary_expr.right = right;
         }
         
         left = rel_expr;
     }
     
     return left;
 }
 
 // Parse additive expression
 ASTNode* parse_additive_expression(Parser *parser) {
     ASTNode *left = parse_multiplicative_expression(parser);
     
     while (match_token(parser, TOKEN_PLUS) || match_token(parser, TOKEN_MINUS)) {
         ASTNode *add_expr = create_ast_node(AST_BINARY_EXPR);
         if (!add_expr) {
             free_ast(left);
             return NULL;
         }
         
         if (match_token(parser, TOKEN_PLUS)) {
             add_expr->data.binary_expr.op = OP_ADD;
             expect_token(parser, TOKEN_PLUS);
         } else {
             add_expr->data.binary_expr.op = OP_SUBTRACT;
             expect_token(parser, TOKEN_MINUS);
         }
         
         add_expr->data.binary_expr.left = left;
         
         ASTNode *right = parse_multiplicative_expression(parser);
         if (right) {
             add_expr->data.binary_expr.right = right;
         }
         
         left = add_expr;
     }
     
     return left;
 }
 
 // Parse multiplicative expression
 ASTNode* parse_multiplicative_expression(Parser *parser) {
     ASTNode *left = parse_unary_expression(parser);
     
     while (match_token(parser, TOKEN_STAR) || 
            match_token(parser, TOKEN_SLASH) || 
            match_token(parser, TOKEN_PERCENT)) {
         ASTNode *mul_expr = create_ast_node(AST_BINARY_EXPR);
         if (!mul_expr) {
             free_ast(left);
             return NULL;
         }
         
         if (match_token(parser, TOKEN_STAR)) {
             mul_expr->data.binary_expr.op = OP_MULTIPLY;
             expect_token(parser, TOKEN_STAR);
         } else if (match_token(parser, TOKEN_SLASH)) {
             mul_expr->data.binary_expr.op = OP_DIVIDE;
             expect_token(parser, TOKEN_SLASH);
         } else {
             mul_expr->data.binary_expr.op = OP_MODULO;
             expect_token(parser, TOKEN_PERCENT);
         }
         
         mul_expr->data.binary_expr.left = left;
         
         ASTNode *right = parse_unary_expression(parser);
         if (right) {
             mul_expr->data.binary_expr.right = right;
         }
         
         left = mul_expr;
     }
     
     return left;
 }
 
 // Parse unary expression
 ASTNode* parse_unary_expression(Parser *parser) {
     if (match_token(parser, TOKEN_MINUS) || 
         match_token(parser, TOKEN_NOT) ||
         match_token(parser, TOKEN_BITNOT)) {
         ASTNode *unary = create_ast_node(AST_UNARY_EXPR);
         if (!unary) return NULL;
         
         if (match_token(parser, TOKEN_MINUS)) {
             unary->data.unary_expr.op = OP_NEGATE;
             expect_token(parser, TOKEN_MINUS);
         } else if (match_token(parser, TOKEN_NOT)) {
             unary->data.unary_expr.op = OP_NOT;
             expect_token(parser, TOKEN_NOT);
         } else {
             unary->data.unary_expr.op = OP_BITWISE_NOT;
             expect_token(parser, TOKEN_BITNOT);
         }
         
         ASTNode *operand = parse_unary_expression(parser);
         if (operand) {
             unary->data.unary_expr.operand = operand;
         }
         
         return unary;
     }
     
     return parse_postfix_expression(parser);
 }
 
 // Parse postfix expression
 ASTNode* parse_postfix_expression(Parser *parser) {
     ASTNode *expr = parse_primary_expression(parser);
     
     for (;;) {
         // Array subscript
         if (match_token(parser, TOKEN_LBRACKET)) {
             ASTNode *subscript = create_ast_node(AST_SUBSCRIPT_EXPR);
             if (!subscript) {
                 free_ast(expr);
                 return NULL;
             }
             
             subscript->data.subscript_expr.array = expr;
             
             expect_token(parser, TOKEN_LBRACKET);
             
             ASTNode *index = parse_expression(parser);
             if (index) {
                 subscript->data.subscript_expr.index = index;
             }
             
             expect_token(parser, TOKEN_RBRACKET);
             
             expr = subscript;
         }
         // Function call
         else if (match_token(parser, TOKEN_LPAREN)) {
             ASTNode *call = create_ast_node(AST_CALL_EXPR);
             if (!call) {
                 free_ast(expr);
                 return NULL;
             }
             
             call->data.call_expr.function = expr;
             
             expect_token(parser, TOKEN_LPAREN);
             
             // Parse arguments if present
             if (!match_token(parser, TOKEN_RPAREN)) {
                 ASTNode *args = create_ast_node(AST_ARG_LIST);
                 if (!args) {
                     free_ast(call);
                     return NULL;
                 }
                 
                 // First argument
                 ASTNode *arg = parse_expression(parser);
                 if (arg) {
                     add_child(args, arg);
                 }
                 
                 // Additional arguments
                 while (match_token(parser, TOKEN_COMMA)) {
                     expect_token(parser, TOKEN_COMMA);
                     
                     arg = parse_expression(parser);
                     if (arg) {
                         add_child(args, arg);
                     }
                 }
                 
                 call->data.call_expr.arguments = args;
             }
             
             expect_token(parser, TOKEN_RPAREN);
             
             expr = call;
         }
         // Postfix increment/decrement
         else if (match_token(parser, TOKEN_INC) || match_token(parser, TOKEN_DEC)) {
             ASTNode *postfix = create_ast_node(AST_UNARY_EXPR);
             if (!postfix) {
                 free_ast(expr);
                 return NULL;
             }
             
             if (match_token(parser, TOKEN_INC)) {
                 postfix->data.unary_expr.op = OP_POST_INC;
                 expect_token(parser, TOKEN_INC);
             } else {
                 postfix->data.unary_expr.op = OP_POST_DEC;
                 expect_token(parser, TOKEN_DEC);
             }
             
             postfix->data.unary_expr.operand = expr;
             
             expr = postfix;
         }
         else {
             break;
         }
     }
     
     return expr;
 }
 
 // Parse primary expression
 ASTNode* parse_primary_expression(Parser *parser) {
     // Identifier
     if (match_token(parser, TOKEN_IDENTIFIER)) {
         ASTNode *identifier = create_ast_node(AST_IDENTIFIER);
         if (!identifier) return NULL;
         
         identifier->data.identifier.name = strdup(parser->current_token.value);
         expect_token(parser, TOKEN_IDENTIFIER);
         
         return identifier;
     }
     
     // Integer literal
     if (match_token(parser, TOKEN_INTEGER)) {
         ASTNode *integer = create_ast_node(AST_INTEGER);
         if (!integer) return NULL;
         
         integer->data.integer.value = atoi(parser->current_token.value);
         expect_token(parser, TOKEN_INTEGER);
         
         return integer;
     }
     
     // Character literal
     if (match_token(parser, TOKEN_CHARACTER)) {
         ASTNode *character = create_ast_node(AST_CHARACTER);
         if (!character) return NULL;
         
         character->data.character.value = parser->current_token.value[0];
         expect_token(parser, TOKEN_CHARACTER);
         
         return character;
     }
     
     // String literal
     if (match_token(parser, TOKEN_STRING)) {
         ASTNode *string = create_ast_node(AST_STRING);
         if (!string) return NULL;
         
         string->data.string.value = strdup(parser->current_token.value);
         expect_token(parser, TOKEN_STRING);
         
         return string;
     }
     
     // Parenthesized expression
     if (match_token(parser, TOKEN_LPAREN)) {
         expect_token(parser, TOKEN_LPAREN);
         
         ASTNode *expr = parse_expression(parser);
         
         expect_token(parser, TOKEN_RPAREN);
         
         return expr;
     }
     
     parser_error(parser, "Expected expression");
     return NULL;
 }
 