/*Parser
 * The parser translates token stream into an abstract syntax tree.
 */

 #ifndef PARSER_H
 #define PARSER_H
 
 #include "lexeme.h"
 #include "ast.h"
 
 // Parser structure
 typedef struct {
     Lexer *lexer;             // Lexer providing tokens
     Token current_token;      // Current token being processed
 } Parser;
 
 // Parser functions
 Parser* init_parser(Lexer *lexer);
 void free_parser(Parser *parser);
 
 // Parsing functions for different grammar constructs
 ASTNode* parse_program(Parser *parser);
 ASTNode* parse_function(Parser *parser);
 ASTNode* parse_parameter_list(Parser *parser);
 ASTNode* parse_compound_statement(Parser *parser);
 ASTNode* parse_statement(Parser *parser);
 ASTNode* parse_declaration(Parser *parser);
 ASTNode* parse_variable_declaration(Parser *parser, TokenType type_token);
 ASTNode* parse_expression_statement(Parser *parser);
 ASTNode* parse_if_statement(Parser *parser);
 ASTNode* parse_while_statement(Parser *parser);
 ASTNode* parse_return_statement(Parser *parser);
 ASTNode* parse_expression(Parser *parser);
 ASTNode* parse_assignment_expression(Parser *parser);
 ASTNode* parse_logical_or_expression(Parser *parser);
 ASTNode* parse_logical_and_expression(Parser *parser);
 ASTNode* parse_equality_expression(Parser *parser);
 ASTNode* parse_relational_expression(Parser *parser);
 ASTNode* parse_additive_expression(Parser *parser);
 ASTNode* parse_multiplicative_expression(Parser *parser);
 ASTNode* parse_unary_expression(Parser *parser);
 ASTNode* parse_postfix_expression(Parser *parser);
 ASTNode* parse_primary_expression(Parser *parser);
 
 // Utility functions
 int expect_token(Parser *parser, TokenType type);
 int match_token(Parser *parser, TokenType type);
 void parser_error(Parser *parser, const char *message);
 
 #endif // PARSER_H
 