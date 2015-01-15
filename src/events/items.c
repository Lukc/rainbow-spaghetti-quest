#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "items.h"

#include "../parser.h"
#include "../events.h"

#include "../items.h"

Event*
load_item_event(ParserElement* element)
{
	GiveItemEvent* event;

	event = malloc(sizeof(*event));
	event->item = NULL;
	event->quantity = 1;

	if (!strcmp(element->name, "give item"))
		event->type = EVENT_GIVE_ITEM;
	else
		event->type = EVENT_REMOVE_ITEM;

	event->name = NULL;

	if (element->type == PARSER_STRING)
		event->item = (Item*) parser_get_string(element, NULL);
	else if (element->type == PARSER_LIST)
	{
		List* l;

		for (l = element->value; l; l = l->next)
		{
			ParserElement* element = l->data;

			if (!strcmp(element->name, "name"))
				event->name = parser_get_string(element, NULL);
			else if (!strcmp(element->name, "item"))
				event->item = (Item*) parser_get_string(element, NULL);
			else if (!strcmp(element->name, "quantity"))
				event->quantity = parser_get_integer(element, NULL);
			else
				fprintf(stderr, "[:%i] Unrecognized field “%s”.\n",
					element->lineno, element->name);
		}
	}
	else
	{
		fprintf(stderr,
			"[:%i] Event “Give Item” is not a list or string.\n",
			element->lineno);

		return NULL;
	}

	return (Event*) event;
}

void
fire_give_item_event(Game* game, Event* event)
{
	GiveItemEvent* e = (GiveItemEvent*) event;
	int i;

	for (i = 0; i < e->quantity; i++)
	{
		give_item(game->player, e->item);
	}

	if (e->quantity == 1)
		printf("Received a %s!\n", e->item->name);
	else
		printf("Received %ix %s!\n", e->quantity, e->item->name);
	printf("\n");
}

void
fire_remove_item_event(Game* game, Event* event)
{
	RemoveItemEvent* e = (RemoveItemEvent*) event;

	remove_items(game->player, e->item, e->quantity);

	if (e->quantity == 1)
		printf("Lost a %s!\n", e->item->name);
	else
		printf("Lost %ix %s!\n", e->quantity, e->item->name);
	printf("\n");
}

