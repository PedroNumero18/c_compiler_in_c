#ifndef _LEXEME_H_
#define _LEXEME_H_

typedef enum{
    PARO,
    PARC,
    CROCHO,
    CROCHF,
    SEMCOL,
    INT
}lex_Token;

typedef enum{
    error_Char,
    INTERGER,
    SYMBOL,
    END_SEQ
}Char_Nature;

typedef struct {
    lex_Token nature;
    unsigned int line;
    unsigned int column;
    char * chain;
    int value;
}lexem;

void printlex(lexem l);



#endif