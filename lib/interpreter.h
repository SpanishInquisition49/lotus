#ifndef INTERPRETER_H
#define INTERPRETER_H
#include "syntax.h"

typedef Exp_literal_t Value;

typedef struct {
    Exp_t *ast;
    Value *result;
} Interpreter;

void interpreter_init(Interpreter*, Exp_t*);

void interpreter_destroy(Interpreter);

void interpreter_eval(Interpreter*);

#endif // !INTERPRETER_HINTERPRETER_H
