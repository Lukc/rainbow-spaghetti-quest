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


/* vim: set ts=4 sw=4 cc=80 : */
