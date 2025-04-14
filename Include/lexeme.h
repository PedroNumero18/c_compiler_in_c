 #ifndef LEXEME_H
 #define LEXEME_H
 
 #include <stdio.h>
 
 // Token types
 typedef enum {
     TOKEN_EOF,
     
     // Literals
     TOKEN_IDENTIFIER,
     TOKEN_INTEGER,
     TOKEN_CHARACTER,
     TOKEN_STRING,
     
     // Keywords
     TOKEN_INT,
     TOKEN_CHAR,
     TOKEN_VOID,
     TOKEN_IF,
     TOKEN_ELSE,
     TOKEN_WHILE,
     TOKEN_FOR,
     TOKEN_RETURN,
     
     // Operators and punctuation
     TOKEN_PLUS,       // +
     TOKEN_MINUS,      // -
     TOKEN_STAR,       // *
     TOKEN_SLASH,      // /
     TOKEN_PERCENT,    // %
     TOKEN_EQ,         // ==
     TOKEN_NEQ,        // !=
     TOKEN_LT,         // <
     TOKEN_GT,         // >
     TOKEN_LTE,        // <=
     TOKEN_GTE,        // >=
     TOKEN_ASSIGN,     // =
     TOKEN_INC,        // ++
     TOKEN_DEC,        // --
     TOKEN_AND,        // &&
     TOKEN_OR,         // ||
     TOKEN_NOT,        // !
     TOKEN_BITAND,     // &
     TOKEN_BITOR,      // |
     TOKEN_BITXOR,     // ^
     TOKEN_BITNOT,     // ~
     TOKEN_SHL,        // <<
     TOKEN_SHR,        // >>
     
     // Punctuation
     TOKEN_SEMICOLON,  // ;
     TOKEN_COLON,      // :
     TOKEN_COMMA,      // ,
     TOKEN_DOT,        // .
     TOKEN_LPAREN,     // (
     TOKEN_RPAREN,     // )
     TOKEN_LBRACE,     // {
     TOKEN_RBRACE,     // }
     TOKEN_LBRACKET,   // [
     TOKEN_RBRACKET,   // ]
     
     // Preprocessor
     TOKEN_POUND,      // #
     
     // Error
     TOKEN_ERROR
 } TokenType;
 
 // Token structure
 typedef struct {
     TokenType type;
     char *value;      // Lexeme value
     int line;         // Line number
     int column;       // Column number
     char *filename;   // Source filename
 } Token;
 
 // Lexer structure
 typedef struct {
     FILE *file;       // Source file
     char *filename;   // Source filename
     char *buffer;     // Buffer for reading file
     int buffer_size;  // Size of buffer
     int position;     // Current position in buffer
     int line;         // Current line number
     int column;       // Current column number
     Token current;    // Current token
 } Lexer;
 
 // Lexer functions
 Lexer* init_lexer(FILE *file, char *filename);
 void free_lexer(Lexer *lexer);
 Token get_token(Lexer *lexer);
 void advance_token(Lexer *lexer);
 Token peek_token(Lexer *lexer);
 void lexer_error(Lexer *lexer, const char *message);
 
 // Token utilities
 const char* token_type_str(TokenType type);
 void print_token(Token token);
 
 #endif // LEXER_H
 