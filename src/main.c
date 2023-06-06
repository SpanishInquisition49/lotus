#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../lib/scanner.h"
#include "../lib/parser.h"
#include "../lib/interpreter.h"
#include "../lib/list.h"

List scanner(const char*);
Exp_t *parser(List);
void interpreter(Exp_t*);

static int scanner_error = 0; 
static int parser_error = 0;

int main(int argc, char *argv[]) {
    // Input validation
    Log_set_level(WARNING);
    if(argc != 2) {
        printf("Usage: main [filename]\n");
        return EXIT_FAILURE;
    }
    List tokens = scanner(argv[1]);
    if(scanner_error) {
        list_free(tokens, token_free);
        exit(EXIT_FAILURE);
    }
    Exp_t *ast = parser(tokens);
    if(parser_error) {
        exp_destroy(ast);
        exit(EXIT_FAILURE);
    }
    interpreter(ast);
    return EXIT_SUCCESS;
}

List scanner(const char* filename) {
    Scanner scanner;
    scanner_init(&scanner, filename);
    scanner_scan_tokens(&scanner);
    List tokens = NULL;
    tokens_dup(scanner.tokens, &tokens);
    scanner_errors_report(scanner);
    scanner_error = scanner_had_error(scanner);
    scanner_destroy(scanner);
    return tokens;
}

Exp_t *parser(List tokens) {
    Parser parser;
    parser_init(&parser, tokens);
    Exp_t *ast = parser_generate_ast(&parser);
    parser_errors_report(parser);
    parser_error = parser_had_errors(parser);
    parser_destroy(parser);
    return ast;
}

void interpreter(Exp_t* ast) {
    Interpreter i;
    interpreter_init(&i, ast);
    interpreter_eval(&i);
    switch(i.result->type) {
        case T_STRING:
            printf("%s\n", (char*)i.result->value);
            break;
        case T_NIL:
            printf("nil\n");
            break;
        case T_BOOLEAN:
            printf("%s\n", *((int*)i.result->value) ? "true" : "false");
            break;
        case T_NUMBER:
            printf("%f\n", *((double*)i.result->value));
            break;
    }
    interpreter_destroy(i);
    return;
}
