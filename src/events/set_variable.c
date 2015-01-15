#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "set_variable.h"

Event*
load_set_variable_event(ParserElement* element)
{
	SetVariableEvent* e;
	int value = 0;
	char* varname = NULL;
	char* name = NULL;

	if (element->type == PARSER_STRING)
		varname = parser_get_string(element, NULL);
	else if (element->type == PARSER_LIST)
	{
		List* l;

		for (l = element->value; l; l = l->next)
		{
			ParserElement* elem = l->data;

			if (!strcmp(elem->name, "name"))
				name = parser_get_string(elem, NULL);
			else if (!strcmp(elem->name, "variable name") ||
				!strcmp(elem->name, "variable"))
				varname = parser_get_string(elem, NULL);
			else if (!strcmp(elem->name, "value"))
				value = parser_get_integer(elem, NULL);
			else
				fprintf(stderr, "[:%i] Unrecognized field: %s.\n",
					elem->lineno, elem->name);
		}
	}
	else
		fprintf(stderr,
			"[:%i] “Set Variable” event is not a list or string.\n",
			element->lineno);

	if (varname)
	{
		e = malloc(sizeof(*e));
		e->name = name;
		e->type = EVENT_SET_VARIABLE;
		e->variable = varname;
		e->value = value;

		return (Event*) e;
	}
	else
	{
		fprintf(stderr, "[:%i] “Set Variable” with no Name field.\n",
			element->lineno);

		return NULL;
	}
}

void
fire_set_variable_event(Game* game, Event* event)
{
	SetVariableEvent* e = (SetVariableEvent*) event;
	Variable* v;
	List* l;

	for (l = game->variables; l; l = l->next)
	{
		v = l->data;

		if (!strcmp(v->name, e->variable))
		{
			v->value = e->value;

			return;
		}
	}

	/* Still here? None found, uh. :( */
	v = malloc(sizeof(*v));
	v->name = strdup(e->variable);
	v->value = e->value;

	list_add(&game->variables, v);
}

