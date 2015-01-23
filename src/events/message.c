#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "message.h"

#include "../events.h"
#include "../list.h"
#include "../colors.h"
#include "../term.h"

Event*
load_message_event(ParserElement* element)
{
	MessageEvent* event;
	List* l;
	ParserElement* e;

	if (element->type != PARSER_LIST)
	{
		fprintf(stderr, "[:%i] Event “%s” is not a list.\n",
			element->lineno, element->name);

		return NULL;
	}

	event = malloc(sizeof(*event));
	event->type = EVENT_MESSAGE;
	event->name = NULL;
	event->from = NULL;
	event->text = NULL;

	for (l = element->value; l; l = l->next)
	{
		e = l->data;

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

	return (Event*) event;
}

void
fire_message_event(Event* event)
{
	MessageEvent* e = (MessageEvent*) event;
	int i;

	if (e->from)
	{
		fg(YELLOW);
		printf("  >> ");
		fg(WHITE);
		printf("%s\n", e->from);
	}

	fg(WHITE);
	printf(" %s\n", e->text);

	printf("\nPress any key to continue...\n");
	nocolor();

	/* Removing the above line after a key’s pressed. */
	getch();
	back(1);
	for (i = 0; i < 80; i++)
		printf(" ");
	printf("\n");
	back(1);
}

