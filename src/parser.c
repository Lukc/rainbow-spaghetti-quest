#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/types.h>

#include "parser.h"
#include "skills.h"
#include "destinations.h"
#include "recipe.h"
#include "enemies.h"
#include "skills.h"
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

	if (!f)
		return NULL;

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

		list_free(list, NULL);
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
		int len;
		char* log = (char*) malloc(sizeof(char) * 128);
		len = snprintf(log, 128, "Element “%s” is no string!", element->name);
		realloc(log, len + 1);
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
		int len;
		char* log = (char*) malloc(sizeof(char) * 128);
		len = snprintf(log, 128,
			"Element “%s” element is no integer!", element->name);
		realloc(log, len + 1);
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
		drop->quantity = 1;

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
			else if (!strcmp(element->name, "quantity"))
				drop->quantity = parser_get_integer(element, logs);
			else
				fprintf(stderr, "[Drop:%i] Unknown element: %s.\n",
					element->lineno, element->name);
		}

		return drop;
	}
}

Attack*
parser_get_attack(ParserElement* element, Logs* logs)
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
				attack->damage = parser_get_integer(element, logs);
			else if (!strcmp(element->name, "strikes"))
				attack->strikes = parser_get_integer(element, logs);
			else if (!strcmp(element->name, "mana"))
				attack->mana_cost =	parser_get_integer(element, logs);
			else if (!strcmp(element->name, "name"))
				attack->name = parser_get_string(element, logs);
			else if (!strcmp(element->name, "cures"))
				list_add(&attack->cures_status_names,
					parser_get_string(element, logs));
			else if (!strcmp(element->name, "health on use"))
				attack->gives_health = parser_get_integer(element, logs);
			else if (!strcmp(element->name, "mana on use"))
				attack->mana_cost = - parser_get_integer(element, logs);
			else if (!strcmp(element->name, "inflicts"))
				attack->inflicts_status_name = parser_get_string(element, logs);
			else if (!strcmp(element->name, "self inflicts"))
				attack->self_inflicts_status_name = parser_get_string(element, logs);
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
			else
			{
				char* log = (char*) malloc(sizeof(char) * 128);
				snprintf(log, 128, "[Attack:%i] Unknown field ignored: %s.",
					element->lineno, element->name);
				logs_add(logs, log);
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
				else if (!strcmp(field, "recipe"))
				{
					if (element->type == PARSER_LIST)
						load_recipe(game, element->value);
					else
						fprintf(stderr,
							"Recipe is not a list of keys and values.\n");
				}
				else if (!strcmp(field, "skill"))
				{
					if (element->type == PARSER_LIST)
						load_skill(game, element->value);
					else
						fprintf(stderr,
							"Skill is not a list of keys and values.\n");
				}
				else
					fprintf(stderr,
						"Unknown top-level element: %s\n", field);

				parser_free(element);
			}

			list_free(parsed_file, NULL);
		}

		free(filename);
	}

	closedir(dir);
}

/**
 * Makes an Attack* usable in-game.
 *
 * Strings representing statuses are converted into pointers.
 */
