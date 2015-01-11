#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "characters.h"

#include "parser.h"
#include "term.h"
#include "colors.h"

static void load_events(List**, List*);

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

static void
load_message_event(MessageEvent* event, List* elements)
{
	ParserElement* e;

	for (; elements; elements = elements->next)
	{
		e = elements->data;

		if (!strcmp(e->name, "from"))
		{
			if (event->from)
				fprintf(stderr, "[:%i] Duplicated “from” field in Message event.\n",
					e->lineno);
			else
				event->from = parser_get_string(e, NULL);
		}
		else if (!strcmp(e->name, "text"))
		{
			if (event->text)
				fprintf(stderr, "[:%i] Duplicated “text” field in Message event.\n",
					e->lineno);
			else
				event->text = parser_get_string(e, NULL);
		}
		else
			fprintf(stderr, "[:%i] Unrecognized field “%s”.\n",
				e->lineno, e->name);
	}
}

static void
load_choice_event(ChoiceEvent* event, List* elements)
{
	ParserElement* e;

	for (; elements; elements = elements->next)
	{
		e = elements->data;

		if (!strcmp(e->name, "option"))
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
					load_events(&option->events, e->value);
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
}

static void
load_events(List** events, List* elements)
{
	ParserElement* element;

	for (; elements; elements = elements->next)
	{
		element = elements->data;

		if (element->type != PARSER_LIST)
		{
			/* Everything here has to be a list. */
			fprintf(stderr, "[:%i] Event “%s” is not a list.\n",
				element->lineno, element->name);

			continue;
		}

		if (!strcmp(element->name, "message"))
		{
			MessageEvent* event;

			event = malloc(sizeof(*event));
			event->type = EVENT_MESSAGE;
			event->from = NULL;
			event->text = NULL;

			load_message_event(event, element->value);

			list_add(events, event);
		}
		else if (!strcmp(element->name, "choice"))
		{
			ChoiceEvent* event;

			event = malloc(sizeof(*event));
			event->type = EVENT_CHOICE;
			event->options = NULL;

			load_choice_event(event, element->value);

			list_add(events, event);
		}
		else
			fprintf(stderr, "[:%i] Unknown event ignored: %s.\n",
				element->lineno, element->name);
	}
}

Character*
load_character(ParserElement* element)
{
	List* l;
	Character* c;

	if (element->type != PARSER_LIST)
	{
		fprintf(stderr, "[:%i] Character is not a list.\n", element->lineno);

		return NULL;
	}

	c = malloc(sizeof(*c));

	c->name = NULL;
	c->description = NULL;
	c->events = NULL;
	c->quester = 0;
	c->trader = 0;

	for (l = element->value; l; l = l->next)
	{
		ParserElement* element = l->data;

		if (!strcmp(element->name, "name"))
			c->name = parser_get_string(element, NULL);
		else if (!strcmp(element->name, "description"))
			c->description = parser_get_string(element, NULL);
		else if (!strcmp(element->name, "quester"))
			c->quester = parser_get_integer(element, NULL);
		else if (!strcmp(element->name, "trader"))
			c->trader = parser_get_integer(element, NULL);
		else if (!strcmp(element->name, "on talk"))
		{
			if (element->type == PARSER_LIST)
				load_events(&c->events, element->value);
			else
				fprintf(stderr, "[:%i] Character->On Talk is not a list.\n",
					element->lineno);
		}
	}

	if (!c->name)
		fprintf(stderr, "[:%i] Invalid character has no name.\n",
			element->lineno);
	if (!c->description)
		fprintf(stderr, "[:%i] Invalid character has no description.\n",
			element->lineno);

	return c;
}

static void
fire_event(Event* event)
{
	int i;

	if (event->type == EVENT_MESSAGE)
	{
		MessageEvent* e = (MessageEvent*) event;

		if (e->from)
		{
			fg(5, 5, 0);
			printf(BRIGHT "  >> ");
			printf(WHITE);
			printf("%s\n", e->from);
		}

		printf(NOCOLOR WHITE " %s\n", e->text);

		printf(NOCOLOR "\nPress any key to continue...\n");
		getch();
		back(1);
		for (i = 0; i < 80; i++)
			printf(" ");
		printf("\n");
		back(1);
	}
	else if (event->type == EVENT_CHOICE)
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
						fire_event(l->data);

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
}

static void
fire_events(List* l)
{
	Event* e;

	system("clear");
	printf("\n");

	for (; l; l = l->next)
	{
		e = l->data;

		fire_event(e);
	}

	system("clear");
}

void
quests(Game* game)
{
	int input = KEY_CLEAR;
	char* log;
	int i;
	int selection = 0;
	Place* location;
	List* l;

	location = game->location;

	system("clear");

	while (input != 'l')
	{
		switch (input)
		{
			case ' ':
				log = NULL;
			case KEY_CLEAR:
				break;
			case KEY_UP:
				selection = selection > 0 ? selection - 1 : selection;
				break;
			case KEY_DOWN:
				selection = selection < list_size(location->characters) - 1 ?
					selection + 1 : selection;
				break;
			case 't':
				fire_events(
					((Character*) list_nth(location->characters, selection))
						->events
				);
				/* Oh my oh my, firing all those events. :( */
				break;
		}

		back_to_top();

		l = location->characters;
		for (i = 0; i < 10; i++)
		{
			if (l)
			{
				Character* c = l->data;

				selection_color(i == selection);
				printf("  - " BRIGHT "%-56s" NOCOLOR, c->name);

				selection_color(i == selection);
				printf(MAGENTA);
				printf("%-10s", c->quester ? "quester" : "");

				printf(GREEN);
				printf("%-10s", c->trader ? "trader" : "");

				printf("\n" NOCOLOR);
				selection_color(i == selection);
				printf("    %-76s\n", c->description);

				printf(NOCOLOR);

				l = l->next;
			}
			else
				printf("\n\n");
		}

		menu_separator();

		printf(WHITE "  (t) Talk to\n" NOCOLOR);

		menu_separator();

		input = getch();
	}

	system("clear");
}

