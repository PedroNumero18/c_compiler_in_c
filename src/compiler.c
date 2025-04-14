#include <stdio.h>
#include <stdlib.h>

#include "../Include/lexeme.h"
#include "../Include/parser.h"
#include "../Include/codegen.h"
#include "../Include/error.h"

void print_usage(char *program_name) {
    fprintf(stderr, "Usage: %s input.c [-o output] [-S] [-t=target]\n", program_name);
    fprintf(stderr, "  -o output   Specify output filename (default: a.out)\n");
    fprintf(stderr, "  -S          Generate assembly output instead of executable\n");
    fprintf(stderr, "  -t=target   Specify target architecture: arm (default: arm)\n");
}

int main(int argc, char **argv){
    char *input_filename = NULL;
    char *output_filename = "a.out";
    int generate_assembly = 0; 
    
    // Parse command-line arguments
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            switch (argv[i][1]) {
                case 'o':
                    if (i + 1 < argc) {
                        output_filename = argv[++i];
                    } else {
                        print_usage(argv[0]);
                        return 1;
                    }
                    break;
                case 'S':
                    generate_assembly = 1;
                    break;
                default:
                    fprintf(stderr, "Unknown option: %s\n", argv[i]);
                    print_usage(argv[0]);
                    return 1;
            }
        } else {
            if (input_filename == NULL) {
                input_filename = argv[i];
            } else {
                fprintf(stderr, "Multiple input files not supported.\n");
                print_usage(argv[0]);
                return 1;
            }
        }
    }
    if (input_filename == NULL) {
        fprintf(stderr, "No input file specified.\n");
        print_usage(argv[0]);
        return 1;
    }

    // Set ARM as the target architecture name (because good looking)
    const char* target_name = "ARM";
    
    printf("Compiling %s to %s%s (target: %s)...\n", 
           input_filename, output_filename, 
           generate_assembly ? " (assembly)" : "",
           target_name);
    

    // Open input file
    FILE *input_file = fopen(input_filename, "r");
    if (input_file == NULL) {
        perror("Error opening input file");
        return 1;
    }
    
    // Open output file
    FILE *output_file = fopen(output_filename, generate_assembly ? "w" : "wb");
    if (output_file == NULL) {
        perror("Error opening output file");
        fclose(input_file);
        return 1;
    }

    // Initialize lexer with the input file
    Lexer *lexer = init_lexer(input_file, input_filename);
    if (!lexer) {
        error_report(NULL, "Failed to initialize lexer");
        fclose(input_file);
        fclose(output_file);
        return 1;
       }
       
    // Initialize parser with the lexer
    Parser *parser = init_parser(lexer);
    if (!parser) {
        error_report(NULL, "Failed to initialize parser");
        free_lexer(lexer);
        fclose(input_file);
        fclose(output_file);
        return 1;
    }
    
    // Parse the input and generate AST
    ASTNode *ast = parse_program(parser);
    if (!ast) {
        error_report(NULL, "Failed to parse program");
        free_parser(parser);
        free_lexer(lexer);
        fclose(input_file);
        fclose(output_file);
        return 1;
    }
    

    // Generate code from the AST
    int success = generate_code(ast, output_file, generate_assembly);
    if (!success) {
        error_report(NULL, "Code generation failed");
        free_ast(ast);
        free_parser(parser);
        free_lexer(lexer);
        fclose(input_file);
        fclose(output_file);
        return 1;
    }

    // Clean up resources
    free_ast(ast);
    free_parser(parser);
    free_lexer(lexer);
    fclose(input_file);
    fclose(output_file);
    
    printf("Compilation successful.\n");
    return 0;
}

