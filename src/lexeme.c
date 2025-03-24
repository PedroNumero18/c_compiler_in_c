/* ---------------
-- module Lexeme reader 
-------------- */ 

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "lexeme.h"

/* locals variables */
char CC[2048];
lexem LC;
FILE *f = NULL; 

/*---------------Start the lexical analysis----------------*/

void start(char* fileN){
    if (strlen(fileN) == 0){
        fprintf(stderr, "Error file name incorrect");
        exit(1);
    }
    f = fopen(fileN,"r");
    LC.line = 1;
    LC.column = 1;
}

void next(){

}

/*-----------------print de current lexem------------------*/
char *Token_to_Char (lex_Token Token) {
	switch (Token) {
		case INT: return "INT" ;
		case SEMCOL: return "SEMCOL" ;
		case PARO : return "OPEN PARENTHESIS";
		case PARC : return " CLOSED PARENTHESIS";              
      	case END_SEQ: return "END_SEQ" ;     
      	default: return "ERREUR" ;            
	} ;
   } 

void printlex(lexem l){
    switch (l.nature) {
         case END_SEQ: 
            break;
         default: 
            printf("(ligne %d, ", l.line);
            printf("colonne %d) : ",l.column);
	    	printf("[") ;
            printf("nature = %s", Token_to_Char(l.nature)) ;
            printf(", chain = %s, ", l.chain) ;
            switch(l.nature) {
                 case INT:
                      printf("valeur = %d", l.value);
                 default:
                      break;
            } ;
	    printf("]") ;
	} ;
}
/*---------------------------------------------------------*/
/*--------Detect the Nature of the current Character-------*/
int isNumber(char c){
    if(c >= '0' && c<'9')
    {return 1;}
    else{return 0;}
}
int isSymbol(char c){
    switch (c)
    {
    case ';':
    case '(':
    case ')':
    case '{':
    case '}':
        return 1;
    default:
        return 0;
    }
}
int isSeparator(char c) { 
      return c == ' ' || c == '\t' || c == '\n' ;
   }
Char_Nature nature_char (char c) {
	   if (c == EOF) return END_SEQ;
	   if (isNumber(c)) return INTERGER;
	   if (isSeparator(c)) return SYMBOL;
	   return error_Char ;
   }
/*---------------------------------------------------------*/