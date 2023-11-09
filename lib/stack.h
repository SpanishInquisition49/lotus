#ifndef STACK_H
#define STACK_H
#include "./environment.h"
#include <setjmp.h>

#define MAX_STACK_SIZE 100000

typedef struct {
  jmp_buf lr;
  env_t *local_env;
} stack_frame_t;

typedef struct {
  stack_frame_t stack[MAX_STACK_SIZE];
  int stack_pointer;
} stack_t;

#endif // !STACK_H
