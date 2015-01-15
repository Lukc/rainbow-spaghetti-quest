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
			fprintf(stderr, "Unknown item: %s\n", name);
			exit(1);
		}
	}
}

