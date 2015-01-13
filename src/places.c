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
#include "destinations.h"
#include "skills.h"
#include "enemies.h"
#include "characters.h"

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

		list_add(&list, strdup(string));

		string = strtok(NULL, ",");
	}

	return list;
}

Destination*
parse_destination(List* elements, Logs* logs)
{
	ParserElement* element;
	Destination* destination;
	char* field;

	destination = (Destination*) malloc(sizeof(Destination));
	memset(destination, 0, sizeof(Destination));

	for (; elements; elements = elements->next)
	{
		element = elements->data;
		field = element->name;

		if (!strcmp(field, "name"))
		{
			destination->name = parser_get_string(element, logs);
		}
		else if (!strcmp(field, "if"))
		{
			if (element->type == PARSER_LIST)
				load_condition(&destination->condition, element->value);
			else
				fprintf(stderr, "[:%i] “if” field is not a list.\n",
					element->lineno);
		}
		else
		{
			char* log = (char*) malloc(sizeof(char) * 128);
			snprintf(log, 128, "Unknown field: “%s”.", element->name);
			logs_add(logs, log);
		}
	}

	return destination;
}

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
			Drop* drop = parser_get_drop(element, logs);

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

		if (!strcmp(field, "skill"))
			_load_skill(place, element, logs);
		else if (!strcmp(field, "name"))
			place->name = parser_get_string(element, logs);
		else if (!strcmp(field, "shop item"))
		{
			char* string = parser_get_string(element, logs);

			if (string)
			{
				temp = comas_to_list(string);

				for (helper = temp; helper; helper = helper->next)
				{
					list_add(&place->shop_item_names, helper->data);
				}
			}
		}
		else if (!strcmp(field, "leads to"))
		{
			Destination* dest;

			if (element->type == PARSER_STRING)
			{
				dest = (Destination*) malloc(sizeof(Destination));
				memset(dest, 0, sizeof(Destination));

				dest->name = parser_get_string(element, logs);

				list_add(&place->destinations, dest);
			}
			else if (element->type == PARSER_LIST)
			{
				list_add(&place->destinations,
					parse_destination(element->value, logs));
			}
		}
		else if (!strcmp(field, "random enemy"))
		{
			RandomEnemy* r;

			if (element-> type == PARSER_STRING)
			{
				char* string = parser_get_string(element, logs);

				if (string)
				{
					r = malloc(sizeof(*r));

					/* Will be converted to Class* later. */
					r->class = (Class*) string;
					r->frequency = 1;

					list_add(&place->random_enemies, r);
				}
			}
			else if (element->type == PARSER_LIST)
			{
				char* class = NULL;
				int frequency = 0;
				List* sl = element->value;

				for (; sl; sl = sl->next)
				{
					element = sl->data;
					field = element->name;

					if (!strcmp(field, "class"))
						class = parser_get_string(element, logs);
					else if (!strcmp(field, "frequency"))
						frequency = parser_get_integer(element, logs);
					else
						fprintf(stderr,
							"[Place:%s/Random Enemy:%i] Unknown field: %s\n",
							place->name ? place->name : "??",
							element->lineno, field);
				}

				if (class)
				{
					r = malloc(sizeof(*r));

					r->class = (Class*) class;
					r->frequency = frequency > 0 ? frequency : 1;

					list_add(&place->random_enemies, r);
				}
			}
			else
				fprintf(stderr,
					"[Place:%s:%i] <Random enemy> field is not a string or a list.\n",
					place->name ? place->name : "??", element->lineno);
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
			List* helper;
			List* names = comas_to_list(parser_get_string(element, logs));

			for (helper = names; helper; helper = helper->next)
			{
				char* name = helper->data;
				char* filename = (char*) malloc(42 + strlen(name));

				snprintf(filename, 42 + strlen(name), "data/images/%s", name);

				helper->data = load_image(filename);

				free(name);
			}

			place->on_first_visit = list_rev_and_free(names);
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
