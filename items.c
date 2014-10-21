#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>

#include "entities.h"
#include "items.h"

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
		items[n] = load_item(filename);
		n++;

		free(filename);
	}

	closedir(dir);

	memset(items + n, 0, sizeof(Item));

	return items;
}

Item
load_item (char* filename)
{
	FILE* f = fopen(filename, "r");
	char* str = NULL;
	size_t n = 0;
	int i;
	Item item;

	memset(&item, 0, sizeof(Item));

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
			item.name = strdup(value);
		else if (!strcmp(field, "id"))
			item.id = atoi(value);
		else if (!strcmp(field, "slot"))
		{
			if (!strcmp(value, "weapon"))
				item.slot = EQ_WEAPON;
			else
			{
				for (i = 0; value[i]; i++)
					value[i] = tolower(value[i]);

				fprintf(stderr,
					" [%s]> Field %s has an unrecognized value of %s\n",
					filename, field, value);
				item.slot = atoi(value);
			}
		}
		else if (!strcmp(field, "price"))
			item.price = atoi(value);
		else if (!strcmp(field, "attack bonus"))
			item.attack_bonus = atoi(value);
		else if (!strcmp(field, "defense bonus"))
			item.defense_bonus = atoi(value);
		else
			fprintf(stderr, " [%s]> Unknown field: %s\n", filename, field);
	}

	free(str);

	fclose(f);

	return item;
}

/* vim: set ts=4 sw=4 cc=80 : */
