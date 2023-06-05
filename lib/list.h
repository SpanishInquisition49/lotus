#ifndef  LIST_H
#define  LIST_H
#include  <stdlib.h>

typedef struct l_node {
    void *data;
    struct l_node *next;
} ListNode;

typedef ListNode *List;
typedef void(*CallBackFree)(void*);

void list_add(List*, void*);

int list_len(List);

void list_free(List, CallBackFree);

List list_reverse(List);

void list_reverse_in_place(List*);

List list_dup(List);

#endif // !LIST_H
