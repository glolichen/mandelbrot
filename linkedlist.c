#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include "linkedlist.h"

void make_linked_list(LinkedList *ll) {
	ll->front = NULL, ll->back = NULL;
	ll->size = 0;
}

void linked_list_add(LinkedList *ll, int value) {
	LinkedListNode *add = (LinkedListNode *) malloc(sizeof(LinkedListNode));
	assert(add != NULL);
	add->value = value;
	add->next = NULL;
	add->prev = ll->back;
	if (ll->front == NULL && ll->back == NULL) {
		ll->front = add;
		ll->back = add;
	}
	else {
		assert(ll->front != NULL && ll->back != NULL);
		ll->back->next = add;
		ll->back = ll->back->next;
	}
	ll->size++;
}
int linked_list_pop_front(LinkedList *ll) {
	assert(ll->size != 0);
	LinkedListNode *front = ll->front;
	ll->front = front->next;
	if (ll->front == NULL)
		ll->back = NULL;
	int ret = front->value;
	free(front);
	ll->size--;
	return ret;
}
int linked_list_pop_back(LinkedList *ll) {
	assert(ll->size != 0);
	LinkedListNode *back = ll->back;
	ll->back = back->prev;
	if (ll->back == NULL)
		ll->front = NULL;
	int ret = back->value;
	free(back);
	ll->size--;
	return ret;
}

void free_linked_list(LinkedList *ll) {
	LinkedListNode *next, *current = ll->front;
	while (current != NULL) {
		next = current->next;
		free(current);
		current = next;
	}
}

void print_linked_list(LinkedList *ll) {
	LinkedListNode *next, *current = ll->front;
	printf("[");
	while (current != NULL) {
		if (current != ll->front)
			printf(", ");
		printf("%d", current->value);
		next = current->next;
		current = next;
	}
	printf("]\n");
}
