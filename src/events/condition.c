#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "condition.h"

#include "../events.h"
#include "../parser.h"

Event*
load_condition_event(Game* game, ParserElement* element)
{
	ConditionEvent* event;
	List* l;
	ParserElement* e;

	if (element->type != PARSER_LIST)
	{
		fprintf(stderr, "[:%i] Event “%s” is not a list.\n",
			element->lineno, element->name);

		return NULL;
	}

	event = malloc(sizeof(*event));
	memset(event, 0, sizeof(*event));
	event->type = EVENT_CONDITION;

	for (l = element->value; l; l = l->next)
	{
		e = l->data;

		if (!strcmp(e->name, "name"))
			event->name = parser_get_string(e, NULL);
		else if (!strcmp(e->name, "if"))
		{
			if (e->type == PARSER_LIST)
				load_condition(&event->condition, e->value);
			else
				fprintf(stderr, "[:%i] “if” field is not a list.\n",
					e->lineno);
		}
		else if (!strcmp(e->name, "then"))
		{
			if (e->type == PARSER_LIST)
				load_events(game, &event->then, e->value);
			else
				fprintf(stderr, "[:%i] “then” field is not a list.\n",
					e->lineno);
		}
		else if (!strcmp(e->name, "else"))
		{
			if (e->type == PARSER_LIST)
				load_events(game, &event->_else, e->value);
			else
				fprintf(stderr, "[:%i] “else” field is not a list.\n",
					e->lineno);
		}
		else
			fprintf(stderr, "[:%i] Unrecognized field: %s.\n",
				e->lineno, e->name);
	}

	return (Event*) event;
}

void
fire_condition_event(Game* game, Event* event)
{
	ConditionEvent* e = (ConditionEvent*) event;
	List* l;
	List* events;

	if (condition_check(game, &e->condition))
		events = e->then;
	else
		events = e->_else;

	for (l = events; l; l = l->next)
		fire_event(game, l->data);
}

