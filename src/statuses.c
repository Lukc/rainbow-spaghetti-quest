#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "string.h"
#include "parser.h"
#include "statuses.h"

void
load_status(Game* game, List* elements)
{
	List* list;
	Status* status;

	status = (Status*) malloc(sizeof(Status));
	memset(status, 0, sizeof(Status));

	for (list = elements; list; list = list->next)
	{
		char* field;
		ParserElement* element;

		element = list->data;
		field = element->name;

		if (!strcmp(field, "name"))
			status->name = parser_get_string(element, NULL);
		else if (!strcmp(field, "affliction name"))
			status->affliction_name = parser_get_string(element, NULL);
		/* FIXME: Those values should be booleans, not integers. */
		else if (!strcmp(field, "divides attack"))
			status->divides_attack = parser_get_integer(element, NULL);
		else if (!strcmp(field, "divides defense"))
			status->divides_defense = parser_get_integer(element, NULL);
		else if (!strcmp(field, "increases mana costs"))
			status->increases_mana_costs = parser_get_integer(element, NULL);
		else if (!strcmp(field, "reduces physical strikes"))
			status->reduces_physical_strikes = parser_get_integer(element, NULL);
		else if (!strcmp(field, "reduces magical strikes"))
			status->reduces_magical_strikes = parser_get_integer(element, NULL);
		else if (!strcmp(field, "prevents recovery"))
			status->prevents_recovery = parser_get_integer(element, NULL);
		else if (!strcmp(field, "removed on focus"))
			status->removed_on_focus = parser_get_integer(element, NULL);
		else if (!strcmp(field, "removes health"))
			status->removes_health = parser_get_integer(element, NULL);
		else
		{
			fprintf(stderr,
				"[Status:%s:%i] Unknown field “%s”.\n",
				status->name ? status->name : "??", element->lineno, field);
		}
	}

	list_add(&game->statuses, status);
}

int
inflict_status(Entity* e, Status* status)
{
	List* l;
	StatusData* data;

	for (l = e->statuses; l; l = l->next)
	{
		data = l->data;

		if (data->status == status)
			/* Already inflicted? Kewl. */
			return 0;
	}

	/* No similar status found in e->statuses? Let’s add one! */
	data = (StatusData*) malloc(sizeof(StatusData));

	data->strength = 1;
	data->status = status;

	list_add(&e->statuses, data);

	return 1;
}

void
cure_status(Entity* e, Status* status)
{
	List* list;
	List* prev = NULL;

	for (list = e->statuses; list; list = list->next)
	{
		StatusData* data = list->data;

		if (data->status == status)
		{
			if (prev)
				prev->next = list->next;
			else
				e->statuses = list->next;

			free(data);
			free(list);

			return;
		}
		else
			prev = list;
	}
}

int
has_status(Entity* e, Status* status)
{
	List* list;
	StatusData* data;

	for (list = e->statuses; list; list = list->next)
	{
		data = list->data;

		if (data->status == status)
			return 1;
	}

	return 0;
}

Status*
get_status_by_name(List* list, char* name)
{
	Status* status;

	for (; list; list = list->next)
	{
		status = list->data;

		if (!strcasecmp(status->name, name))
			return status;
	}

	return NULL;
}

