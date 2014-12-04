#include <string.h>

#include "statuses.h"

char*
status_to_string(int status)
{
	switch (status)
	{
		case STATUS_CONFUSED:
			return "confused";
		case STATUS_FROZEN:
			return "frozen";
		default:
			return "unknown status";
	}
}

int
string_to_status(char* string)
{
	int i;

	for (i = 0; i < STATUS_MAX; i++)
	{
		if (!strcmp(status_to_string(i), string))
			return i;
	}

	return -1;
}

int
has_status(Entity* e, int status)
{
	List* list;

	for (list = e->statuses; list; list = list->next)
	{
		if ((int) list->data == status)
			return 1;
	}

	return 0;
}

