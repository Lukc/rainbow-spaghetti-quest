#include <stdlib.h>

#include "list.h"
#include "entities.h"
#include "items.h"
#include "drop.h"
#include "shop.h"

/**
 * Gives random drop from a list of Drop* to an Entity*. Returns a list
 * of the given Item*s.
 *
 * @param droplist: List* of Drop*
 * @return List* of Item*
 */
List*
give_drop(Entity* to, List* droplist)
{
	List* out = NULL;
	List* list;
	Drop* drop;
	Item* item;
	int i;

	for (list = droplist; list; list = list->next)
	{
		drop = list->data;
		item = drop->item;

		for (i = 0; i < drop->quantity; i++)
			if (rand() % drop->rarity == 0)
			{
				if (give_item(to, drop->item))
					list_add(&out, (void*) drop->item);
				/* Maybe we should notify in the future if the items couldn’t
				 * be given and have been destroyed? */
			}
	}

	return out;
}

/* vim: set ts=4 sw=4 cc=80 : */
