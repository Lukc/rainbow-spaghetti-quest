#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>

#include "entities.h"
#include "items.h"
#include "types.h"

#include "colors.h"

#include "parser.h"

#include "parser/attack.h"

static int
check_type_resistance(Item* item, ParserElement* element, Logs* logs)
{
	char* type;
	int i;
	size_t len;

	for (i = 0; i < TYPE_MAX; i++)
	{
		type = type_to_string(i);

		len = strlen(type);

		if (
			!strncmp(element->name, type, len) &&
			element->name[len] == ' ' &&
			!strcmp(element->name + len + 1, "resistance"))
		{
			item->type_resistance[i] = parser_get_integer(element, logs);

			return 1;
		}
	}

	return 0;
}

/**
 * Gets the slot number matching a string representing the slot.
 * For example, "weapon" becomes 0.
 */
static int
get_slot(char* string, Logs* logs)
{
	char* equipment;
	char* log;
	int i;

	for (i = 0; i < EQ_MAX; i++)
	{
		equipment = equipment_string(i);

		if (!strcmp(string, equipment))
			return i;
	}

	log = (char*) malloc(sizeof(char) * 128);
	snprintf(log, 128, "Invalid slot: “%s”.", string);

	logs_add(logs, log);

	return 0;
}

void
load_item(Game* game, List* list)
{
	ParserElement* element;
	Item* item;
	Logs* logs;

	item = (Item*) malloc(sizeof(Item));

	memset(item, 0, sizeof(Item));

	/* Can’t be equipped by default */
	item->slot = -1;

	logs = logs_new();

	while (list)
	{
		char* field;

		element = list->data;

		field = element->name;

		if (!strcmp(field, "name"))
			item->name = parser_get_string(element, logs);
		else if (!strcmp(field, "unique"))
			/* FIXME: Maybe we gotta have a boolean type here… */
			item->unique = parser_get_integer(element, logs);
		else if (!strcmp(field, "consumable"))
			item->consumable = parser_get_integer(element, logs);
		else if (!strcmp(field, "health on focus"))
			item->health_on_focus = parser_get_integer(element, logs);
		else if (!strcmp(field, "mana on focus"))
			item->mana_on_focus = parser_get_integer(element, logs);
		else if (!strcmp(field, "price"))
			item->price = parser_get_integer(element, logs);
		else if (!strcmp(field, "attack bonus"))
			item->attack_bonus = parser_get_integer(element, logs);
		else if (!strcmp(field, "defense bonus"))
			item->defense_bonus = parser_get_integer(element, logs);
		else if (!strcmp(field, "max health"))
			item->health_bonus = parser_get_integer(element, logs);
		else if (!strcmp(field, "max mana"))
			item->mana_bonus = parser_get_integer(element, logs);
		else if (!strcmp(field, "on use"))
			item->on_use = parser_get_attack(element);
		else if (!strcmp(field, "slot"))
		{
			char* slot = parser_get_string(element, logs);

			if (slot)
				item->slot = get_slot(slot, logs);
		}
		else if (!strcmp(field, "attack"))
		{
			Attack* attack = parser_get_attack(element);

			if (attack)
				list_add(&item->attacks, (void*) attack);
		}
		else if (check_type_resistance(item, element, logs))
			;
		else
		{
			char* log = (char*) malloc(sizeof(char) * 128);
			snprintf(log, 128, "Unknown field: “%s”.", element->name);
			logs_add(logs, log);
		}

		list = list->next;
	}

	if (logs->head)
	{
		/* FIXME: stderr */
		logs_print(logs);
		logs_free(logs);
	}

	item->attacks = list_rev_and_free(item->attacks);

	list_add(&game->items, item);
}

/**
 * @param list: List* of Item*
 */
Item*
get_item_by_name(List* list, char* name)
{
	Item* item;

	while (list)
	{
		item = list->data;

		if (!strcmp(((Item*) list->data)->name, name))
			return item;

		list = list->next;
	}

	return NULL;
}

/**
 * Checks whether or not an Entity* has an Item* in its inventory or equipment.
 */
int
possesses_item(Entity* e, Item* item)
{
	int i;

	for (i = 0; i < INVENTORY_SIZE; i++)
	{
		ItemStack* s = e->inventory + i;

		if (s && s->item == item)
			return 1;
	}

	for (i = 0; i < EQ_MAX; i++)
		if (e->equipment[i] == item)
			return 1;

	return 0;
}

/**
 * Counts how many times a given Item* is present in an inventory.
 * It counts the items in stacks and not just the number of stacks of the
 * given item.
 */
int
get_count_from_inventory(ItemStack inventory[INVENTORY_SIZE], Item* item)
{
	int i;
	int count = 0;

	for (i = 0; i < INVENTORY_SIZE; i++)
		if (inventory[i].item == item)
			count += inventory[i].quantity;

	return count;
}

/**
 * Checks whether or not an item can has effects during a battle.
 */
int
is_item_usable(Item* item)
{
	return item->on_use != NULL;
}

/**
 * Adds an item to the inventory of an Entity*.
 * It tries to stack non-equipment whenever possible.
 * Unique items already in the player’s inventory are not added.
 *
 * @return -1: No space left in which to add the item.
 * @return -2: Item is unique and already in the player’s inventory.
 * @return (int) >= 0: The ItemStack index in which the given item was put.
 */
int
give_item(Entity* player, Item* item)
{
	int i;

	if (item->unique)
		for (i = 0; i < EQ_MAX; i++)
			if (player->equipment[i] == item)
				return -2; /* Unique item already possessed AND equipped */

	if (item->slot >= 0)
	{
		for (i = 0; i < INVENTORY_SIZE && player->inventory[i].item; i++)
			if (item->unique && player->inventory[i].item == item)
				return -2; /* Unique item already possessed */
	}
	else
	{
		/* Non-equipment is stackable. */
		i = 0;
		while (
			i < INVENTORY_SIZE &&
			! (
				player->inventory[i].item == item &&
				player->inventory[i].quantity < 99
			)
		)
			i++;

		/* No compatible stack found, looking for free slot. */
		if (i == INVENTORY_SIZE)
		{
			for (i = 0; i < INVENTORY_SIZE && player->inventory[i].item; i++)
				;

			/* Precaution. */
			player->inventory[i].quantity = 0;
		}
		else if (item->unique && player->inventory[i].item == item)
			return -2; /* Unique item already possessed */
	}

	if (i == INVENTORY_SIZE)
		return -1; /* No space left in inventory */

	player->inventory[i].item = item;
	player->inventory[i].quantity += 1;

	return i;
}

/**
 * @return 0 if no error occurred and the item was removed. 1 otherwise.
 * @return 1 no such item or quantity is greater than number of items found.
 *
 * @precheck
 *   - player is not NULL
 *   - item is not NULL
 *   - quantity > 0
 */
int
remove_items(Entity* player, Item* item, int quantity)
{
	int i;

	for (i = 0; i < INVENTORY_SIZE && player->inventory[i].item != item; i++)
		;

	if (i == INVENTORY_SIZE)
		return 1;

	if (player->inventory[i].quantity >= quantity)
	{
		player->inventory[i].quantity -= quantity;
		if (player->inventory[i].quantity <= 0)
			player->inventory[i].item = NULL;
	}
	else
	{
		quantity -= player->inventory[i].quantity;
		player->inventory[i].item = NULL;

		return remove_items(player, item, quantity);
	}

	return 0;
}

void
free_item(void* ptr)
{
	Item* i = ptr;

	free(i->name);
	free(i);
}

/* vim: set ts=4 sw=4 cc=80 : */
