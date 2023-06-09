#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../lib/scanner.h"
#include "../lib/parser.h"
#include "../lib/interpreter.h"
#include "../lib/list.h"
#include "../lib/config.h"

static List scanner(const char*);
static List parser(List);
static void interpreter(List);
static void set_config(void);

static int scanner_error = 0; 
static int parser_error = 0;
static int show_reports = 0;

int main(int argc, char *argv[]) {
    // Input validation
    set_config();
    if(argc != 2) {
        printf("Usage: main [filename]\n");
        return EXIT_FAILURE;
    }
    List tokens = scanner(argv[1]);
    if(scanner_error) {
        list_free(tokens, token_free);
        exit(EXIT_FAILURE);
    }
    List statements = parser(tokens);
    if(parser_error) {
        list_free(statements, stmt_free);
        exit(EXIT_FAILURE);
    }
    interpreter(statements);
    return EXIT_SUCCESS;
}

List scanner(const char* filename) {
    Scanner scanner;
    scanner_init(&scanner, filename);
    scanner_scan_tokens(&scanner);
    List tokens = NULL;
    tokens_dup(scanner.tokens, &tokens);
    scanner_error = scanner_had_error(scanner);
    if(show_reports || scanner_error)
        scanner_errors_report(scanner);
    scanner_destroy(scanner);
    return tokens;
}

List parser(List tokens) {
    Parser parser;
    parser_init(&parser, tokens);
    List statements = parser_parse(&parser);
    parser_error = parser_had_errors(parser);
    if(show_reports || parser_error)
        parser_errors_report(parser);
    parser_destroy(parser);
    return statements;
}

void interpreter(List statements) {
    Interpreter i;
    interpreter_init(&i, statements);
    interpreter_eval(&i);
    interpreter_destroy(i);
    return;
}

void set_config(void) {
   char *v = config_read("LOG_LEVEL");
   int log_level;
   if(v == NULL) {
    log_level = WARNING;
   }
   else if(strcmp(v, "INFO\n") == 0)
       log_level = INFO;
   else if(strcmp(v, "ERROR\n") == 0)
        log_level = ERROR;
   else
    log_level = WARNING;
   if(v)
    free(v);
   Log_set_level(log_level);
   v = config_read("PRINT_REPORT");
   if(v == NULL)
    show_reports = 0;
   else if(strcmp(v, "TRUE\n") == 0)
    show_reports = 1;
   else
    show_reports = 0;
   if(v)
    free(v);
   return;
}

