#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>

#include "string.h"
#include "images.h"
#include "places.h"
#include "items.h"
#include "classes.h"
#include "parser.h"
#include "destinations.h"
#include "skills.h"
#include "enemies.h"
#include "characters.h"

#include "parser/drop.h"
#include "parser/destination.h"
#include "parser/spawn_data.h"

static int
_load_skill(Place* place, ParserElement* element, Logs* logs)
{
	char* name;
	List* list;
	List* drops = NULL;

	for (list = element->value; list; list = list->next)
	{
		element = list->data;

		if (!strcmp(element->name, "name"))
			name = parser_get_string(element, logs);
		else if (!strcmp(element->name, "drop"))
		{
			Drop* drop = parser_get_drop(element);

			if (drop)
				list_add(&drops, drop);
		}
		else
			fprintf(stderr,
				"[Place:Skill:%i] Unrecognized element ignored: %s.\n",
				element->lineno, element->name);
	}

	if (name)
	{
		SkillDrops* sd = malloc(sizeof(*sd));

		sd->skill = (Skill*) name;
		sd->drops = drops;

		list_add(&place->skill_drops, sd);
	}
	else
	{
		for (list = drops; list; list = list->next)
			free(list->data);

		list_free(drops, NULL);
	}

	return 0;
}

void
load_place (Game* game, List* elements)
{
	List* list = elements;
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

		if (!strcmp(field, "skill"))
			_load_skill(place, element, logs);
		else if (!strcmp(field, "name"))
			place->name = parser_get_string(element, logs);
		else if (!strcmp(field, "shop item"))
		{
			char* string = parser_get_string(element, logs);

			if (string)
				list_add(&place->shop_item_names, string);
		}
		else if (!strcmp(field, "leads to"))
		{
			Destination* dest = parser_get_destination(element);

			if (dest)
				list_add(&place->destinations, dest);
		}
		else if (!strcmp(field, "random enemy"))
		{
			SpawnData* spawn = parser_get_spawn_data(element);

			if (spawn)
				list_add(&place->random_enemies, spawn);
		}
		else if (!strcmp(field, "image"))
		{
			char* name = parser_get_string(element, logs);

			if (name)
			{
				char* filename = (char*) malloc(
					strlen("data/images/") + strlen(name) + 1);

				sprintf(filename, "data/images/%s", name);

				place->image = load_image(filename);
			}
		}
		else if (!strcmp(field, "on first visit"))
		{
			/* FIXME: We want events, here! */
			char* name = parser_get_string(element, logs);
			char* filename = (char*) malloc(42 + strlen(name));

			snprintf(filename, 42 + strlen(name), "data/images/%s", name);

			list_add(&place->on_first_visit, load_image(filename));

			free(name);
		}
		else if (!strcmp(field, "character"))
		{
			Character* c = load_character(game, element);

			if (c)
				list_add(&place->characters, c);
		}
		else
		{
			char* log = (char*) malloc(sizeof(char) * 128);
			snprintf(log, 128, "[Place/%s] Unknown field: “%s”.",
				place->name ? place->name : "??", element->name);
			logs_add(logs, log);
		}

		list = list->next;
	}

	if (logs->head)
	{
		/* FIXME: stderr */
		logs_print(logs);
		logs_free(logs);

		exit(1);
	}

	place->shop_item_names = list_rev_and_free(place->shop_item_names);

	list_add(&game->places, place);
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

void
free_place(void* ptr)
{
	Place* p = ptr;

	free(p->name);
	free(p);
}

/* vim: set ts=4 sw=4 cc=80 : */
