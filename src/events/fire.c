#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "fire.h"

#include "../events.h"
#include "../parser.h"
#include "../list.h"

Event*
load_fire_event(ParserElement* element)
{
	FireEvent* e;
	char* name = NULL;
	char* event = NULL;

	if (element->type == PARSER_STRING)
		event = parser_get_string(element, NULL);
	else if (element->type == PARSER_LIST)
	{
		List* l;

		for (l = element->value; l; l = l->next)
		{
			ParserElement* elem = l->data;

			if (!strcmp(elem->name, "name"))
				name = parser_get_string(elem, NULL);
			else if (!strcmp(elem->name, "event"))
				event = parser_get_string(elem, NULL);
			else
				fprintf(stderr, "[:%i] Unrecognized field: %s.\n",
					elem->lineno, elem->name);
		}
	}
	else
		fprintf(stderr,
			"[:%i] “Fire Event” event is not a list or string.\n",
			element->lineno);

	if (event)
	{
		e = malloc(sizeof(*e));
		e->name = name;
		e->type = EVENT_FIRE;
		e->event = event;

		return (Event*) e;
	}
	else
	{
		fprintf(stderr, "[:%i] “Fire Event” with no Event field.\n",
			element->lineno);

		return NULL;
	}
}

