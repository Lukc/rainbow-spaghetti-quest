
#ifndef QUEUE_H
#define QUEUE_H

#include "list.h"

/**
 * @note: We’re only using the “List” structure and its freeing code, nothing
 * more.
 */
typedef struct {
	List* head;
	List* tail;
} Queue;

Queue* queue_new();
void queue_add(Queue*, void*);
void queue_free(Queue*, void(void*));

#endif

