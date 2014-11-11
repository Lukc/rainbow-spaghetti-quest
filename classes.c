#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>

#include "classes.h"

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
check_type_resistance(Class* class, char* field, char* value)
{
	char* type;
	int i;
	size_t len;

	for (i = 0; i < TYPE_MAX; i++)
	{
		type = type_string(i);

		len = strlen(type);

		if (
			!strncmp(field, type, len) &&
			field[len] == ' ' &&
			!strcmp(field + len + 1, "defense"))
		{
			class->type_resistance[i] = atoi(value);

			return 1;
		}
	}

	return 0;
}

Class*
load_class (char* filename)
{
	FILE* f = fopen(filename, "r");
	char* str = NULL;
	size_t n = 0;
	Class* class;
	int i;

	class = (Class*) malloc(sizeof(Class));

	memset(class, 0, sizeof(Class));

	while (getline(&str, &n, f) > 0)
	{
		char* line;
		char* field;
		char* value;

		if (str[0] != '#')
			line = strtok(str, "#");
		else
			continue;

		field = strtok(line, ":");
		value = strtok(NULL, ":");

		while (value[0] && value[0] == ' ')
			value++;

		value[strlen(value)-1] = '\0';

		for (i = 0; field[i]; i++)
			field[i] = tolower(field[i]);

		if (!strcmp(field, "name"))
			class->name = strdup(value);
		else if (!strcmp(field, "id"))
			class->id = atoi(value);
		else if (!strcmp(field, "health"))
			class->base_health = atoi(value);
		else if (!strcmp(field, "mana"))
			class->base_mana = atoi(value);
		else if (!strcmp(field, "attack"))
		{
			int damage, strikes, i;

			if (sscanf(value, "%d-%d ", &damage, &strikes) < 2)
			{
				fprintf(stderr, " [%s]> Attack could not be parsed: %s.\n", filename, value);
				getchar();
			}

			for (i = 0; value[i] && value[i] != ' '; i++)
				;;

			for (; value[i] && value[i] == ' '; i++)
				;;

			if (value[i])
			{
				Attack* attack = &class->default_attack;

				attack->damage = damage;
				attack->strikes = strikes;

				/* Hoping there’s no padding at the end. */
				attack->type = type_id(value + i);
			}
			else
			{
				fprintf(stderr, " [%s]> No attack type spectified in attack field!\n", filename);
				getchar();
			}
		}
		else if (!strcmp(field, "attack bonus"))
			class->attack_bonus = atoi(value);
		else if (!strcmp(field, "defense bonus"))
			class->defense_bonus = atoi(value);
		else if (!strcmp(field, "caps") || !strcmp(field, "caps on kill"))
			class->caps_on_kill = atoi(value);
		else if (check_type_resistance(class, field, value)) ;
		else
			fprintf(stderr, " [%s]> Unknown field: %s\n", filename, field);
	}

	free(str);

	fclose(f);

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
