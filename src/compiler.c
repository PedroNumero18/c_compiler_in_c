#include <stdio.h>
#include <stdlib.h>

#include "lexeme.h"
#include "parser.h"
#include "ast.h"

int main(int argc, char **argv){
    if (argc <1){
        fprintf(stderr, "argument manquant");
        exit(1);
    }
    FILE *F = fopen(argv[1],"r"); 
    printf("le fichier :%s\n", argv[1]);
    
    Lexer *LC = init_lexer(F, argv[1]);
    Parser *parse = init_parser(LC); 
    ASTNode *as = parse_program(parse);
    print_ast(as, 10);
    /*do
    {
        parse->current_token = peek_token(LC);
        print_token(parse->current_token);
        advance_token(LC);
    } while (parse->current_token.type!= TOKEN_EOF);    
    */
    free_lexer(LC);
    return 0;
}

