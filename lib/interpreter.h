#ifndef INTERPRETER_H
#define INTERPRETER_H
#include "environment.h"
#include "garbage.h"
#include "list.h"
#include "syntax.h"
#include <setjmp.h>

#define STACK_SIZE 100000

typedef struct {
  l_list_t statements;
  env_t *environment;
  garbage_collector_t *garbage_collector;
  value_t *returned_value;
} interpreter_t;

/**
 * Initialize the given interpreter
 * @param interpreter a pointer to the interpreter to initialize
 * @param env a pointer to the environment to use
 * @param statements a list of statements to be interpreted
 * @param garbage_collector a pointer to the GC to use
 */
void interpreter_init(interpreter_t *, env_t *, l_list_t,
                      garbage_collector_t *);

/**
 * Destroy the given interpreter
 * @param interpreter a pointer to the interpreter to destroy
 */
void interpreter_destroy(interpreter_t);

/**
 * Run the given interpreter
 * @param interpreter a pointer to the interpreter to run
 */
void interpreter_eval(interpreter_t *);

#endif // !INTERPRETER_H
