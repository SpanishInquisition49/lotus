#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H
#include "list.h"
typedef struct {
    char *identifier;
    void *value;
} env_item_t;

typedef struct {
    l_list_t env;
    int size;
} env_t;

void env_init(env_t*);
void env_bind(env_t*, char*, void*);
void *env_get(env_t*, char*);
void *env_set(env_t*, char*, void*);
void env_unbind(env_t*);
void env_destroy(env_t*);
void env_restore(env_t*, int);
int env_bulk_bind(env_t*, l_list_t, l_list_t);

#endif // !ENVIRONMENT_H
#define ENVIRONMENT_H
