#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>

#include "classes.h"
#include "parser.h"

/**
 * @return: List* of Class*
 */
List*
load_classes(char* dirname)
{
	List* classes = NULL;
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
		list_add(&classes, load_class(filename));

		free(filename);
	}

	closedir(dir);

	return classes;
}

static int
check_type_resistance(Class* class, ParserElement* element, Logs* logs)
{
	char* type;
	int i;
	size_t len;

	for (i = 0; i < TYPE_MAX; i++)
	{
		type = type_string(i);

		len = strlen(type);

		if (
			!strncmp(element->name, type, len) &&
			element->name[len] == ' ' &&
			!strcmp(element->name + len + 1, "defense"))
		{
			class->type_resistance[i] = parser_get_integer(element, logs);

			return 1;
		}
	}

	return 0;
}

static int
get_type(char* string, Logs* logs)
{
	char* type;
	char* log;
	int i;

	for (i = 0; i < TYPE_MAX; i++)
	{
		type = type_string(i);

		if (!strcmp(string, type))
			return i;
	}

	log = (char*) malloc(sizeof(char) * 128);
	snprintf(log, 128, "Invalid type: “%s”.", string);

	logs_add(logs, log);

	return 0;
}

Class*
load_class (char* filename)
{
	List* list = load_file(filename);
	List* temp;
	ParserElement* element;
	Class* class;
	Logs* logs;

	class = (Class*) malloc(sizeof(Class));

	memset(class, 0, sizeof(Class));

	logs = logs_new();

	while (list)
	{
		char* field;

		element = list->data;

		field = element->name;

		if (!strcmp(field, "name"))
			class->name = parser_get_string(element, logs);
		else if (!strcmp(field, "mana"))
			class->base_mana = parser_get_integer(element, logs);
		else if (!strcmp(field, "health"))
			class->base_health = parser_get_integer(element, logs);
		else if (!strcmp(field, "attack bonus"))
			class->attack_bonus = parser_get_integer(element, logs);
		else if (!strcmp(field, "defense bonus"))
			class->defense_bonus = parser_get_integer(element, logs);
		else if (!strcmp(field, "caps on kill"))
			class->caps_on_kill = parser_get_integer(element, logs);
		else if (!strcmp(field, "attack"))
		{
			List* sublist;
			ParserElement* subelement;
			Attack* attack = (Attack*) malloc(sizeof(Attack));

			if (element->type != PARSER_LIST)
			{
				logs_add(logs,
					strdup("Trying to add attack improperly defined.\n"));
				free(attack);
			}
			else
				for (sublist = element->value; sublist; sublist = sublist->next)
				{
					subelement = sublist->data;

					if (!strcmp(subelement->name, "damage"))
						attack->damage =
							parser_get_integer(subelement, logs);
					else if (!strcmp(subelement->name, "strikes"))
						attack->strikes =
							parser_get_integer(subelement, logs);
					else if (!strcmp(subelement->name, "type"))
					{
						char* type = parser_get_string(subelement, logs);

						if (type)
							attack->type = get_type(type, logs);
					}
				}

			list_add(&class->attacks, (void*) attack);
		}
		else if (check_type_resistance(class, element, logs))
			;
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

	return class;
}

/**
 * @param list: List* of Class*
 */
Class*
get_class_by_name(List* list, char* name)
{
	Class* class;

	while (list)
	{
		class = (Class*) list->data;

		if (!strcmp(class->name, name))
			return class;

		list = list->next;
	}

	return NULL;
}

/* vim: set ts=4 sw=4 cc=80 : */
