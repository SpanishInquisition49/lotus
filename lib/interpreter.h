#ifndef INTERPRETER_H
#define INTERPRETER_H
#include "syntax.h"
#include "environment.h"

typedef Exp_literal_t Value;

typedef struct {
    List statements;
    Env *environment;
} Interpreter;

void interpreter_init(Interpreter*, List);

void interpreter_destroy(Interpreter);

void interpreter_eval(Interpreter*);

#endif // !INTERPRETER_H
