#ifndef PARSER_H
#define PARSER_H
#include "list.h"
#include "errors.h"
#include "syntax.h"

typedef struct {
    List tokens;
    int current;
    int errors[LOG_LEVELS];
} Parser;

void parser_init(Parser*, List tokens);

void parser_destroy(Parser);

Exp_t *parser_generate_ast(Parser*);

void parser_errors_report(Parser);

int parser_had_errors(Parser);

#endif // !PARSER_H
