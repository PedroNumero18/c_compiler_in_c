/**
 * Lexer Implementation
 * 
 * Handles the lexical analysis phase of compilation,
 * converting source text into a stream of tokens.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "common.h"
#include "lexeme.h"
#include "error.h"
 
#define BUFFER_SIZE 4096
 
// Initialize lexer with input file
Lexer* init_lexer(FILE *file, char *filename) {
    Lexer *lexer = (Lexer*)malloc(sizeof(Lexer));
    if (!lexer) return NULL;
     
    lexer->file = file;
    lexer->filename = strdup(filename);
    lexer->buffer_size = BUFFER_SIZE;
    lexer->buffer = (char*)malloc(lexer->buffer_size);
    if (!lexer->buffer) {
        free(lexer->filename);
        free(lexer);
        return NULL;
    }
    
    // Validate file and buffer
    if (!file || !lexer->buffer) {
        free(lexer->filename);
        free(lexer->buffer);
        free(lexer);
        return NULL;
    }

    // Read initial buffer
    size_t bytes_read = fread(lexer->buffer, 1, lexer->buffer_size, file);
    assert(lexer->buffer_size >= 0); // Ensure non-negative first
    if (bytes_read < (size_t)lexer->buffer_size){lexer->buffer[bytes_read] = '\0';} // Ensure null termination
     
    lexer->position = 0;
    lexer->line = 1;
    lexer->column = 1;
     
    // Initialize with first token
    lexer->current.type = TOKEN_ERROR;
    lexer->current.value = NULL;
    advance_token(lexer);
     
    return lexer;
}
 
// Free lexer resources
void free_lexer(Lexer *lexer) {
     if (lexer) {
         free(lexer->filename);
         free(lexer->buffer);
         if (lexer->current.value) {
             free(lexer->current.value);
         }
         free(lexer);
     }
 }
 
// Reload buffer with more file data
static int reload_buffer(Lexer *lexer) {
    // If we're at the end of the file, nothing to reload
    if (feof(lexer->file)) return 0;
     
    size_t bytes_read = fread(lexer->buffer, 1, lexer->buffer_size, lexer->file);
    if (bytes_read == 0) return 0;
     
    lexer->position = 0;
    assert(lexer->buffer_size >= 0);
    if (bytes_read < (size_t)lexer->buffer_size){lexer->buffer[bytes_read] = '\0';} 
    return 1;
}
 
// Get current character
static char current_char(Lexer *lexer) {
     if (lexer->position >= lexer->buffer_size) {
         if (!reload_buffer(lexer)) {
             return '\0';  // EOF
         }
     }
     
     return lexer->buffer[lexer->position];
 }
 
// Advance to next character
static void advance_char(Lexer *lexer) {
     if (current_char(lexer) == '\n') {
         lexer->line++;
         lexer->column = 1;
     } else {
         lexer->column++;
     }
     
     lexer->position++;
 }
 
// Peek at next character without advancing
static char peek_char(Lexer *lexer) {
     if (lexer->position + 1 >= lexer->buffer_size) {
         return '\0';  // Can't peek beyond buffer
     }
     
     return lexer->buffer[lexer->position + 1];
 }
 
// Skip whitespace
static void skip_whitespace(Lexer *lexer) {
     while (isspace(current_char(lexer))) {
         advance_char(lexer);
     }
 }
 
// Skip comments
static void skip_comments(Lexer *lexer) {
     // Single-line comment
     if (current_char(lexer) == '/' && peek_char(lexer) == '/') {
         // Skip until end of line
         advance_char(lexer);  // Skip first '/'
         advance_char(lexer);  // Skip second '/'
         
         while (current_char(lexer) != '\n' && current_char(lexer) != '\0') {
             advance_char(lexer);
         }
         if (current_char(lexer) == '\n') {
             advance_char(lexer);  // Skip newline
         }
     }
     // Multi-line comment
     else if (current_char(lexer) == '/' && peek_char(lexer) == '*') {
         advance_char(lexer);  // Skip '/'
         advance_char(lexer);  // Skip '*'
         
         while (!(current_char(lexer) == '*' && peek_char(lexer) == '/') && 
                current_char(lexer) != '\0') {
             advance_char(lexer);
         }
         
         if (current_char(lexer) != '\0') {
             advance_char(lexer);  // Skip '*'
             advance_char(lexer);  // Skip '/'
         } else {
             lexer_error(lexer, "Unterminated multi-line comment");
         }
     }
 }
 
// Scan identifier or keyword
static Token scan_identifier(Lexer *lexer) {
     Token token;
     int start_pos = lexer->position;
     int start_line = lexer->line;
     int start_col = lexer->column;
     
     while (isalnum(current_char(lexer)) || current_char(lexer) == '_') {
         advance_char(lexer);
     }
     
     // Extract identifier text
     int length = lexer->position - start_pos;
     char *value = (char*)malloc(length + 1);
     strncpy(value, lexer->buffer + start_pos, length);
     value[length] = '\0';
     
     // Check if this is a keyword
     token.type = TOKEN_IDENTIFIER;
     if (strcmp(value, "int") == 0) token.type = TOKEN_INT;
     else if (strcmp(value, "char") == 0) token.type = TOKEN_CHAR;
     else if (strcmp(value, "void") == 0) token.type = TOKEN_VOID;
     else if (strcmp(value, "if") == 0) token.type = TOKEN_IF;
     else if (strcmp(value, "else") == 0) token.type = TOKEN_ELSE;
     else if (strcmp(value, "while") == 0) token.type = TOKEN_WHILE;
     else if (strcmp(value, "for") == 0) token.type = TOKEN_FOR;
     else if (strcmp(value, "return") == 0) token.type = TOKEN_RETURN;
     
     token.value = value;
     token.line = start_line;
     token.column = start_col;
     token.filename = lexer->filename;
     
     return token;
 }
 
// Scan numeric literal
static Token scan_number(Lexer *lexer) {
     Token token;
     int start_pos = lexer->position;
     int start_line = lexer->line;
     int start_col = lexer->column;
     
     while (isdigit(current_char(lexer))) {
         advance_char(lexer);
     }
     
     // Extract number text
     int length = lexer->position - start_pos;
     char *value = (char*)malloc(length + 1);
     strncpy(value, lexer->buffer + start_pos, length);
     value[length] = '\0';
     
     token.type = TOKEN_INTEGER;
     token.value = value;
     token.line = start_line;
     token.column = start_col;
     token.filename = lexer->filename;
     
     return token;
 }
 
// Scan character literal
static Token scan_character(Lexer *lexer) {
     Token token;
     int start_line = lexer->line;
     int start_col = lexer->column;
     
     advance_char(lexer);  // Skip opening quote
     
     char c;
     if (current_char(lexer) == '\\') {
         advance_char(lexer);  // Skip backslash
         switch (current_char(lexer)) {
             case 'n': c = '\n'; break;
             case 't': c = '\t'; break;
             case 'r': c = '\r'; break;
             case '0': c = '\0'; break;
             case '\\': c = '\\'; break;
             case '\'': c = '\''; break;
             case '\"': c = '\"'; break;
             default:
                 lexer_error(lexer, "Invalid escape sequence");
                 token.type = TOKEN_ERROR;
                 token.value = strdup("Invalid escape sequence");
                 token.line = start_line;
                 token.column = start_col;
                 token.filename = lexer->filename;
                 return token;
         }
         advance_char(lexer);
     } else {
         c = current_char(lexer);
         advance_char(lexer);
     }
     
     if (current_char(lexer) != '\'') {
         lexer_error(lexer, "Unterminated character literal");
         token.type = TOKEN_ERROR;
         token.value = strdup("Unterminated character literal");
         token.line = start_line;
         token.column = start_col;
         token.filename = lexer->filename;
         return token;
     }
     
     advance_char(lexer);  // Skip closing quote
     
     // Create token
     char *value = (char*)malloc(2);
     value[0] = c;
     value[1] = '\0';
     
     token.type = TOKEN_CHARACTER;
     token.value = value;
     token.line = start_line;
     token.column = start_col;
     token.filename = lexer->filename;
     
     return token;
 }
 
// Scan string literal
static Token scan_string(Lexer *lexer) {
     Token token;
     int start_line = lexer->line;
     int start_col = lexer->column;
     
     advance_char(lexer);  // Skip opening quote
     
     int start_pos = lexer->position;
     int length = 0;
     
     while (current_char(lexer) != '\"' && current_char(lexer) != '\0') {
         if (current_char(lexer) == '\\') {
             advance_char(lexer);  // Skip backslash
             if (current_char(lexer) == '\0') {
                 break;
             }
         }
         advance_char(lexer);
         length++;
     }
     
     if (current_char(lexer) == '\0') {
         lexer_error(lexer, "Unterminated string literal");
         token.type = TOKEN_ERROR;
         token.value = strdup("Unterminated string literal");
         token.line = start_line;
         token.column = start_col;
         token.filename = lexer->filename;
         return token;
     }
     
     // Extract string value
     char *value = (char*)malloc(length + 1);
     strncpy(value, lexer->buffer + start_pos, length);
     value[length] = '\0';
     
     advance_char(lexer);  // Skip closing quote
     
     token.type = TOKEN_STRING;
     token.value = value;
     token.line = start_line;
     token.column = start_col;
     token.filename = lexer->filename;
     
     return token;
 }
 
// Get the next token from input
Token get_token(Lexer *lexer) {
     Token token;
     
     skip_whitespace(lexer);
     
     // Handle comments
     while (current_char(lexer) == '/' && 
            (peek_char(lexer) == '/' || peek_char(lexer) == '*')) {
         skip_comments(lexer);
         skip_whitespace(lexer);
     }
     
     int start_line = lexer->line;
     int start_col = lexer->column;
     
     // EOF
     if (current_char(lexer) == '\0') {
         token.type = TOKEN_EOF;
         token.value = strdup("EOF");
         token.line = start_line;
         token.column = start_col;
         token.filename = lexer->filename;
         return token;
     }
     
     // Identifiers and keywords
     if (isalpha(current_char(lexer)) || current_char(lexer) == '_') {
         return scan_identifier(lexer);
     }
     
     // Numbers
     if (isdigit(current_char(lexer))) {
         return scan_number(lexer);
     }
     
     // Character literals
     if (current_char(lexer) == '\'') {
         return scan_character(lexer);
     }
     
     // String literals
     if (current_char(lexer) == '\"') {
         return scan_string(lexer);
     }
     
     // Operators and punctuation
     token.type = TOKEN_ERROR;
     token.line = start_line;
     token.column = start_col;
     token.filename = lexer->filename;
     
     switch (current_char(lexer)) {
         case '+':
             advance_char(lexer);
             if (current_char(lexer) == '+') {
                 advance_char(lexer);
                 token.type = TOKEN_INC;
                 token.value = strdup("++");
             } else {
                 token.type = TOKEN_PLUS;
                 token.value = strdup("+");
             }
             break;
         case '-':
             advance_char(lexer);
             if (current_char(lexer) == '-') {
                 advance_char(lexer);
                 token.type = TOKEN_DEC;
                 token.value = strdup("--");
             } else {
                 token.type = TOKEN_MINUS;
                 token.value = strdup("-");
             }
             break;
         case '*':
             advance_char(lexer);
             token.type = TOKEN_STAR;
             token.value = strdup("*");
             break;
         case '/':
             advance_char(lexer);
             token.type = TOKEN_SLASH;
             token.value = strdup("/");
             break;
         case '%':
             advance_char(lexer);
             token.type = TOKEN_PERCENT;
             token.value = strdup("%");
             break;
         case '=':
             advance_char(lexer);
             if (current_char(lexer) == '=') {
                 advance_char(lexer);
                 token.type = TOKEN_EQ;
                 token.value = strdup("==");
             } else {
                 token.type = TOKEN_ASSIGN;
                 token.value = strdup("=");
             }
             break;
         case '!':
             advance_char(lexer);
             if (current_char(lexer) == '=') {
                 advance_char(lexer);
                 token.type = TOKEN_NEQ;
                 token.value = strdup("!=");
             } else {
                 token.type = TOKEN_NOT;
                 token.value = strdup("!");
             }
             break;
         case '<':
             advance_char(lexer);
             if (current_char(lexer) == '=') {
                 advance_char(lexer);
                 token.type = TOKEN_LTE;
                 token.value = strdup("<=");
             } else if (current_char(lexer) == '<') {
                 advance_char(lexer);
                 token.type = TOKEN_SHL;
                 token.value = strdup("<<");
             } else {
                 token.type = TOKEN_LT;
                 token.value = strdup("<");
             }
             break;
         case '>':
             advance_char(lexer);
             if (current_char(lexer) == '=') {
                 advance_char(lexer);
                 token.type = TOKEN_GTE;
                 token.value = strdup(">=");
             } else if (current_char(lexer) == '>') {
                 advance_char(lexer);
                 token.type = TOKEN_SHR;
                 token.value = strdup(">>");
             } else {
                 token.type = TOKEN_GT;
                 token.value = strdup(">");
             }
             break;
         case '&':
             advance_char(lexer);
             if (current_char(lexer) == '&') {
                 advance_char(lexer);
                 token.type = TOKEN_AND;
                 token.value = strdup("&&");
             } else {
                 token.type = TOKEN_BITAND;
                 token.value = strdup("&");
             }
             break;
         case '|':
             advance_char(lexer);
             if (current_char(lexer) == '|') {
                 advance_char(lexer);
                 token.type = TOKEN_OR;
                 token.value = strdup("||");
             } else {
                 token.type = TOKEN_BITOR;
                 token.value = strdup("|");
             }
             break;
         case '^':
             advance_char(lexer);
             token.type = TOKEN_BITXOR;
             token.value = strdup("^");
             break;
         case '~':
             advance_char(lexer);
             token.type = TOKEN_BITNOT;
             token.value = strdup("~");
             break;
         case ';':
             advance_char(lexer);
             token.type = TOKEN_SEMICOLON;
             token.value = strdup(";");
             break;
         case ':':
             advance_char(lexer);
             token.type = TOKEN_COLON;
             token.value = strdup(":");
             break;
         case ',':
             advance_char(lexer);
             token.type = TOKEN_COMMA;
             token.value = strdup(",");
             break;
         case '.':
             advance_char(lexer);
             token.type = TOKEN_DOT;
             token.value = strdup(".");
             break;
         case '(':
             advance_char(lexer);
             token.type = TOKEN_LPAREN;
             token.value = strdup("(");
             break;
         case ')':
             advance_char(lexer);
             token.type = TOKEN_RPAREN;
             token.value = strdup(")");
             break;
         case '{':
             advance_char(lexer);
             token.type = TOKEN_LBRACE;
             token.value = strdup("{");
             break;
         case '}':
             advance_char(lexer);
             token.type = TOKEN_RBRACE;
             token.value = strdup("}");
             break;
         case '[':
             advance_char(lexer);
             token.type = TOKEN_LBRACKET;
             token.value = strdup("[");
             break;
         case ']':
             advance_char(lexer);
             token.type = TOKEN_RBRACKET;
             token.value = strdup("]");
             break;
         case '#':
             advance_char(lexer);
             token.type = TOKEN_POUND;
             token.value = strdup("#");
             break;
         default:
             char error_msg[128];
             snprintf(error_msg, sizeof(error_msg), "Unexpected character: '%c'", current_char(lexer));
             lexer_error(lexer, error_msg);
             token.type = TOKEN_ERROR;
             token.value = strdup(error_msg);
             advance_char(lexer);
             break;
     }
     
     return token;
 }
 
// Advance to the next token
void advance_token(Lexer *lexer) {
     if (lexer->current.value) {
         free(lexer->current.value);
     }
     lexer->current = get_token(lexer);
 }
 
// Peek at current token without consuming it
Token peek_token(Lexer *lexer) {
    if (lexer == NULL){
        
    }
    return lexer->current;
 }
 
// Report lexer error
void lexer_error(Lexer *lexer, const char *message) {
     error_report_location(lexer->filename, lexer->line, lexer->column, message);
 }
 
// Get string representation of token type
const char* token_type_str(TokenType type) {
     switch(type) {
         case TOKEN_EOF: return "EOF";
         case TOKEN_IDENTIFIER: return "IDENTIFIER";
         case TOKEN_INTEGER: return "INTEGER";
         case TOKEN_CHARACTER: return "CHARACTER";
         case TOKEN_STRING: return "STRING";
         case TOKEN_INT: return "INT";
         case TOKEN_CHAR: return "CHAR";
         case TOKEN_VOID: return "VOID";
         case TOKEN_IF: return "IF";
         case TOKEN_ELSE: return "ELSE";
         case TOKEN_WHILE: return "WHILE";
         case TOKEN_FOR: return "FOR";
         case TOKEN_RETURN: return "RETURN";
         case TOKEN_PLUS: return "PLUS";
         case TOKEN_MINUS: return "MINUS";
         case TOKEN_STAR: return "STAR";
         case TOKEN_SLASH: return "SLASH";
         case TOKEN_PERCENT: return "PERCENT";
         case TOKEN_EQ: return "EQ";
         case TOKEN_NEQ: return "NEQ";
         case TOKEN_LT: return "LT";
         case TOKEN_GT: return "GT";
         case TOKEN_LTE: return "LTE";
         case TOKEN_GTE: return "GTE";
         case TOKEN_ASSIGN: return "ASSIGN";
         case TOKEN_INC: return "INC";
         case TOKEN_DEC: return "DEC";
         case TOKEN_AND: return "AND";
         case TOKEN_OR: return "OR";
         case TOKEN_NOT: return "NOT";
         case TOKEN_BITAND: return "BITAND";
         case TOKEN_BITOR: return "BITOR";
         case TOKEN_BITXOR: return "BITXOR";
         case TOKEN_BITNOT: return "BITNOT";
         case TOKEN_SHL: return "SHL";
         case TOKEN_SHR: return "SHR";
         case TOKEN_SEMICOLON: return "SEMICOLON";
         case TOKEN_COLON: return "COLON";
         case TOKEN_COMMA: return "COMMA";
         case TOKEN_DOT: return "DOT";
         case TOKEN_LPAREN: return "LPAREN";
         case TOKEN_RPAREN: return "RPAREN";
         case TOKEN_LBRACE: return "LBRACE";
         case TOKEN_RBRACE: return "RBRACE";
         case TOKEN_LBRACKET: return "LBRACKET";
         case TOKEN_RBRACKET: return "RBRACKET";
         case TOKEN_POUND: return "POUND";
         case TOKEN_ERROR: return "ERROR";
         default: return "UNKNOWN";
     }
 }
 
// Print token for debugging
void print_token(Token token) {
     printf("Token{ type=%s, value=\"%s\", line=%d, column=%d }\n",
            token_type_str(token.type),
            token.value,
            token.line,
            token.column);
 }
 