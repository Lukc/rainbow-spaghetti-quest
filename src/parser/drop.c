#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "drop.h"

#include "../items.h"

void
parser_load_drop(Game* game, Drop* drop)
{
	drop->item = get_item_by_name(game->items, drop->item_name);

	if (!drop->item)
	{
		fprintf(stderr,
			"Item “%s” does not exist!\n", drop->item_name);

		exit(1);
	}

	free(drop->item_name);
}

Drop*
parser_get_drop(ParserElement* element)
{
	List* list;
	Drop* drop;

	if (element->type != PARSER_LIST)
	{
		fprintf(stderr, "<%s:%i> %s is not a list.\n",
			element->filename, element->lineno, element->name);

		return NULL;
	}
	else
	{
		drop = (Drop*) malloc(sizeof(Drop));
		memset(drop, 0, sizeof(Drop));
		drop->quantity = 1;

		for (list = element->value; list; list = list->next)
		{
			element = list->data;

			if (!strcmp(element->name, "item"))
			{
				/* Will be transformed into an Item* later */
				drop->item_name = parser_get_string(element, NULL);
			}
			else if (!strcmp(element->name, "rarity"))
				drop->rarity = parser_get_integer(element, NULL);
			else if (!strcmp(element->name, "quantity"))
				drop->quantity = parser_get_integer(element, NULL);
			else
				fprintf(stderr, "<%s:%i> Unknown element: %s.\n",
					element->filename, element->lineno, element->name);
		}

		return drop;
	}
}

