#include "list.h"
#include "memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void list_add(List *list, void *data) {
   List new = mem_calloc(1, sizeof(ListNode));
   new->data = data;
   new->next = *list;
   *list = new;
   return;
}

int list_len(List list) {
    int len = 0;
    List head = list;
    while(head != NULL){
        len++;
        head = head->next;
    }
    return len;
}

List list_reverse(List l) {
    List res = NULL;
    List current = l;
    while(current) {
        list_add(&res, current->data);
        current = current->next;
    }
    return res;
}

void list_reverse_in_place(List *l) {
    List prev = NULL;
    List current = *l;
    List next;
    while (current != NULL)
    {
        next = current->next; 
        current->next = prev;  
        prev = current;
        current = next;
    }
    *l = prev;
    return;
}

List list_dup(List l) {
    List res = NULL;
    list_reverse_in_place(&l);
    List current = l;
    while(current) {
        list_add(&res, current->data);
        current = current->next;
    }
    list_reverse_in_place(&l);
    return res;
}

void list_free(List list, CallBackFree fn_free) {
    if(list == NULL) return;
    List head = list;
    while(head != NULL) {
        if(fn_free != NULL) {
            fn_free(head->data);
        }
        else {
            free(head->data);
        }
        List tmp = head;
        head = head->next;
        free(tmp);
    }
    return;
}
