#ifndef GARBAGE_H
#define GARBAGE_H
#include "environment.h"
#include "list.h"
#include "syntax.h"
#include "thread.h"

typedef struct {
  literal_type_t type;
  void *value;
  int status;
} value_t;

typedef struct {
  dl_list_t values;
  env_t *environment;
  l_list_t temporary_values;
  mutex *mtx_memory;
  cond *cond_between_statements;
  int marked;
  int swept;

} garbage_collector_t;

/**
 * Initialize the given garbage collector
 * @param gc a pointer to the GC to initialize
 * @param env a pointer to the environment used at runtime
 * @param mtx a pthread_mutex_t pointer to the mutex used for the "stop the
 * world"
 * @param cond a pthread_cond_t pointer used by the interpreter to signal the GC
 * when it's safe to run
 */
void gc_init(garbage_collector_t *, env_t *, mutex *, cond *);

/**
 * Destroy the given garbage collector
 * @param gc a pointer to the GC to destroy
 */
void gc_destroy(garbage_collector_t *);

/**
 * Run a single time the "Mark & Sweep" algorithm
 * @param gc a pointer to the GC that will run the Mark & Sweep
 */
void gc_run(garbage_collector_t *);

/**
 * Hold a given value and prevent the deletion until release
 * @param gc a pointer to the GC that will hold the value
 * @param val a pointer to the value to hold
 */
void gc_hold(garbage_collector_t *, value_t *);

/**
 *  Release the last x values hold by the garbage collector
 * @param gc a pointer to the GC that hold the values
 * @param count the number of values to release
 * @note The released values are not instantly swept
 */
void gc_release(garbage_collector_t *, int);

/**
 * Initialize a number value in the lotus language and add it in the GC values
 * list
 * @param gc a pointer to the GC that will track the value
 * @param v the number value
 * @return a pointer to the value created by the GC
 */
value_t *gc_init_number(garbage_collector_t *, double);

/**
 * Initialize a number value in the lotus language and add it in the GC values
 * list
 * @param gc a pointer to the GC that will track the value
 * @param v the boolean value
 * @return a pointer to the value created by the GC
 */
value_t *gc_init_boolean(garbage_collector_t *, int);

/**
 * Initialize a number value in the lotus language and add it in the GC values
 * list
 * @param gc a pointer to the GC that will track the value
 * @param s the string value
 * @return a pointer to the value created by the GC
 */
value_t *gc_init_string(garbage_collector_t *, char *);

/**
 * Initialize a number value in the lotus language and add it in the GC values
 * list
 * @param gc a pointer to the GC that will track the value
 * @param c the closure value
 * @return a pointer to the value created by the GC
 */
value_t *gc_init_closure(garbage_collector_t *, closure_t);

/**
 * Initialize a nil value in the lotus language and add it in the GC values list
 * @param gc a pointer to the GC that will track the value
 * @return a pointer to the value created by the GC
 */
value_t *gc_init_nil(garbage_collector_t *);

#endif // !GARBAGE_H
