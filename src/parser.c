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

#include "parser/class.h"
#include "parser/item.h"
#include "parser/recipe.h"
#include "parser/place.h"
#include "parser/event.h"

/**
 * Opens and reads a file and returns a List* of ParserElement* representing
 * the content of the file, assuming it was a valid Spaghetti Quest file.
 *
 * Try not to mess with this. That code hates everyone.
 */
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
#	define IDENTIFIER 0
#	define VALUE 1

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

#	undef IDENTIFIER
#	undef VALUE

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
			fprintf(stderr, "<%s:%i> %s: String expected.\n",
				element->filename, element->lineno, element->name);

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
			fprintf(stderr, "<%s:%i> %s: Integer expected.\n",
				element->filename, element->lineno, element->name);

		return 0; /* Great… what if the value’s a zero? … */
	}
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
							"Class is not a list of keys and values.\n");
				}
				else if (!strcmp(field, "place"))
				{
					if (element->type == PARSER_LIST)
						load_place(game, element->value);
					else
						fprintf(stderr,
							"Place is not a list of keys and values.\n");
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
	/* Updating game structures’ content and making sure that content is
	 * consistent. */

	/* Items-related updates. */
	for (l = game->classes; l; l = l->next)
		parser_load_class(game, (Class*) l->data);

	for (l = game->items; l; l = l->next)
		parser_load_item(game, (Item*) l->data);

	for (l = game->places; l; l = l->next)
		parser_load_place(game, (Place*) l->data);

	for (l = game->recipes; l; l = l->next)
		parser_load_recipe(game, (Recipe*) l->data);

	/* Most important thing of any RPG. \o/ */
	for (l = game->events; l; l = l->next)
		parser_load_event(game, (Event*) l->data);
}

void
unload_game(Game* game)
{
	list_free(game->places, free_place);
	list_free(game->items, free_item);
	/* FIXME: Not complete. */
}

/* vim: set ts=4 sw=4 cc=80 : */
