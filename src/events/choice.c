#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "choice.h"

#include "../events.h"
#include "../parser.h"
#include "../game.h"

#include "../colors.h"
#include "../term.h"

Event*
load_choice_event(Game* game, ParserElement* element)
{
	ChoiceEvent* event;
	ParserElement* e;
	List* l;

	if (element->type != PARSER_LIST)
	{
		fprintf(stderr, "[:%i] Event “%s” is not a list.\n",
			element->lineno, element->name);

		return NULL;
	}

	event = malloc(sizeof(*event));
	event->type = EVENT_CHOICE;
	event->name = NULL;
	event->options = NULL;

	for (l = element->value; l; l = l->next)
	{
		e = l->data;

		if (!strcmp(e->name, "name"))
			event->name = parser_get_string(e, NULL);
		else if (!strcmp(e->name, "option"))
		{
			ChoiceEventOption* option;
			List* l;

			if (e->type != PARSER_LIST)
			{
				fprintf(stderr, "[:%i] Option is not a list.\n", e->lineno);

				continue;
			}

			option = malloc(sizeof(*option));
			option->text = NULL;
			option->events = NULL;

			for (l = e->value; l; l = l->next)
			{
				ParserElement* e = l->data;

				if (!strcmp(e->name, "text"))
					option->text = parser_get_string(e, NULL);
				else if (!strcmp(e->name, "events"))
					load_events(game, &option->events, e->value);
				else
					fprintf(stderr, "[:%i] Unrecognized field “%s”.\n",
						e->lineno, e->name);
			}

			if (option->text && option->events)
				list_add(&event->options, option);
			else
				fprintf(stderr, "[:%i] Incomplete Option in Choice event.\n",
					e->lineno);
		}
		else
			fprintf(stderr, "[:%i] Unrecognized field “%s”.\n",
				e->lineno, e->name);
	}

	return (Event*) event;
}

/* FIXME: Put that in colors.c/term.c? */
static void
selection_color(int selected)
{
	if (selected)
	{
		bg(4, 4, 4);
		fg(0, 0, 0);
	}
	else
	{
		printf(WHITE);
	}
}

void
fire_choice_event(Game* game, Event* event)
{
	ChoiceEvent* e = (ChoiceEvent*) event;
	List* l;
	ChoiceEventOption* option;
	int input = KEY_CLEAR;
	int selection = 0;

	while (1)
	{
		int i = 0;

		switch (input)
		{
			case KEY_CLEAR:
			case ' ':
				break;
			case KEY_UP:
				selection = selection > 0 ? selection - 1 : selection;
				break;
			case KEY_DOWN:
				selection = selection < list_size(e->options) - 1 ?
					selection + 1 : selection;
				break;
			case '\n':
			case KEY_ENTER:
				back(1);
				for (i = 0; i < 80; i++)
					printf(" ");
				printf("\n");
				back(1);

				option = list_nth(e->options, selection);
				for (l = option->events; l; l = l->next)
					fire_event(game, l->data);

				return;
		}

		if (input != KEY_CLEAR)
			back(list_size(e->options) + 2);

		for (l = e->options; l; l = l->next)
		{
			ChoiceEventOption* option = l->data;

			selection_color(selection == i);
			printf("    - %-74s\n", option->text);
			printf(NOCOLOR);

			i++;
		}

		printf("\n  Press (Enter) to select and continue.\n");

		input = getch();
	}
}

