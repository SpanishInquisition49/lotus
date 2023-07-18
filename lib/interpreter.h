#ifndef INTERPRETER_H
#define INTERPRETER_H
#include <setjmp.h>
#include "list.h"
#include "syntax.h"
#include "environment.h"
#include "garbage.h"

#define STACK_SIZE 100000

typedef struct {
    l_list_t statements;
    env_t *environment;
    garbage_collector_t *garbage_collector;
    value_t *returned_value;
} interpreter_t;

void interpreter_init(interpreter_t*, env_t*, l_list_t, garbage_collector_t*);

void interpreter_destroy(interpreter_t);

void interpreter_eval(interpreter_t*);

#endif // !INTERPRETER_H
