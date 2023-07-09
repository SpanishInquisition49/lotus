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
} Env;

void env_init(Env*);
void env_bind(Env*, char*, void*);
void *env_get(Env*, char*);
void *env_set(Env*, char*, void*);
void env_unbind(Env*);
void env_destroy(Env*);
void env_restore(Env*, int);
int env_bulk_bind(Env*, l_list_t, l_list_t);

#endif // !ENVIRONMENT_H
#define ENVIRONMENT_H
