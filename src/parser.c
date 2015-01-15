#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "parser.h"
#include "skills.h"
#include "destinations.h"
#include "recipe.h"
#include "enemies.h"
#include "skills.h"
#include "characters.h"
#include "list.h"

#include "events/condition.h"
#include "events/items.h"

/**
 * Opens and reads a file and returns a List* of ParserElement* representing
 * the content of the file, assuming it was a valid Spaghetti Quest file.
 *
 * Try not to mess with this. That code hates everyone.
 */
#define IDENTIFIER 0
#define VALUE 1
static int
ignore_comments(int c, FILE* f)
{
	if (c == '#')
	{
		/* Comment? Let’s ignore everything after it. */
		while ((c = getc(f)) > 0 && c != '\n')
			;
	}

	return c;
}

static List*
parser_helper(FILE* f, char* filename, int* lineno, int has_parent)
{
	List* list = NULL;
	ParserElement* element;
	int expecting = IDENTIFIER;
	char buffer[4096];
	int buffer_index = 0;
	int c;

	while ((c = getc(f)) > 0)
	{
		if (c == '\n')
			(*lineno)++;

		c = ignore_comments(c, f);

		if (expecting == IDENTIFIER)
		{
			if (c == ']')
			{
				if (has_parent)
					return list;
				else
				{
					fprintf(stderr,
						"<%s:%i> Syntax error: unexpected character ']'\n",
						filename, *lineno);
				}
			}
			else if (isalnum(c))
			{
				buffer_index = 0;
				buffer[buffer_index++] = c;

				while ((c = getc(f)) && (isalnum(c) || c == ' '))
				{
					buffer[buffer_index++] = c;
				}

				c = ignore_comments(c, f);

				if (c == '\n')
					(*lineno)++;

				if (!isalnum(c) && c != ' ' && c != ':')
				{
					printf(" <<<%c>>>\n", c);
				}

				buffer[buffer_index] = '\0';

				element = malloc(sizeof(*element));
				element->lineno = *lineno;
				element->filename = strdup(filename);
				element->name = strdup(buffer);
				for (int i = 0; element->name[i]; i++)
					element->name[i] = tolower(element->name[i]);

				expecting = VALUE;
			}
			else if (c == '@')
			{
				char* instruction;

				buffer_index = 0;
				while ((c = buffer[buffer_index++] = getc(f)) > 0)
					if (c == '\n')
					{
						(*lineno)++;
						buffer[buffer_index] = '\0';
						break;
					}

				instruction = strtok(buffer, " ");
				if (!strcmp(instruction, "include"))
				{
					char* relative_filename;
					char* complete_filename;
					int i;
					
					relative_filename = strtok(NULL, " \n");

					if (relative_filename)
					{
						List* included;
						List* last;

						/* Getting the relative path first. */
						for (i = strlen(filename); i >= 0 && filename[i] != '/'; i--)
							;

						if (i < 0)
							complete_filename = relative_filename;
						else
						{
							i += 1;

							complete_filename =
								malloc(i + strlen(relative_filename) + 1);

							strncpy(complete_filename, filename, i);
							strcpy(complete_filename + i, relative_filename);
						}

						included = parse_file(complete_filename);
						if (included)
						{
							for (last = included; last->next;
								 last = last->next)
								;
							last->next = list;
							list = included;
						}
						else
							; /* Empty include? Oh, well, whatever. */
					}
					else
					{
						fprintf(stderr, "<%s:%i> Syntax error: "
							"expected filename in @include directive.\n",
							filename, *lineno);
						exit(1);
					}
				}
				else
				{
					fprintf(stderr,
						"<%s:%i> Unknown preprocessor directive: %s.\n",
						filename, *lineno, instruction);
				}

				buffer_index = 0;
			}
			else if (!isblank(c) && c != '\n')
			{
				fprintf(stderr, "<%s:%i> Syntax error: character expected.\n", filename, *lineno);
				exit(1);
			}
		}
		else /* if (expecting == VALUE) */
		{
			c = ignore_comments(c, f);

			if (isblank(c) || c == '\n')
				;
			else if (c == '[')
			{
				/* Entering sub-list. */
				element->type = PARSER_LIST;
				element->value = parser_helper(f, filename, lineno, 1);
				list_add(&list, element);
				expecting = IDENTIFIER;
			}
			else if (isdigit(c) || c == '+' || c == '-')
			{
				element->type = PARSER_INTEGER;
				buffer_index = 0;
				buffer[buffer_index++] = c;

				while ((c = getc(f)) && isdigit(c))
					buffer[buffer_index++] = c;

				buffer[buffer_index] = '\0';

				while (isblank(c))
					c = getc(f);

				c = ignore_comments(c, f);

				if (c == '\n')
					(*lineno)++;
				else
				{
					fprintf(stderr, "<%s:%i> Syntax error: "
						"unexpected character '%c'\n",
						filename, *lineno, c);
					exit(1);
				}

				element->value = (void*) atol(buffer);
				list_add(&list, element);

				expecting = IDENTIFIER;
			}
			else if (isalnum(c))
			{
				int i;

				buffer_index = 0;
				element->type = PARSER_STRING;
				buffer[buffer_index++] = c;

				while ((c = getc(f)) && c != '\n')
					buffer[buffer_index++] = c;


				c = ignore_comments(c, f);
				if (c == '\n')
					(*lineno)++;

				buffer[buffer_index] = '\0';

				/* Trimming buffer content. */
				for (i = strlen(buffer); i > 0 && isblank(buffer[i]); i--)
					buffer[i] = '\0';

				element->value = strdup(buffer);
				list_add(&list, element);

				expecting = IDENTIFIER;
			}
			else
			{
				printf(" %i <%s>\n", has_parent, buffer);
				fprintf(stderr, "<%s:%i> Syntax error: "
					"unexpected character '%c'.\n",
					filename, *lineno, c);
				exit(1);
			}
		}
	}

	return list;
}

