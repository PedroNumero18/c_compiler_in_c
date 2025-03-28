#include <stdio.h>
#include <stdlib.h>

#include "lexeme.h"

int main(int argc, char **argv){
    if (argc <1){
        fprintf(stderr, "argument manquant");
        exit(1);
    }
    FILE *F = fopen(argv[1],"r"); 
    printf("le fichier :%s\n", argv[1]);
    
    Lexer *LC = init_lexer(F, argv[1]);
    Token tk = peek_token(LC);
    do
    {
        tk = peek_token(LC);
        print_token(tk);
        advance_token(LC);
    } while (tk.type!= TOKEN_EOF);    
    free_lexer(LC);
    return 0;
}

