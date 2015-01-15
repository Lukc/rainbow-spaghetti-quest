#include <stdlib.h>
#include <stdio.h>

#include "event.h"
#include "condition.h"

#include "../events/condition.h"
#include "../events/items.h"

void
parser_load_event(Game* game, Event* event)
{
	if (event->type == EVENT_CONDITION)
	{
		ConditionEvent* e = (ConditionEvent*) event;

		parser_load_condition(game, &e->condition);
	}
	else if (event->type == EVENT_GIVE_ITEM ||
			 event->type == EVENT_REMOVE_ITEM)
	{
		GiveItemEvent* e = (GiveItemEvent*) event;
		char* name = (char*) e->item;

		e->item = get_item_by_name(game->items, name);

		if (!e->item)
		{
			fprintf(stderr, "Non-existent item: %s\n", name);

			exit(1);
		}
	}
}

