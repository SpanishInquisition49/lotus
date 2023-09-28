#include <string.h>
#include <stdio.h>
#include "garbage.h"
#include "list.h"
#include "memory.h"
#include "syntax.h"

#define MARKED 1
#define UNMARKED 0

/**
 * Perform a DFS og the given value
 * @param gc a pointer to the GC (used for debug purpose)
 * @param v a pointer to the value
 */
static void dfs(garbage_collector_t*, value_t*);

/**
 * Perform the marking phase of the mark and sweep algorithm
 * @param gc a pointer to the GC that will run the algorithm 
 */
static void mark(garbage_collector_t*);

/**
 * Perform the sweeping phase of the mark and sweep algorithm
 * @param gc a pointer to the GC that will run the algorithm
 */
static void sweep(garbage_collector_t*);

/**
 * Destroy the given value
 * @param val a pointer to the value to destroy
 */
static void value_destroy(value_t*);

/**
 * A wrapper for the value_destroy() function 
 */
static void value_free(void*);

void gc_init(garbage_collector_t* gc, env_t* env, mutex* mtx, cond* cond) {
    memset(gc, 0, sizeof(*gc));
    gc->values = NULL;
    gc->environment = env;
    gc->temporary_values = NULL;
    gc->mtx_memory = mtx;
    gc->cond_between_statements = cond;
    gc->marked = 0;
    gc->swept = 0;
    return;
}

void gc_destroy(garbage_collector_t *gc) {
    list_dl_free(gc->values, value_free);
    list_free(gc->temporary_values, value_free);
    return;
}

void gc_hold(garbage_collector_t *gc, value_t *val) {
    list_add(&gc->temporary_values, val);
    return;
}

void gc_release(garbage_collector_t *gc, int count) {
    for(int i = 0; i<count; i++) {
        l_list_t current = gc->temporary_values;
        gc->temporary_values = current->next;
        current->data = NULL;
        current->next = NULL;
        mem_free(current);
    }
    return;
}

void gc_run(garbage_collector_t *gc) {
    gc->marked = 0;
    gc->swept = 0;
    int total = list_dl_len(gc->values);
    mark(gc);
    sweep(gc);
    dprintf(2, "[GC]: Count:%d -> %d\tMarked:%d\tSwept:%d\n",total, list_dl_len(gc->values), gc->marked, gc->swept);
}

void dfs(garbage_collector_t *gc, value_t* v) {
    if(v->status == UNMARKED)
        gc->marked++;
    v->status = MARKED;
    // TODO: in case of array or list
    switch (v->type) {
        case T_NIL:
        case T_NUMBER:
        case T_STRING:
        case T_CLOSURE:
        case T_BOOLEAN:
            break;
    }
    return;
}

void mark(garbage_collector_t *gc) {
    l_list_t current = gc->environment->env;
    while(current) {
        value_t *v = (value_t*)current->data;
        dfs(gc, v);
        current = current->next;
    }
    current = gc->temporary_values;
    while(current) {
        value_t *v = (value_t*)current->data;
        dfs(gc, v);
        current = current->next;
    }
    return;
}

void sweep(garbage_collector_t *gc) {
   dl_list_t current = gc->values;
   while(current) {
        dl_list_t next = current->next;
        value_t *v = (value_t*)current->data;
        if(v->status == MARKED)
            v->status = UNMARKED;
        else {
            gc->swept++;
            value_destroy(v);
            dl_list_t p = current->prev;
            dl_list_t n = current->next;
            if(p)
                p->next = n;
            if(n)
                n->prev = p;
            mem_free(current);
        }
        current = next;
   }
   return;
}

value_t *gc_init_number(garbage_collector_t* gc, double v) {
    value_t *val = mem_calloc(1, sizeof(value_t));
    double *d = mem_calloc(1, sizeof(double));
    *d= v;
    val->type = T_NUMBER;
    val->value = d;
    val->status = MARKED;
    list_dl_add(&gc->values, val);
    return val;
}

value_t *gc_init_boolean(garbage_collector_t *gc, int v) {
    value_t *val = mem_calloc(1, sizeof(value_t));
    int *i = mem_calloc(1, sizeof(int));
    *i = v;
    val->type = T_BOOLEAN;
    val->value = i;
    val->status = MARKED;
    list_dl_add(&gc->values, val);
    return val;
}

value_t *gc_init_string(garbage_collector_t *gc, char* s) {
    value_t *val = mem_calloc(1, sizeof(value_t));
    val->type = T_STRING;
    val->value = strdup(s);
    val->status = MARKED;
    list_dl_add(&gc->values, val);
    return val;
}

value_t *gc_init_closure(garbage_collector_t* gc, closure_t c) {
    value_t *val = mem_calloc(1, sizeof(value_t));
    closure_t *cls = mem_calloc(1, sizeof(closure_t));
    l_list_t f = NULL;
    l_list_t current = c.formals;
    while(current) {
        list_add(&f, strdup(current->data));
        current = current->next;
    }
    list_reverse_in_place(&f);
    cls->identifier =  strdup(c.identifier);
    cls->formals = f;
    cls->body = stmt_dup(c.body);
    val->type = T_CLOSURE;
    val->value = cls;
    val->status = MARKED;
    list_dl_add(&gc->values, val);
    return val; 
}

value_t *gc_init_nil(garbage_collector_t *gc) {
    value_t *val = mem_calloc(1, sizeof(value_t));
    val->type = T_NIL;
    val->value = NULL;
    val->status = MARKED;
    list_dl_add(&gc->values, val);
    return val;
}

void value_destroy(value_t *val) {
    if(val == NULL)
        return;
    switch(val->type) {
        case T_CLOSURE: {
                closure_t *tmp = (closure_t*)val->value;
                mem_free(tmp->identifier);
                list_free(tmp->formals, NULL);
                stmt_destroy(tmp->body);
                mem_free(tmp);
                break;
            }
        case T_STRING:
        case T_NUMBER:
        case T_BOOLEAN:
        case T_NIL:
            mem_free(val->value);
    }
    val->type = 0;
    val->status = 0;
    mem_free(val);
    return;
}

void value_free(void * v) {
    value_destroy(v);
}
