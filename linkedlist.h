#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

typedef struct LinkedListNode {
	int value;
	struct LinkedListNode *next, *prev;
} LinkedListNode;
typedef struct {
	LinkedListNode *front, *back;
	int size;
} LinkedList;

void make_linked_list(LinkedList *queue);
void linked_list_add(LinkedList *queue, int value);
int linked_list_pop_front(LinkedList *queue);
int linked_list_pop_back(LinkedList *queue);
void free_linked_list(LinkedList *queue);
void print_linked_list(LinkedList *queue);

#endif