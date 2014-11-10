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

