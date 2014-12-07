#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/types.h>

#include "parser.h"
#include "skills.h"
#include "destinations.h"
#include "list.h"

/**
 * Opens and reads a file and returns a List* of ParserElement* representing
 * the content of the file, assuming it was a valid Spaghetti Quest file.
 */
List*
parse_file(char* filename)
{
	FILE* f = fopen(filename, "r");
	char* str = NULL;
	size_t n;
	int lineno = 0;
	int i;
	char* field;
	char* value;
	char* line;
	List* list = NULL;
	ParserElement* element;
	ParserElement* parent = NULL;

	while (getline(&str, &n, f) > 0)
	{
		lineno++;

		/* Whole line is a comment? Let’s go to the next one. */
		if (str[0] == '#')
			continue;

		/* Removing whatever’s after the first '#' encountered. */
		line = strtok(str, "#");

		field = strtok(line, ":\n");

		if (!field)
			continue;

		/* Ignoring leading whitespace? */
		for (i = 0; field[i] && isblank(field[i]); i++)
			;;
		field = field + i;

		value = strtok(NULL, ":\n");

		element = (ParserElement*) malloc(sizeof(ParserElement));
		element->parent = parent;
		element->lineno = lineno;

		if (field[0] == ']')
		{
			free(element);

			if (!parent)
				fprintf(stderr, " [%s]> Line %i: unexpected ']'...\n",
					filename, lineno);
			else
				parent = parent->parent;

			continue;
		}
		else if (!strcmp(field, ""))
		{
			free(element);

			continue;
		}
		else
		{
			/* Ignoring leading whitespace, again? */
			for (i = 0; value[i] && isblank(value[i]); i++)
				;;
			value = value + i;

			/* Same for the end of string, I guess. */
			for (i = strlen(value); i >= 0 && isblank(value[i]); i--)
				;;
			value[i+1] = '\0';

			if (isdigit(value[0]) || value[0] == '-' || value[0] == '+')
			{
				element->type = PARSER_INTEGER;
				element->value = (void*) (long) atoi(value);
			}
			else if (value[0] == '[')
			{
				element->type = PARSER_LIST;
				element->value = NULL;
			}
			else
			{
				element->type = PARSER_STRING;
				element->value = (void*) strdup(value);
			}
		}

		element->name = strdup(field);

		/* Case is ignored for keys, alright? */
		for (i = 0; element->name[i]; i++)
			element->name[i] = tolower(element->name[i]);

		if (parent)
			list_add((List**) &parent->value, element);
		else
			list_add(&list, element);

		if (element->type == PARSER_LIST)
			parent = element;
	}

	return list;
}

void
parser_free(ParserElement* element)
{
	if (element->type == PARSER_LIST)
	{
		List* list;

		for (list = element->value; list; list = list->next)
		{
			parser_free(list->data);
		}

		list_free(list);
	}
	else if (element->type == PARSER_STRING)
	{
		free(element->value);
	}

	free(element->name);
	free(element);
}

char*
parser_get_string(ParserElement* element, Logs* logs)
{
	if (element->type == PARSER_STRING)
		return strdup(element->value);
	else
	{
		char* log = (char*) malloc(sizeof(char) * 128);
		snprintf(log, 128, "Element “%s” is no string!", element->name);
		logs_add(logs, log);

		return NULL;
	}
}

int
parser_get_integer(ParserElement* element, Logs* logs)
{
	if (element->type == PARSER_INTEGER)
		return (int) element->value;
	else
	{
		char* log = (char*) malloc(sizeof(char) * 128);
		snprintf(log, 128,
			"Element “%s” element is no integer!", element->name);
		logs_add(logs, log);

		return 0;
	}
}

Drop*
parser_get_drop(ParserElement* element, Logs* logs)
{
	List* list;
	Drop* drop = (Drop*) malloc(sizeof(Drop));

	if (element->type != PARSER_LIST)
	{
		logs_add(logs,
			strdup("Drop that is not a list found.\n"));
		free(drop);

		return NULL;
	}
	else
	{
		memset(drop, 0, sizeof(Drop));

		for (list = element->value; list; list = list->next)
		{
			element = list->data;

			if (!strcmp(element->name, "item"))
			{
				/* Will be transformed into an Item* later */
				drop->item_name = parser_get_string(element, logs);
			}
			else if (!strcmp(element->name, "rarity"))
				drop->rarity = parser_get_integer(element, logs);
		}

		return drop;
	}
}

