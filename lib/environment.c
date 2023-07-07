#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "environment.h"
#include "errors.h"
#include "memory.h"

static int env_len(Env*);

int env_len(Env *e) {
    int i = 0;
    Env *current = e;
    while(current->identifier != NULL) {
        i++;
        current = current->prev;
    }
    return i;
}

void env_init(Env *e) {
    e->prev = NULL;
    e->identifier = NULL;
    e->value = NULL;
}

Env* env_bind(Env *e, char *identifier, void *value) {
    Env *new = mem_calloc(1, sizeof(Env));
    new->prev = e;
    new->value = value;
    new->identifier = strdup(identifier);
    return new;
}

void *env_get(Env *e, char *key) {
    Env current = *e;
    while(current.identifier != NULL) {
        if(strcmp(current.identifier, key) == 0)
            return current.value;
        current = *current.prev;
    }
    return NULL;
}

void *env_set(Env *e, char *key, void *new_val) {
    Env *current = e;
    while(current->identifier != NULL) {
        if(strcmp(current->identifier, key) == 0) {
            void *old_value = current->value;
            current->value = new_val;
            return old_value;
        }
    }
    return NULL;
}

Env* env_unbind(Env *e) {
    if(e->identifier == NULL)
        return e;
    free(e->identifier);
    free(e->value);
    return e->prev;
}

void env_destroy(Env *e) {
    Env *current = e;
    while(current->identifier != NULL) {
        Env *tmp = env_unbind(current);
        free(current);
        current = tmp;
    }
    return;
}

Env *env_restore(Env *current, Env *original) {
    int n = env_len(current) - env_len(original);
    for(int i=0; i<n; i++)
        current = env_unbind(current);
    return current;
}

Env *env_bulk_bind(Env *env, List identifiers, List values) {
    Env *current_env = env;
    List current_ide = identifiers;
    List current_val = values;
    while(current_ide != NULL && current_val != NULL) {
        current_env = env_bind(current_env, current_ide->data, current_val->data);
        current_val = current_val->next;
        current_ide = current_ide->next;
    }

    if(current_ide != NULL || current_val != NULL)
        return NULL;
    return current_env;
}
