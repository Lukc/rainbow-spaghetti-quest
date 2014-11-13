#include <stdlib.h>

#include "list.h"
#include "entities.h"
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
	int i;

	for (list = of->class->drop; list; list = list->next)
	{
		drop = list->data;

		if (rand() % drop->rarity == 0)
			for (i = 0; i < INVENTORY_SIZE; i++)
				if (!to->inventory[i])
				{
					to->inventory[i] = drop->item;
					list_add(&out, (void*) drop->item);

					i = INVENTORY_SIZE;
				}
	}

	return out;
}

/* vim: set ts=4 sw=4 cc=80 : */
