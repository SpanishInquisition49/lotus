#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../lib/scanner.h"
#include "../lib/parser.h"
#include "../lib/list.h"

List scanner(const char*);
void parser(List);

static int scanner_error = 0; 

int main(int argc, char *argv[]) {
    // Input validation
    Log_set_level(INFO);
    if(argc != 2) {
        printf("Usage: main [filename]\n");
        return EXIT_FAILURE;
    }
    parser(scanner(argv[1]));
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
    list_reverse_in_place(&tokens);
    return tokens;
}

void parser(List tokens) {
    Parser parser;
    parser_init(&parser, tokens);
    Exp_t *ast = parser_generate_ast(&parser);
    exp_destroy(ast);
    parser_errors_report(parser);
    parser_destroy(parser);
    return;
}
