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

/**
 * Initialize the given environment
 * @param e a pointer to the Env to initialize
 */
void env_init(env_t*);

/**
 * Bind the Ide x Val pair to the given environment
 * @param e a pointer to the Env
 * @param identifier the identifier associated to the value
 * @param value a pointer to the value to bind
 */
void env_bind(env_t*, char*, void*);

/**
 * Get the value associated to the given Ide in the given Env
 * @param e a pointer to the Env
 * @param key the identifier to search
 * @return a void ptr to the value if found NULL otherwise  
 */
void *env_get(env_t*, char*);

/**
 * Change the value associated to the given Ide with the given Val in the given Env
 * @param e a pointer to the Env
 * @param key the identifier for the value to be changed
 * @param new_val a pointer to the new value
 * @return the old value on a successful call, NULL otherwise 
 */
void *env_set(env_t*, char*, void*);

/**
 * Unbind the last Ide x Val pair from the given environment
 * @param e a pointer to the Env
 */
void env_unbind(env_t*);

/**
 * Destroy the given environment
 * @param e a pointer to the Env 
 */
void env_destroy(env_t*);

/**
 * Unbind the last n Ide x Val pairs in the given environment
 * @param current a pointer to the Env
 * @param old_size the size of the Env to restore
 */
void env_restore(env_t*, int);

/**
 * Bind multiple Ide x Val pairs to the given environment
 * @param env a pointer to the Env
 * @param identifiers a list of identifiers
 * @param values a list of value pointers
 */
int env_bulk_bind(env_t*, l_list_t, l_list_t);

#endif // !ENVIRONMENT_H
#define ENVIRONMENT_H