Attack*
parser_get_attack(Game* game, ParserElement* element, Logs* logs)
{
	List* list;
	Attack* attack = (Attack*) malloc(sizeof(Attack));

	if (element->type != PARSER_LIST)
	{
		logs_add(logs,
				strdup("Trying to add attack improperly defined.\n"));
		free(attack);
	}
	else
	{
		memset(attack, 0, sizeof(Attack));
		attack->inflicts_status = NULL;

		for (list = element->value; list; list = list->next)
		{
			element = list->data;

			if (!strcmp(element->name, "damage"))
				attack->damage =
					parser_get_integer(element, logs);
			else if (!strcmp(element->name, "strikes"))
				attack->strikes =
					parser_get_integer(element, logs);
			else if (!strcmp(element->name, "mana"))
				attack->mana_cost =
					parser_get_integer(element, logs);
			else if (!strcmp(element->name, "name"))
				attack->name =
					parser_get_string(element, logs);
			else if (!strcmp(element->name, "cures"))
				list_add(&attack->cures_status_names,
					parser_get_string(element, logs));
			else if (!strcmp(element->name, "inflicts"))
			{
				char* string = parser_get_string(element, logs);

				attack->inflicts_status =
					get_status_by_name(game->statuses, string);

				if (!attack->inflicts_status)
				{
					fprintf(stderr, "[Attack:%s] Unknown status: %s!\n",
						attack->name, string);

					exit(1);
				}
			}
			else if (!strcmp(element->name, "type"))
			{
				char* type = parser_get_string(element, logs);

				if (type)
				{
					attack->type = string_to_type(type);

					if (attack->type == -1)
					{
						char* log = (char*) malloc(sizeof(char) * 128);
						snprintf(log, 128, "Invalid type: “%s”.", type);
						logs_add(logs, log);

						free_attack(attack);

						return NULL;
					}
				}
			}
		}
	}

	return attack;
}

void
import_dir(Game* game, char* dirname)
{
	DIR* dir;
	struct dirent* entry;
	char* filename;
	List* parsed_file;
	List* l;

	dir = opendir(dirname);

	while ((entry = readdir(dir)))
	{
		if (entry->d_name[0] == '.')
			continue;

		filename = (char*) malloc(sizeof(char) *
			(strlen(dirname) + strlen(entry->d_name) + 2));
		sprintf(filename, "%s/%s", dirname, entry->d_name);

		/* FIXME: Portability issue here. */
		if (entry->d_type == DT_DIR)
		{
			import_dir(game, filename);
		}
		else
		{
			if (strcmp(entry->d_name + strlen(entry->d_name) - 4, ".txt"))
				continue;

			printf(" > Loading %s\n", filename);

			parsed_file = parse_file(filename);

			for (l = parsed_file; l; l = l->next)
			{
				ParserElement* element = l->data;
				char* field = element->name;

				if (!strcmp(field, "status"))
				{
					if (element->type == PARSER_LIST)
						load_status(game, element->value);
					else
						fprintf(stderr,
							"Status is not a list of keys and values.\n");
				}
				else if (!strcmp(field, "item"))
				{
					if (element->type == PARSER_LIST)
						load_item(game, element->value);
					else
						fprintf(stderr,
							"Item is not a list of keys and values.\n");
				}
				else if (!strcmp(field, "class"))
				{
					if (element->type == PARSER_LIST)
						load_class(game, element->value);
					else
						fprintf(stderr,
							"Item is not a list of keys and values.\n");
				}
				else if (!strcmp(field, "place"))
				{
					if (element->type == PARSER_LIST)
						load_place(game, element->value);
					else
						fprintf(stderr,
							"Item is not a list of keys and values.\n");
				}
				else
					fprintf(stderr,
						"Unknown top-level element: %s\n", field);

				parser_free(element);
			}

			list_free(parsed_file);
		}

		free(filename);
	}
}

