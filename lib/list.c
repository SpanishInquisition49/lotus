#include "list.h"
#include "memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void list_add(l_list_t *list, void *data) {
  l_list_t new = mem_calloc(1, sizeof(l_node_t));
  new->data = data;
  new->next = *list;
  *list = new;
  return;
}

void list_dl_add(dl_list_t *list, void *data) {
  dl_list_t new = mem_calloc(1, sizeof(dl_node_t));
  new->data = data;
  new->next = *list;
  new->prev = NULL;
  if (*list)
    (*list)->prev = new;
  *list = new;
  return;
}

int list_len(l_list_t list) {
  int len = 0;
  l_list_t head = list;
  while (head != NULL) {
    len++;
    head = head->next;
  }
  return len;
}

int list_dl_len(dl_list_t list) {
  int len = 0;
  dl_list_t head = list;
  while (head) {
    len++;
    head = head->next;
  }
  return len;
}

l_list_t list_reverse(l_list_t l) {
  l_list_t res = NULL;
  l_list_t current = l;
  while (current) {
    list_add(&res, current->data);
    current = current->next;
  }
  return res;
}

void list_reverse_in_place(l_list_t *l) {
  l_list_t prev = NULL;
  l_list_t current = *l;
  l_list_t next;
  while (current != NULL) {
    next = current->next;
    current->next = prev;
    prev = current;
    current = next;
  }
  *l = prev;
  return;
}

void list_free(l_list_t list, call_back_free_t fn_free) {
  if (list == NULL)
    return;
  l_list_t head = list;
  while (head != NULL) {
    if (fn_free != NULL) {
      fn_free(head->data);
    } else {
      mem_free(head->data);
    }
    l_list_t tmp = head;
    head = head->next;
    tmp->next = NULL;
    mem_free(tmp);
  }
  return;
}

void list_dl_free(dl_list_t list, call_back_free_t fn_free) {
  if (list == NULL)
    return;
  dl_list_t head = list;
  while (head) {
    if (fn_free)
      fn_free(head->data);
    else
      mem_free(head->data);
    dl_list_t tmp = head;
    head = head->next;
    tmp->next = NULL;
    tmp->prev = NULL;
    mem_free(tmp);
  }
}
