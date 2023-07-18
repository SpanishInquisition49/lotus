#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "environment.h"
#include "errors.h"
#include "list.h"
#include "memory.h"

/**
 * Initialize a Ide x Val pair
 * @param ide the identifier
 * @param value a value pointer
 * @return a pointer to the Ide x Val pair  
 */
static env_item_t* env_item_init(char*, void*);

/**
 * Destroy the given Ide x Val pair
 * @param item a pointer to the Ide x Val pair 
 */
static void env_item_destroy(env_item_t*);

/**
 * A wrapper for the env_item_destroy 
 */
static void env_item_free(void*);

void env_init(env_t* e) {
    memset(e, 0, sizeof(*e));
    e->env = NULL;
    e->size = 0;
    return;
}

void env_bind(env_t *e, char *identifier, void *value) {
    list_add(&e->env, env_item_init(identifier, value));
    e->size++;
    return;
}

void *env_get(env_t *e, char *key) {
    l_list_t current = e->env;
    while(current != NULL) { 
        env_item_t *tmp = (env_item_t*)current->data;
        if(strcmp(tmp->identifier, key) == 0)
            return tmp->value;
        current = current->next;
    }
    return NULL;
}

void *env_set(env_t *e, char *key, void *new_val) {
    l_list_t current = e->env;
    while(current != NULL) {
        if(strcmp(((env_item_t*)current->data)->identifier, key) == 0) {
            void *old_v = ((env_item_t*)current->data)->value = new_val; 
            return old_v;
        }
        current = current->next;
    }
    return NULL;
}

void env_unbind(env_t *e) {
    if(e->env->data == NULL)
        return;
    l_list_t new_head = e->env->next;
    l_list_t tmp = e->env;
    env_item_free(tmp->data);
    e->env = new_head;
    mem_free(tmp);
    e->size--;
    return;
}

void env_destroy(env_t *e) {
    list_free(e->env, env_item_free);
    return;
}

void env_restore(env_t *current, int old_size) {
    int n = current->size - old_size;
    for(int i=0; i<n; i++)
        env_unbind(current);
    return;
}

int env_bulk_bind(env_t *env, l_list_t identifiers, l_list_t values) {
    l_list_t current_ide = identifiers;
    l_list_t current_val = values;
    while(current_ide != NULL && current_val != NULL) {
        env_bind(env, current_ide->data, current_val->data);
        current_val = current_val->next;
        current_ide = current_ide->next;
    }

    if(current_ide != NULL || current_val != NULL)
        return 0;
    return 1;
}

env_item_t *env_item_init(char *ide, void *value) {
    env_item_t *new = mem_calloc(0, sizeof(env_item_t));
    new->value = value;
    new->identifier = strdup(ide);
    return new;
}

void env_item_destroy(env_item_t *item) {
    mem_free(item->identifier);
    mem_free(item);
    return;   
}

void env_item_free(void *item) {
    env_item_destroy(item);
    return;
}
