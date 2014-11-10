#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>

#include "entities.h"
#include "items.h"
#include "types.h"

#define MAX_ITEMS 255

Item*
load_items(char* dirname)
{
	Item *items;
	DIR* dir;
	struct dirent *entry;
	char* filename;
	int n = 0;

	items = malloc(sizeof(Item) * MAX_ITEMS);

	memset(items + n, 0, sizeof(Item) * MAX_ITEMS);

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
		load_item(items + n, filename);
		n++;

		free(filename);
	}

	closedir(dir);

	memset(items + n, 0, sizeof(Item));

	return items;
}

static int
check_type_defense(Item* item, char* field, char* value)
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
			item->defense[i] = atoi(value);

			return 1;
		}
	}

	return 0;
}

void
load_item (Item* item, char* filename)
{
	FILE* f = fopen(filename, "r");
	char* str = NULL;
	size_t n = 0;
	int i;

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
		else if (!strcmp(field, "slot"))
		{
			for (i = 0; value[i]; i++)
				value[i] = tolower(value[i]);

			if (!strcmp(value, "weapon"))
				item->slot = EQ_WEAPON;
			else if (!strcmp(value, "shield"))
				item->slot = EQ_SHIELD;
			else if (!strcmp(value, "armor"))
				item->slot = EQ_ARMOR;
			else
			{
				fprintf(stderr,
					" [%s]> Field %s has an unrecognized value of %s\n",
					filename, field, value);
				item->slot = atoi(value);
			}
		}
		else if (!strcmp(field, "price"))
			item->price = atoi(value);
		else if (!strcmp(field, "attack type"))
		{
			if (!strcmp(value, "slashing"))
				item->attack_type = TYPE_SLASHING;
			else if (!strcmp(value, "impact"))
				item->attack_type = TYPE_IMPACT;
			else if (!strcmp(value, "piercing"))
				item->attack_type = TYPE_PIERCING;
			else
				fprintf(stderr, " [%s]> Unknown attack type.\n", filename);
		}
		else if (check_type_defense(item, field, value)) ;
		else if (!strcmp(field, "attack bonus"))
			item->attack_bonus = atoi(value);
		else if (!strcmp(field, "defense bonus"))
			item->defense_bonus = atoi(value);
		else
			fprintf(stderr, " [%s]> Unknown field: %s\n", filename, field);
	}

	free(str);

	fclose(f);
}

Item*
get_item_from_id(Battle* battle, int id)
{
	int i;

	for (i = 0; battle->items[i].id; i++)
		if (battle->items[i].id == id)
			return battle->items + i;

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
