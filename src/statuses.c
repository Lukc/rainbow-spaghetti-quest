
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