static void
update_attack(Game* game, Attack* attack)
{
	List* list;

	if (attack->inflicts_status_name)
	{
		attack->inflicts_status =
			get_status_by_name(game->statuses,
				attack->inflicts_status_name);

		if (!attack->inflicts_status)
		{
			fprintf(stderr, "[Attack:%s] Inflicts unknown status: %s!\n",
				attack->name, attack->inflicts_status_name);
		}
	}

	if (attack->self_inflicts_status_name)
	{
		attack->self_inflicts_status =
			get_status_by_name(game->statuses,
				attack->self_inflicts_status_name);

		if (!attack->self_inflicts_status)
		{
			fprintf(stderr, "[Attack:%s] Self inflicts unknown status: %s!\n",
				attack->name, attack->self_inflicts_status_name);
		}
	}

	for (list = attack->cures_status_names; list; list = list->next)
	{
		char* name = list->data;
		Status* status = get_status_by_name(game->statuses, name);

		if (status)
			list_add(&attack->cures_statuses, status);
		else
			fprintf(stderr, "[Attack:%s] Cures unknown status: %s\n",
				attack->name, name);
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

			update_attack(game, attack);
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

			update_attack(game, attack);
		}
	}

	/* Places-related updates. And tons of them. */
	for (l = game->places; l; l = l->next)
	{
		Place* place = l->data;
		List* sl;

		for (sl = place->destinations; sl; sl = sl->next)
		{
			Destination* dest = sl->data;
			Place* place = get_place_by_name(game->places, dest->name);

			if (place)
			{
				List* ssl;

				dest->place = place;

				for (ssl = dest->needed_items; ssl; ssl = ssl->next)
				{
					char* name;
					Item* item;

					name = ssl->data;
					item = get_item_by_name(game->items, name);

					if (item)
					{
						ssl->data = item;
						free(name);
					}
					else
					{
						fprintf(stderr, "Unknown item: %s\n", name);
						exit(1);
					}
				}
			}
			else
			{
				fprintf(stderr, "Unknown place: %s\n", dest->name);
				exit(1);
			}
		}

		for (sl = place->skill_drops; sl; sl = sl->next)
		{
			char* name;
			SkillDrops* sd = sl->data;

			name = (char*) sd->skill;

			sd->skill = get_skill_by_name(game->skills, name);

			if (sd->skill)
			{
				List* ssl;

				for (ssl = sd->drops; ssl; ssl = ssl->next)
				{
					char* name;
					Drop* drop;
					Item* item;

					drop = ssl->data;
					name = (char*) drop->item_name;
					item = get_item_by_name(game->items, name);

					if (item)
						drop->item = item;
					else
					{
						fprintf(stderr, "[Place/%s] Undefined item: %s.\n",
							place->name, name);
						exit(1);
					}
				}
			}
			else
			{
				fprintf(stderr, "[Place/%s] Undefined skill: %s.\n",
					place->name, name);
				exit(1);
			}
		}

		for (sl = place->shop_item_names; sl; sl = sl->next)
		{
			char* name = sl->data;
			Item* item = get_item_by_name(game->items, name);

			if (item)
				list_add(&place->shop_items, item);
			else
				fprintf(stderr, "[Place:%s/Shop Items] Unknown item: %s",
					place->name, name);
		}

		for (sl = place->random_enemies; sl; sl = sl->next)
		{
			RandomEnemy* r = sl->data;
			char* name = (char*) r->class;
			Class* class = get_class_by_name(game->classes, name);

			if (class)
			{
				r->class = class;

				free(name);
			}
			else
			{
				fprintf(stderr, "[Place:%s/Enemies] Unknown class: %s",
					place->name, name);

				free(name);

				exit(0);
			}
				
		}
	}

	for (l = game->recipes; l; l = l->next)
	{
		Recipe* recipe = l->data;
		List* sl;

		recipe->output = get_item_by_name(game->items, (char*) recipe->output);

		if (!recipe->output)
		{
			fprintf(stderr,
				"[Recipe:??] Item “%s” does not exist!\n", (char*) recipe->output);

			exit(1);
		}

		if (recipe->skill)
		{
			char* name = (char*) recipe->skill;

			if (!(recipe->skill = get_skill_by_name(game->skills, name)))
			{
				fprintf(stderr, "[Recipe/%s] Undefined skill: %s.\n",
					recipe->output->name, name);
				exit(1);
			}
		}

		for (sl = recipe->ingredients; sl; sl = sl->next)
		{
			Ingredient* ig = sl->data;
			Item* item = get_item_by_name(game->items, (char*) ig->item);

			if (item)
				ig->item = item;
			else
			{
				fprintf(stderr, "[Recipe:%s] Unknown item: %s\n",
					recipe->output->name, ig->item->name);

				exit(1);
			}
		}
	}
}

void
unload_game(Game* game)
{
	list_free(game->places, free_place);
	list_free(game->items, free_item);
	/* FIXME: Not complete. */
}

/* vim: set ts=4 sw=4 cc=80 : */
