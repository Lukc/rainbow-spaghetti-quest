#include <stdlib.h>

#include "queue.h"

Queue*
queue_new()
{
	Queue* q = malloc(sizeof(*q));

	q->tail = NULL;
	q->head = NULL;

	return q;
}

void
queue_add(Queue* q, void* value)
{
	List* l;

	l = malloc(sizeof(*l));
	l->data = value;
	l->next = NULL;

	if (q->tail)
		q->tail->next = l;
	else
		q->head = l;

	q->tail = l;
}

void
queue_free(Queue* q, void cb(void*))
{
	list_free(q->head, cb); /* q->tail’s in q->head, don’t double-free! */
	free(q);
}

