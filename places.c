#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>

#include "places.h"
#include "items.h"
#include "classes.h"

/**
 * @return: List* of Place*
 */
List*
load_places(Battle* data, char* dirname)
{
	List* placees = NULL;
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
		list_add(&placees, load_place(data, filename));

		free(filename);
	}

	/* @todo: Should we, at this point, replace all place->destination by
	 *        pointers to destinations? */

	closedir(dir);

	return placees;
}

static List*
comas_to_list(char* input)
{
	char* string = NULL;
	int i;
	List* list = NULL;

	string = strtok(input, ",");

	while (string)
	{
		for (i = 0; string[i] == ' '; i++)
			;;

		string = string + i;

		for (i = strlen(string); string[i] && string[i] != ' '; i--)
			;;

		/* In case we stopped at ' '. */
		string[i] = '\0';

		printf(" > %s\n", string);

		list_add(&list, strdup(string));

		string = strtok(NULL, ",");
	}

	return list;
}

Place*
load_place (Battle* data, char* filename)
{
	FILE* f = fopen(filename, "r");
	char* str = NULL;
	size_t n = 0;
	Place* place;
	int i;

	place = (Place*) malloc(sizeof(Place));

	memset(place, 0, sizeof(Place));

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
			place->name = strdup(value);
		else if(!strcmp(field, "shop"))
		{
			List* temp;
			List* list = comas_to_list(value);

			while (list)
			{
				list_add(&place->shop_items,
					(void*) get_item_by_name(data->items, (char*) list->data));

				temp = list;
				list = list->next;

				free(temp->data);
				free(temp);
			}

		}
		else if(!strcmp(field, "enemies"))
		{
			List* temp;
			List* list = comas_to_list(value);

			while (list)
			{
				list_add(&place->random_enemies,
					(void*) get_class_by_name(data->classes, (char*) list->data));

				temp = list;
				list = list->next;

				free(temp->data);
				free(temp);
			}

		}
		else if(!strcmp(field, "destinations") || !strcmp(field, "leads to"))
		{
			place->destinations = comas_to_list(value);
		}
		else
			fprintf(stderr, " [%s]> Unknown field: %s\n", filename, field);
	}

	free(str);

	fclose(f);

	return place;
}

/**
 * @param list: List* of Place*
 */
Place*
get_place_by_name(List* list, char* name)
{
	Place* place;

	while (list)
	{
		place = (Place*) list->data;

		if (!strcmp(place->name, name))
			return place;

		list = list->next;
	}

	return NULL;
}

/* vim: set ts=4 sw=4 cc=80 : */
