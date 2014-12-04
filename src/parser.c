#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "parser.h"
#include "list.h"

List*
load_file(char* filename)
{
	FILE* f = fopen(filename, "r");
	char* str = NULL;
	size_t n;
	int i;
	char* field;
	char* value;
	char* line;
	List* list = NULL;
	ParserElement* element;
	ParserElement* parent = NULL;

	while (getline(&str, &n, f) > 0)
	{
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

		if (field[0] == ']')
		{
			free(element);

			if (!parent)
				fprintf(stderr, " [%s]> Syntax error. Unexpected ]...\n",
					filename);
			else
				parent = parent->parent;

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
parser_get_drop(List* items, ParserElement* element, Logs* logs)
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
				char* name = parser_get_string(element, logs);
				drop->item = get_item_by_name(items, name);
			}
			else if (!strcmp(element->name, "rarity"))
				drop->rarity = parser_get_integer(element, logs);
		}

		if (!drop->item)
		{
			char* log = (char*) malloc(sizeof(char) * 128);
			snprintf(log, 128, "Drop item with no valid “item” field!");
			logs_add(logs, log);

			free(drop);

			return NULL;
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

						/* FIXME: free_attack()? */
						if (attack->name)
							free(attack->name);
						free(attack);

						return NULL;
					}
				}
			}
		}
	}

	return attack;
}


/* vim: set ts=4 sw=4 cc=80 : */
