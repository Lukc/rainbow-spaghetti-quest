#include <stdlib.h>

#include "list.h"

/**
 * Adds an element in a list. The added element becomes the new head of
 * the list.
 *
 * *list can be NULL, in which case a new list is created.
 */
void
list_add(List** list, void* data)
{
	List* new_list;

	new_list = (List*) malloc(sizeof(List));

	new_list->data = data;
	new_list->next = *list;

	*list = new_list;
}

/**
 * Returns the nth element of a list. Indexes start at 0, just like in
 * arrays.
 */
void*
list_nth(List* list, int nth)
{
	int i = 0;

	for(; list; list = list->next)
	{
		if (i == nth)
			return list->data;

		i++;
	}

	return NULL;
}

int
list_size(List* list)
{
	int i = 0;

	for(; list; list = list->next)
		i++;

	return i;
}

/**
 * Returns a new list with the content of the first one in reverse order.
 * Frees the list given as parameter.
 */
List*
list_rev_and_free(List* list)
{
	List* new_list = NULL;
	List* temp = NULL;

	while (list)
	{
		list_add(&new_list, list->data);

		temp = list;
		list = temp->next;
		free(temp);
	}

	return new_list;
}

/**
 * Frees a list.
 *
 * @param cb: A callback to use on each list->data when freeing the list.
 */
void
list_free(List* l, void cb(void*))
{
	List* next;

	if (cb)
	{
		for (; l; l = next)
		{
			next = l->next;

			cb(l->data);

			free(l);
		}
	}
	else
	{
		for (; l; l = next)
		{
			next = l->next;
			free(l);
		}
	}
}

