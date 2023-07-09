#ifndef PARSER_H
#define PARSER_H
#include "list.h"
#include "errors.h"
#include "syntax.h"
#include "list.h"

typedef struct {
    l_list_t tokens;
    int current;
    int errors[LOG_LEVELS];
} Parser;

void parser_init(Parser*, l_list_t tokens);

void parser_destroy(Parser);

l_list_t parser_parse(Parser*);


void parser_errors_report(Parser);

int parser_had_errors(Parser);

#endif // !PARSER_H
