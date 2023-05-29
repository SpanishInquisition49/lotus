#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../lib/scanner.h"
#include "../lib/list.h"

List scanner(const char*);

static int scanner_error = 0; 

int main(int argc, char *argv[]) {
    // Input validation
    Log_set_level(INFO);
    if(argc != 2) {
        printf("Usage: main [filename]\n");
        return EXIT_FAILURE;
    }
    // Scanner Phase
    List tokens = scanner(argv[1]);
    list_free(tokens, NULL);
    // Parser Phase
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

