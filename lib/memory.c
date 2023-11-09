#include "memory.h"
#include <stdio.h>
#include <stdlib.h>

void *mem_calloc(size_t nmemb, size_t size) {
  void *p = calloc(nmemb, size);
  if (p == NULL) {
    perror("calloc");
    exit(EXIT_FAILURE);
  }
  return p;
}

void mem_free(void *p) {
  if (p == NULL)
    return;
  free(p);
  p = NULL;
  return;
}
