#ifndef INTERPRETER_H
#define INTERPRETER_H
#include "list.h"
#include "syntax.h"
#include "environment.h"
#include "garbage.h"

typedef Exp_literal_t Value;

typedef struct {
    l_list_t statements;
    Env *environment;
    garbage_collector_t *garbage_collector;
} Interpreter;

void interpreter_init(Interpreter*, Env*, l_list_t, garbage_collector_t*);

void interpreter_destroy(Interpreter);

void interpreter_eval(Interpreter*);

#endif // !INTERPRETER_H
