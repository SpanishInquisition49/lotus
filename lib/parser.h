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
} parser_t;

void parser_init(parser_t*, l_list_t tokens);

void parser_destroy(parser_t);

l_list_t parser_parse(parser_t*);


void parser_errors_report(parser_t);

int parser_had_errors(parser_t);

#endif // !PARSER_H
