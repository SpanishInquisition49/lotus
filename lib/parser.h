#ifndef PARSER_H
#define PARSER_H
#include "list.h"
#include "errors.h"
#include "syntax.h"
#include "list.h"

typedef struct {
    List tokens;
    int current;
    int errors[LOG_LEVELS];
} Parser;

void parser_init(Parser*, List tokens);

void parser_destroy(Parser);

List parser_parse(Parser*);


void parser_errors_report(Parser);

int parser_had_errors(Parser);

#endif // !PARSER_H
