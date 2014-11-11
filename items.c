#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>

#include "entities.h"
#include "items.h"
#include "types.h"

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
check_type_resistance(Item* item, char* field, char* value)
{
	char* type;
	int i;
	size_t len;

	for (i = 0; i < TYPE_MAX; i++)
	{
		type = type_string(i);

		len = strlen(type);

		if (
			!strncmp(field, type, len) &&
			field[len] == ' ' &&
			!strcmp(field + len + 1, "defense"))
		{
			item->type_resistance[i] = atoi(value);

			return 1;
		}
	}

	return 0;
}

static int
check_slot(Item* item, char* field, char* value)
{
	char* equipment;
	int i;

	if (!strcmp(field, "slot"))
	{
		for (i = 0; i < EQ_MAX; i++)
		{
			equipment = equipment_string(i);

			if (!strcmp(value, equipment))
			{
				item->slot = i;

				return 1;
			}
		}
	}

	return 0;
}

Item*
load_item (char* filename)
{
	FILE* f = fopen(filename, "r");
	char* str = NULL;
	size_t n = 0;
	Item* item;
	int i;

	item = (Item*) malloc(sizeof(Item));

	memset(item, 0, sizeof(Item));

	while (getline(&str, &n, f) > 0)
	{
		char* line;
		char* field;
		char* value;

		if (str[0] != '#')
			line = strtok(str, "#");
		else
			continue;

		field = strtok(line, ":");
		value = strtok(NULL, ":");

		while (value[0] && value[0] == ' ')
			value++;

		value[strlen(value)-1] = '\0';

		for (i = 0; field[i]; i++)
			field[i] = tolower(field[i]);

		if (!strcmp(field, "name"))
			item->name = strdup(value);
		else if (!strcmp(field, "id"))
			item->id = atoi(value);
		else if (!strcmp(field, "price"))
			item->price = atoi(value);
		else if (!strcmp(field, "attack"))
		{
			int damage, strikes, i;
			char* type;

			if (sscanf(value, "%d-%d ", &damage, &strikes) < 2)
			{
				fprintf(stderr, " [%s]> Attack could not be parsed: %s.\n", filename, value);
			}

			for (i = 0; value[i] && value[i] != ' '; i++)
				;;

			for (; value[i] && value[i] == ' '; i++)
				;;

			if (value[i])
			{
				Attack* attack;

				attack = (Attack*) malloc(sizeof(Attack));

				type = value + i;

				attack->damage = damage;
				attack->strikes = strikes;
				attack->type = type_id(type);

				list_add(&item->attacks, (void*) attack);
			}
			else
			{
				fprintf(stderr, " [%s]> No attack type spectified in attack field!\n", filename);
			}
		}
		else if (check_slot(item, field, value)) ;
		else if (check_type_resistance(item, field, value)) ;
		else if (!strcmp(field, "attack bonus"))
			item->attack_bonus = atoi(value);
		else if (!strcmp(field, "defense bonus"))
			item->defense_bonus = atoi(value);
		else
			fprintf(stderr, " [%s]> Unknown field: %s\n", filename, field);
	}

	free(str);

	fclose(f);

	return item;
}

Item*
get_item_by_id(Battle* battle, int id)
{
	List* container;
	Item* item;

	container = battle->items;
	while (container)
	{
		item = container->data;

		if (item->id == id)
			return item;

		container = container->next;
	}

	return NULL;
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

		if (!strcmp((char*) list->data, name))
			return item;

		list = list->next;
	}

	return NULL;
}

int
get_count_from_inventory(int inventory[INVENTORY_SIZE], int id)
{
	int i;
	int count = 0;

	for (i = 0; i < INVENTORY_SIZE; i++)
		if (inventory[i] == id)
			count++;

	return count;
}

/* vim: set ts=4 sw=4 cc=80 : */
