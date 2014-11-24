#include <stdlib.h>

#include "list.h"
#include "entities.h"
#include "items.h"
#include "drop.h"
#include "shop.h"

/**
 * Give to the first entity the drop it should receive if it killed the
 * second one.
 *
 * @return List* of Item*
 */
List*
give_drop(Entity* to, Entity* of)
{
	List* out = NULL;
	List* list;
	Drop* drop;
	Item* item;

	for (list = of->class->drop; list; list = list->next)
	{
		drop = list->data;
		item = drop->item;

		if (rand() % drop->rarity == 0)
		{
			if (give_item(to, drop->item) < 0)
				return out;
			else
				list_add(&out, (void*) drop->item);
		}
	}

	return out;
}

/* vim: set ts=4 sw=4 cc=80 : */
