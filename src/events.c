#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "events.h"
#include "parser.h"

#include "events/message.h"
#include "events/set_variable.h"
#include "events/condition.h"
#include "events/choice.h"
#include "events/fire.h"
#include "events/items.h"

/* Required for messages and condition events. */
#include "colors.h"
#include "term.h"

void
load_events(Game* game, List** events, List* elements)
{
	ParserElement* element;

	for (; elements; elements = elements->next)
	{
		Event* event = NULL;

		element = elements->data;

		if (!strcmp(element->name, "message"))
			event = load_message_event(element);
		else if (!strcmp(element->name, "choice"))
			event = load_choice_event(game, element);
		else if (!strcmp(element->name, "condition"))
			event = load_condition_event(game, element);
		else if (!strcmp(element->name, "give item") ||
		         !strcmp(element->name, "remove item"))
			event = load_item_event(element);
		else if (!strcmp(element->name, "set variable"))
			event = load_set_variable_event(element);
		else if (!strcmp(element->name, "fire event"))
			event = load_fire_event(element);
		else
			fprintf(stderr, "[:%i] Unknown event ignored: %s.\n",
				element->lineno, element->name);

		if (event)
		{
			list_add(events, event);
			list_add(&game->events, event);
		}
	}
}

void
fire_event(Game* game, Event* event)
{
	if (event->type == EVENT_FIRE)
	{
		FireEvent* self = (FireEvent*) event;
		List* l;

		for (l = game->events; l; l = l->next)
		{
			Event* e = l->data;

			if (e->name && !strcmp(e->name, self->event))
				fire_event(game, e);
		}
	}
	else if (event->type == EVENT_MESSAGE)
		fire_message_event(event);
	else if (event->type == EVENT_CHOICE)
		fire_choice_event(game, event);
	else if (event->type == EVENT_CONDITION)
		fire_condition_event(game, event);
	else if (event->type == EVENT_GIVE_ITEM)
		fire_give_item_event(game, event);
	else if (event->type == EVENT_REMOVE_ITEM)
		fire_remove_item_event(game, event);
	else if (event->type == EVENT_SET_VARIABLE)
		fire_set_variable_event(game, event);
	else
		{} /* Broken event? We have nowhere to log it anyway… */
}

void
fire_events(Game* game, List* l)
{
	Event* e;

	system("clear");
	printf("\n");

	for (; l; l = l->next)
	{
		e = l->data;

		fire_event(game, e);
	}

	system("clear");
}

