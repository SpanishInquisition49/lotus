#ifndef LIST_H
#define LIST_H
#include <stdlib.h>

typedef struct l_node {
  void *data;
  struct l_node *next;
} l_node_t;

typedef struct dl_node {
  void *data;
  struct dl_node *prev;
  struct dl_node *next;
} dl_node_t;

typedef l_node_t *l_list_t;
typedef dl_node_t *dl_list_t;
typedef void (*call_back_free_t)(void *);

void list_add(l_list_t *, void *);
void list_dl_add(dl_list_t *, void *);
int list_len(l_list_t);
int list_dl_len(dl_list_t);

void list_free(l_list_t, call_back_free_t);
void list_dl_free(dl_list_t, call_back_free_t);
l_list_t list_reverse(l_list_t);

void list_reverse_in_place(l_list_t *);

#endif // !LIST_H
