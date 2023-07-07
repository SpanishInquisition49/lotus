#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H
#include "list.h"

typedef struct env_s{
    char *identifier;
    void *value;
    struct env_s* prev;
} Env;

void env_init(Env*);
Env *env_bind(Env*, char*, void*);
void *env_get(Env*, char*);
void *env_set(Env*, char*, void*);
Env *env_unbind(Env*);
void env_destroy(Env*);
Env *env_restore(Env*, Env*);
Env *env_bulk_bind(Env*, List, List);

#endif // !ENVIRONMENT_H
#define ENVIRONMENT_H
