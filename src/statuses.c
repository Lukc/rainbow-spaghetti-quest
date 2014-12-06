#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "parser.h"
#include "statuses.h"

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

static Status*
load_status(ParserElement* root)
{
	List* list;
	Status* status;

	status = (Status*) malloc(sizeof(Status));
	memset(status, 0, sizeof(Status));

	for (list = root->value; list; list = list->next)
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
		else
		{
			fprintf(stderr,
				"[Status:%s] Unknown field “%s”.\n",
				status->name ? status->name : "(null)", field);
		}

		parser_free(element);
	}

	return status;
}

List*
load_statuses(char* filename)
{
	List* list = load_file(filename);
	List* statuses = NULL;

	while (list)
	{
		char* field;
		ParserElement* element;

		element = list->data;
		field = element->name;

		if (!strcmp(field, "status"))
		{
			Status* status;

			if (element->type != PARSER_LIST)
			{
				fprintf(stderr, "[%s]> Status improperly defined.\n",
					filename);

				continue;
			}

			status = load_status(element);

			if (status)
				list_add(&statuses, status);
		}
		else
		{
			fprintf(stderr, "[%s]> Unknown field: “%s”.\n",
				filename, element->name);
		}

		list = list->next;
	}

	return statuses;
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
	data->duration = -1;
	data->status = status;

	list_add(&e->statuses, data);

	return 1;
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

