#include <stdlib.h>

#include "list.h"

void
list_add(List** list, void* data)
{
	List* new_list;

	new_list = (List*) malloc(sizeof(List));

	new_list->data = data;
	new_list->next = *list;

	*list = new_list;
}

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

