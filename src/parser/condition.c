#include <stdlib.h>
#include <stdio.h>

#include "condition.h"

void
parser_load_condition(Game* game, Condition* condition)
{
	List* l;

	for (l = condition->items; l; l = l->next)
	{
		ItemStack* stack = l->data;

		char* name;
		Item* item;

		name = (char*) stack->item;
		item = get_item_by_name(game->items, name);

		if (item)
		{
			l->data = item;
			free(name);
		}
		else
		{
			fprintf(stderr, "Unknown item: %s.\n", name);
			exit(1);
		}
	}

	for (l = condition->has_statuses; l; l = l->next)
	{
		Status* status;
		char* name = l->data;

		status = get_status_by_name(game->statuses, name);

		if (status)
		{
			l->data = status;
			free(name);
		}
		else
		{
			fprintf(stderr, "Unknown status: %s.\n", name);
			exit(1);
		}
	}
}

