#ifndef GARBAGE_H
#define GARBAGE_H
#include "thread.h"
#include "list.h"
#include "syntax.h"
#include "environment.h"

typedef struct {
    LiteralType type;
    void *value;
    int status;
} value_t;

typedef struct {
    dl_list_t values;
    Env *environment;
    l_list_t temporary_values;
    mutex *mtx_memory;
    cond *cond_between_statements;
    int marked;
    int sweeped;

} garbage_collector_t;


void gc_init(garbage_collector_t*, Env*, mutex*, cond*);
void gc_destroy(garbage_collector_t*);
void gc_run(garbage_collector_t*);
// Store a value in a temporary stack
void gc_store(garbage_collector_t*, value_t*);
// Discard the last stored values in the stack
void gc_discard(garbage_collector_t*, int);

value_t* gc_init_number(garbage_collector_t*, double);
value_t* gc_init_boolean(garbage_collector_t*, int);
value_t* gc_init_string(garbage_collector_t*, char*);
value_t* gc_init_closure(garbage_collector_t*, closure_t);
value_t* gc_init_nil(garbage_collector_t*);

#endif // !GARBAGE_H
