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
	int i;
	int count;

	for (list = of->class->drop; list; list = list->next)
	{
		drop = list->data;
		item = drop->item;

		count = get_count_from_inventory(to->inventory, item);

		if (item->slot != -1 && to->equipment[item->slot] == item)
			count += 1;

		if (!item->unique || !count)
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
