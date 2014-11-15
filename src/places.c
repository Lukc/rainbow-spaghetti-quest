#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>

#include "images.h"
#include "places.h"
#include "items.h"
#include "classes.h"
#include "parser.h"

/**
 * @return: List* of Place*
 */
List*
load_places(Game* game, char* dirname)
{
	List* places = NULL;
	List* list, * list2;
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
		list_add(&places, load_place(game, filename));

		free(filename);
	}

	for (list = places; list; list = list->next)
	{
		Place* place = list->data;

		list2 = place->destinations;
		while (list2)
		{
			char* name = list2->data;

			list2->data = get_place_by_name(places, name);

			if (!list2->data)
			{
				fprintf(stderr,
					"[Places/%s] invalid destination: %s.\n",
					place->name, name);

				exit(1);
			}

			list2 = list2->next;
		}
	}

	closedir(dir);

	return places;
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
load_place (Game* game, char* filename)
{
	List* list = load_file(filename);
	List* temp;
	List* helper;
	ParserElement* element;
	Place* place;
	Logs* logs;

	place = (Place*) malloc(sizeof(Place));

	memset(place, 0, sizeof(Place));

	logs = logs_new();

	while (list)
	{
		char* field;

		element = list->data;

		field = element->name;

		if (!strcmp(field, "name"))
			place->name = parser_get_string(element, logs);
		else if (!strcmp(field, "shop item"))
		{
			Item* item;
			char* string = parser_get_string(element, logs);

			if (string)
			{
				temp = comas_to_list(string);

				for (helper = temp; helper; helper = helper->next)
				{
					item =
						get_item_by_name(game->items, (char*) helper->data);

					if (item)
						list_add(&place->shop_items, item);
				}
			}
		}
		else if (!strcmp(field, "leads to"))
			list_add(&place->destinations,
				parser_get_string(element, logs));
		else if (!strcmp(field, "random enemies"))
		{
			char* string = parser_get_string(element, logs);

			if (string)
			{
				Class* class;

				temp = comas_to_list(string);

				for (helper = temp; helper; helper = helper->next)
				{
					class = get_class_by_name(game->classes, helper->data);

					if (class)
						list_add(&place->random_enemies, class);
				}
			}
		}
		else if (!strcmp(field, "image"))
		{
			char* name = parser_get_string(element, logs);

			if (name)
			{
				/* Note: Too lazy to put the exact value needed.
				 *       Besides, it’s bound to change. */
				char* filename = (char*) malloc(42 + strlen(name));

				snprintf(filename, 42 + strlen(name), "images/%s", name);

				place->image = load_image(filename);
			}
		}
		else if (!strcmp(field, "on first visit"))
		{
			List* helper;
			List* names = comas_to_list(parser_get_string(element, logs));

			for (helper = names; helper; helper = helper->next)
			{
				char* name = helper->data;
				char* filename = (char*) malloc(42 + strlen(name));

				snprintf(filename, 42 + strlen(name), "images/%s", name);

				helper->data = load_image(filename);

				free(name);
			}

			place->on_first_visit = list_rev_and_free(names);
		}
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

int
has_visited(Game* game, Place* place)
{
	List* list;

	for (list = game->visited; list; list = list->next)
	{
		if (list->data == place)
			return 1;
	}

	return 0;
}

/* vim: set ts=4 sw=4 cc=80 : */
