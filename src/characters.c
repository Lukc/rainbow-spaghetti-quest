#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "characters.h"

#include "events.h"
#include "parser.h"
#include "term.h"
#include "colors.h"

static void
selection_color(int selected)
{
	if (selected)
	{
		bg(WHITE);
		fg(BLACK);
	}
	else
	{
		fg(WHITE);
	}
}

Character*
load_character(Game* game, ParserElement* element)
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
				load_events(game, &c->events, element->value);
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

void
quests(Game* game)
{
	int input = KEY_CLEAR;
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
					game,
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
				printf("  - %-56s", c->name);

				fg(MAGENTA);
				printf("%-10s", c->quester ? "quester" : "");

				fg(GREEN);
				printf("%-10s", c->trader ? "trader" : "");

				printf("\n");
				nocolor();
				selection_color(i == selection);
				printf("    %-76s\n", c->description);

				nocolor();

				l = l->next;
			}
			else
				printf("\n\n");
		}

		menu_separator();

		fg(WHITE);
		printf("  (t) Talk to\n");
		back(1);
		move(40);
		printf("  (l) Leave\n");

		menu_separator();

		input = getch();
	}

	system("clear");
}

