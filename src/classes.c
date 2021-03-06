#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>

#include "classes.h"
#include "parser.h"

#include "parser/drop.h"
#include "parser/attack.h"

#include "events.h"

static int
check_type_resistance(Class* class, ParserElement* element, Logs* logs)
{
	char* type;
	int i;
	size_t len;

	for (i = 0; i < TYPE_MAX; i++)
	{
		type = type_to_string(i);

		len = strlen(type);

		if (
			!strncmp(element->name, type, len) &&
			element->name[len] == ' ' && (
				!strcmp(element->name + len + 1, "defense") ||
				!strcmp(element->name + len + 1, "resistance")
			))
		{
			class->type_resistance[i] = parser_get_integer(element, logs);

			return 1;
		}
	}

	return 0;
}

void
load_class(Game* game, List* list)
{
	ParserElement* element;
	Class* class;
	Logs* logs;

	class = (Class*) malloc(sizeof(Class));

	memset(class, 0, sizeof(Class));

	logs = logs_new();

	for (; list; list = list->next)
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
		else if (!strcmp(field, "mana on focus"))
			class->mana_regen_on_focus = parser_get_integer(element, logs);
		else if (!strcmp(field, "health on focus"))
			class->health_regen_on_focus = parser_get_integer(element, logs);
		else if (!strcmp(field, "attack bonus"))
			class->attack_bonus = parser_get_integer(element, logs);
		else if (!strcmp(field, "defense bonus"))
			class->defense_bonus = parser_get_integer(element, logs);
		else if (!strcmp(field, "gold on kill"))
			class->gold_on_kill = parser_get_integer(element, logs);
		else if (!strcmp(field, "attack"))
		{
			Attack* attack = parser_get_attack(element);

			if (attack)
				list_add(&class->attacks, (void*) attack);
		}
		else if (!strcmp(field, "drop"))
		{
			Drop* drop = parser_get_drop(element);

			if (drop)
				list_add(&class->drop, drop);
		}
		else if (check_type_resistance(class, element, logs))
			;
		else if (!strcmp(field, "start turn events"))
		{
			if (element->type == PARSER_LIST)
				load_events(game, &class->start_turn_events, element->value);
			else
				fprintf(stderr, "<%s:%i> “Start Turn Events” is not a list.\n",
					element->filename, element->lineno);
		}
		else
		{
			char* log = (char*) malloc(sizeof(char) * 128);
			snprintf(log, 128, "Unknown field: “%s”.", element->name);
			logs_add(logs, log);
		}
	}

	if (logs->head)
	{
		/* FIXME: stderr */
		logs_print(logs);
		logs_free(logs);
	}

	list_add(&game->classes, class);
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
