/*
 * linkedlist.c
 *
 * Based on the implementation approach described in "The Practice 
 * of Programming" by Kernighan and Pike (Addison-Wesley, 1999).
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ics.h"
#include "emalloc.h"
#include "listy.h"


node_t *new_node(event_t *val) {
    assert( val != NULL);

    node_t *temp = (node_t *)emalloc(sizeof(node_t));

    temp->val = val;
    temp->next = NULL;

    return temp;
}


node_t *add_front(node_t *list, node_t *new) {
    new->next = list;
    return new;
}


node_t *add_end(node_t *list, node_t *new) {
    node_t *curr;

    if (list == NULL) {
        new->next = NULL;
        return new;
    }

    for (curr = list; curr->next != NULL; curr = curr->next);
    curr->next = new;
    new->next = NULL;
    return list;
}


node_t *peek_front(node_t *list) {
    return list;
}


node_t *remove_front(node_t *list) {
    if (list == NULL) {
        return NULL;
    }

    return list->next;
}



void apply(node_t *list,
           void (*fn)(node_t *list, void *),
           void *arg)
{
    for ( ; list != NULL; list = list->next) {
        (*fn)(list, arg);
    }
}


/*
Purpose:    Given a sorted linked list, the function will
            insert a new node while maintaining it's order.

Parameters: list - the sorted linked list to insert the node in.
            new_node - the node to be inserted.

Returns:    The head of the ordered list.

Source:     https://tinyurl.com/5whnz7at
*/
node_t *sorted_insert(node_t *list, node_t *new_node) {
    node_t *cur;
    /*If linked list is empty or the value of the node to be
      inserted is smaller than the value of the first node,
      insert the node at the start.*/
    if (list == NULL || compare(new_node->val, list->val) < 0) {
        new_node->next = list;
        return new_node;
    }

    cur = list; 
    /*Find the appropiate node after which the input node is to be inserted.*/
    while (cur->next != NULL && compare(cur->next->val, new_node->val) < 0) { 
        cur = cur->next; 
    } 
    new_node->next = cur->next; 
    cur->next = new_node;
    return list;
}


/*
Purpose:    Compare two events by there start date

Parameters: n1 - the first event.
            n2 - the second event.

Returns:   -1 - if n1 has a start date before n2.
            0 - if n1 and n2 have the same start date.
            1 - if n1 has a start date after n2.
*/
int compare(const event_t *n1, const event_t *n2) {
    return strcmp(n1->dtstart, n2->dtstart);
}


