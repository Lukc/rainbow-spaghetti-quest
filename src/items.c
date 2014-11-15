#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>

#include "entities.h"
#include "items.h"
#include "types.h"

#include "parser.h"

#define MAX_ITEMS 255

List*
load_items(char* dirname)
{
	List* items = NULL;
	DIR* dir;
	struct dirent *entry;
	char* filename;

	dir = opendir(dirname);

	while ((entry = readdir(dir)))
	{
		/* Hidden files ignored. Just because. */
		if (entry->d_name[0] == '.')
			continue;

		filename = (char*) malloc(sizeof(char) * (
			strlen(dirname) + strlen(entry->d_name) + 2
		));;
		sprintf(filename, "%s/%s", dirname, entry->d_name);

		printf(" > %s\n", filename);
		list_add(&items, (void*) load_item(filename));

		free(filename);
	}

	closedir(dir);

	return items;
}

static int
check_type_resistance(Item* item, ParserElement* element, Logs* logs)
{
	char* type;
	int i;
	size_t len;

	for (i = 0; i < TYPE_MAX; i++)
	{
		type = type_string(i);

		len = strlen(type);

		if (
			!strncmp(element->name, type, len) &&
			element->name[len] == ' ' &&
			!strcmp(element->name + len + 1, "defense"))
		{
			item->type_resistance[i] = parser_get_integer(element, logs);

			return 1;
		}
	}

	return 0;
}

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

static int
get_type(char* string, Logs* logs)
{
	char* type;
	char* log;
	int i;

	for (i = 0; i < TYPE_MAX; i++)
	{
		type = type_string(i);

		if (!strcmp(string, type))
			return i;
	}

	log = (char*) malloc(sizeof(char) * 128);
	snprintf(log, 128, "Invalid type: “%s”.", string);

	logs_add(logs, log);

	return 0;
}

Item*
load_item(char* filename)
{
	List* list = load_file(filename);
	List* temp;
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
		else if (!strcmp(field, "health on use"))
			item->health_on_use = parser_get_integer(element, logs);
		else if (!strcmp(field, "mana on use"))
			item->mana_on_use = parser_get_integer(element, logs);
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
		else if (!strcmp(field, "slot"))
		{
			char* slot = parser_get_string(element, logs);

			if (slot)
				item->slot = get_slot(slot, logs);
		}
		else if (!strcmp(field, "attack"))
		{
			List* sublist;
			ParserElement* subelement;
			Attack* attack = (Attack*) malloc(sizeof(Attack));

			memset(attack, 0, sizeof(Attack));

			if (element->type != PARSER_LIST)
			{
				logs_add(logs,
					strdup("Trying to add attack improperly defined.\n"));
				free(attack);
			}
			else
				for (sublist = element->value; sublist; sublist = sublist->next)
				{
					subelement = sublist->data;

					if (!strcmp(subelement->name, "damage"))
						attack->damage =
							parser_get_integer(subelement, logs);
					else if (!strcmp(subelement->name, "strikes"))
						attack->strikes =
							parser_get_integer(subelement, logs);
					else if (!strcmp(subelement->name, "mana"))
						attack->mana_cost =
							parser_get_integer(subelement, logs);
					else if (!strcmp(subelement->name, "name"))
						attack->name =
							parser_get_string(subelement, logs);
					else if (!strcmp(subelement->name, "type"))
					{
						char* type = parser_get_string(subelement, logs);

						if (type)
							attack->type = get_type(type, logs);
					}
				}

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

		parser_free(element);

		temp = list;
		list = list->next;

		free(temp);
	}

	if (logs->head)
	{
		/* FIXME: stderr */
		logs_print(logs);
		logs_free(logs);

		exit(1);
	}

	return item;
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

int
get_count_from_inventory(Item* inventory[INVENTORY_SIZE], Item* item)
{
	int i;
	int count = 0;

	for (i = 0; i < INVENTORY_SIZE; i++)
		if (inventory[i] == item)
			count++;

	return count;
}

int
is_item_usable(Item* item)
{
	return item->health_on_use || item->mana_on_use;
}

/* vim: set ts=4 sw=4 cc=80 : */