/**
 * Loads the game’s data in two stages.
 *
 * The first stage is about reading files and storing their data in the
 * corresponding structures used internally.
 *
 * The second stage is about replacing references (char*) by pointers
 * to the corresponding structures (Entity*, Place*, Item*, …) in various
 * places.
 * This takes place mostly in lists, where no type checking is done at
 * build-time. Be very careful when adding that kind of stuff!
 */
void
load_game(Game* game, char* dirname)
{
	List* l;

	/* Phase 1. Separate because (not so) very highly recursive. */
	import_dir(game, dirname);

	/* Phase 2 */
	/* FIXME: Put somewhere else? */

	/* Items-related updates. */
	for (l = game->classes; l; l = l->next)
	{
		Class* class = l->data;
		List* l2;

		for (l2 = class->drop; l2; l2 = l2->next)
		{
			Drop* drop = l2->data;

			drop->item = get_item_by_name(game->items, drop->item_name);

			if (!drop->item)
			{
				fprintf(stderr,
					"Item “%s” does not exist!\n", drop->item_name);

				exit(0);
			}

			free(drop->item_name);
		}
	}

	/* Attacks-related updates. */
	for (l = game->classes; l; l = l->next)
	{
		Class* class = l->data;
		List* sl;

		for (sl = class->attacks; sl; sl = sl->next)
		{
			Attack* attack = sl->data;
			List* ssl; /* “sub-sub-list”? This is getting crazy. */

			for (ssl = attack->cures_status_names; ssl; ssl = ssl->next)
			{
				char* name = ssl->data;
				Status* status = get_status_by_name(game->statuses, name);

				if (status)
					list_add(&attack->cures_statuses, status);
				else
					fprintf(stderr, "Unknown status: %s\n", name);
			}
		}
	}

	/* Exactly the same code as above, but the Classes are being replaced
	 * by Items… */
	for (l = game->items; l; l = l->next)
	{
		Item* item = l->data;
		List* sl;

		for (sl = item->attacks; sl; sl = sl->next)
		{
			Attack* attack = sl->data;
			List* ssl; /* “sub-sub-list”? This is getting crazy. */

			for (ssl = attack->cures_status_names; ssl; ssl = ssl->next)
			{
				char* name = ssl->data;
				Status* status = get_status_by_name(game->statuses, name);

				if (status)
					list_add(&attack->cures_statuses, status);
				else
					fprintf(stderr, "Unknown status: %s\n", name);
			}
		}
	}

	/* Places-related updates. And tons of them. */
	for (l = game->places; l; l = l->next)
	{
		int i;
		Place* place = l->data;
		List* sl;
		
		for (sl = place->random_enemy_names; sl; sl = sl->next)
		{
			Class* class = get_class_by_name(game->classes, sl->data);

			if (class)
				list_add(&place->random_enemies, class);
			else
				fprintf(stderr, "Unknown enemy: %s\n", sl->data);
		}

		for (sl = place->destinations; sl; sl = sl->next)
		{
			Destination* dest = sl->data;
			Place* place = get_place_by_name(game->places, dest->name);

			if (place)
				dest->place = place;
			else
			{
				fprintf(stderr, "Unknown place: %s\n", dest->name);
				exit(1);
			}
		}

		for (i = 0; i < SKILL_MAX; i++)
		{
			for (sl = place->skill_drop[i]; sl; sl = sl->next)
			{
				Drop* drop = sl->data;

				drop->item = get_item_by_name(game->items, drop->item_name);

				if (!drop->item)
				{
					fprintf(stderr,
						"Item “%s” does not exist!\n", drop->item_name);

					exit(0);
				}
			}
		}

		for (sl = place->shop_item_names; sl; sl = sl->next)
		{
			char* name = sl->data;
			Item* item = get_item_by_name(game->items, name);

			if (item)
				list_add(&place->shop_items, item);
			else
				fprintf(stderr, "Unknown item: %s", name);
		}
	}
}

/* vim: set ts=4 sw=4 cc=80 : */