List*
parse_file(char* filename)
{
	FILE* f;
	int lineno = 1;

	if (!(f = fopen(filename, "r")))
		return NULL;

	return parser_helper(f, filename, &lineno, 0);
}
#undef IDENTIFIER
#undef VALUE

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

	free(element->filename);
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
		if (logs)
		{
			int len;
			char* log = (char*) malloc(sizeof(char) * 128);
			len = snprintf(log, 128, "Element “%s” is no string!", element->name);
			realloc(log, len + 1);
			logs_add(logs, log);
		}
		else
			fprintf(stderr, "[:%i] %s: String expected.\n",
				element->lineno, element->name);

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
		if (logs)
		{
			int len;
			char* log = (char*) malloc(sizeof(char) * 128);
			len = snprintf(log, 128,
				"Element “%s” element is no integer!", element->name);
			realloc(log, len + 1);
			logs_add(logs, log);
		}
		else
			fprintf(stderr, "[:%i] %s: Integer expected.\n",
				element->lineno, element->name);

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
			else if (!strcmp(element->name, "name"))
				attack->name = parser_get_string(element, logs);
			else if (!strcmp(element->name, "cures"))
				list_add(&attack->cures_status_names,
					parser_get_string(element, logs));
			else if (!strcmp(element->name, "health"))
				attack->health = parser_get_integer(element, logs);
			else if (!strcmp(element->name, "mana"))
				attack->mana = parser_get_integer(element, logs);
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
	struct stat s;
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

		stat(filename, &s);
		if (S_ISDIR(s.st_mode))
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
		List* next;
		List** prev;

		prev = &place->destinations;
		for (sl = place->destinations; sl; sl = next)
		{
			Destination* dest;
			Place* place;

			next = sl->next;

			dest = sl->data;
			if (!strcmp(dest->name, ((Place*) l->data)->name))
			{
				*prev = sl->next;

				free(dest->name);
				free(dest);

				continue;
			}

			place = get_place_by_name(game->places, dest->name);

			if (place)
			{
				List* ssl;

				dest->place = place;

				for (ssl = dest->condition.items; ssl; ssl = ssl->next)
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

			prev = &sl->next;
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
				fprintf(stderr, "[Place:%s/Shop Items] Unknown item: %s.\n",
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
				fprintf(stderr, "[Place:%s/Enemies] Unknown class: %s.\n",
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
					recipe->output->name, (char*) ig->item);

				exit(1);
			}
		}
	}

	/* Most important thing of any RPG. \o/ */
	for (l = game->events; l; l = l->next)
	{
		Event* event = l->data;

		if (event->type == EVENT_CONDITION)
		{
			ConditionEvent* e = (ConditionEvent*) event;
			List* sl;
			ItemStack* stack;

			for (sl = e->condition.items; sl; sl = sl->next)
			{
				char* name;

				stack = sl->data;
				name = (char*) stack->item;

				stack->item = get_item_by_name(game->items, name);

				if (!stack->item)
				{
					fprintf(stderr, "Non-existent item: %s\n", name);

					exit(1);
				}
			}
		}
		else if (event->type == EVENT_GIVE_ITEM ||
		         event->type == EVENT_REMOVE_ITEM)
		{
			GiveItemEvent* e = (GiveItemEvent*) event;
			char* name = (char*) e->item;

			e->item = get_item_by_name(game->items, name);

			if (!e->item)
			{
				fprintf(stderr, "Non-existent item: %s\n", name);

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
