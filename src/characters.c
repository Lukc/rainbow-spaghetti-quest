#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "characters.h"

#include "parser.h"
#include "term.h"
#include "colors.h"

static void load_events(Game*, List**, List*);

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
load_choice_event(Game* game, ChoiceEvent* event, List* elements)
{
	ParserElement* e;

	for (; elements; elements = elements->next)
	{
		e = elements->data;

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
}

static void
load_condition_event(Game* game, ConditionEvent* event, List* elements)
{
	ParserElement* e;

	for (; elements; elements = elements->next)
	{
		e = elements->data;

		if (!strcmp(e->name, "name"))
			event->name = parser_get_string(e, NULL);
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
		else if (!strcmp(e->name, "variable"))
		{
			VariableCondition* cond;

			if (e->type == PARSER_STRING)
			{
				cond = malloc(sizeof(*cond));
				cond->condition = VARIABLE_EXISTS;
				cond->variable = parser_get_string(e, NULL);

				list_add(&event->variables, cond);
			}
			else if (e->type == PARSER_LIST)
			{
				int condition = -1;
				char* variable = NULL;
				int value = 0;
				List* l;

				for (l = e->value; l; l = l->next)
				{
					ParserElement* elem = l->data;

					if (!strcmp(elem->name, "name"))
						variable = parser_get_string(elem, NULL);
					else if (!strcmp(elem->name, "not equals"))
					{
						condition = VARIABLE_NOT_EQUALS;

						value = parser_get_integer(elem, NULL);
					}
					else if (!strcmp(elem->name, "exists"))
					{
						condition = VARIABLE_EXISTS;

						value = parser_get_integer(elem, NULL);
					}
				}

				if (condition != -1)
				{
					cond = malloc(sizeof(*cond));
					cond->condition = condition;
					cond->variable = variable;
					cond->value = value;

					list_add(&event->variables, cond);
				}
			}
			else
				fprintf(stderr,
					"[:%i] “Variable” condition is not a list or string.\n",
					e->lineno);
		}
		else if (!strcmp(e->name, "requires item"))
		{
			ItemStack* is;
			int quantity = 0;
			char* name = NULL;

			if (e->type == PARSER_STRING)
			{
				quantity = 1;
				name = parser_get_string(e, NULL);
			}
			else if (e->type == PARSER_LIST)
			{
				List* l;

				for (l = e->value; l; l = l->next)
				{
					ParserElement* element = l->data;

					if (!strcmp(element->name, "item"))
						name = parser_get_string(element, NULL);
					else if (!strcmp(element->name, "quantity"))
						quantity = parser_get_integer(element, NULL);
					else
						fprintf(stderr, "[:%i] Unrecognized field “%s”.\n",
							element->lineno, element->name);
				}
			}
			else
			{
				fprintf(stderr,
					"[:%i] “requires item” field is not a string or list.\n",
					e->lineno);
				continue;
			}

			is = malloc(sizeof(*is));
			is->quantity = quantity;
			is->item = (Item*) name;
		}
		else
			fprintf(stderr, "[:%i] Unrecognized field: %s.\n",
				e->lineno, e->name);
	}
}

static void
load_events(Game* game, List** events, List* elements)
{
	ParserElement* element;
	List* l;

	for (; elements; elements = elements->next)
	{
		element = elements->data;

		if (!strcmp(element->name, "message"))
		{
			MessageEvent* event;

			if (element->type != PARSER_LIST)
			{
				fprintf(stderr, "[:%i] Event “%s” is not a list.\n",
					element->lineno, element->name);

				continue;
			}

			event = malloc(sizeof(*event));
			event->type = EVENT_MESSAGE;
			event->name = NULL;
			event->from = NULL;
			event->text = NULL;

			load_message_event(event, element->value);

			list_add(events, event);
		}
		else if (!strcmp(element->name, "choice"))
		{
			ChoiceEvent* event;

			if (element->type != PARSER_LIST)
			{
				fprintf(stderr, "[:%i] Event “%s” is not a list.\n",
					element->lineno, element->name);

				continue;
			}

			event = malloc(sizeof(*event));
			event->type = EVENT_CHOICE;
			event->name = NULL;
			event->options = NULL;

			load_choice_event(game, event, element->value);

			list_add(events, event);
		}
		else if (!strcmp(element->name, "condition"))
		{
			ConditionEvent* event;

			if (element->type != PARSER_LIST)
			{
				fprintf(stderr, "[:%i] Event “%s” is not a list.\n",
					element->lineno, element->name);

				continue;
			}

			event = malloc(sizeof(*event));
			memset(event, 0, sizeof(*event));
			event->type = EVENT_CONDITION;

			load_condition_event(game, event, element->value);

			list_add(events, event);
		}
		else if (!strcmp(element->name, "give item") ||
		         !strcmp(element->name, "remove item"))
		{
			GiveItemEvent* event;

			event = malloc(sizeof(*event));
			event->item = NULL;
			event->quantity = 1;

			if (!strcmp(element->name, "give item"))
				event->type = EVENT_GIVE_ITEM;
			else
				event->type = EVENT_REMOVE_ITEM;

			event->name = NULL;

			if (element->type == PARSER_STRING)
				event->item = (Item*) parser_get_string(element, NULL);
			else if (element->type == PARSER_LIST)
			{
				List* l;

				for (l = element->value; l; l = l->next)
				{
					ParserElement* element = l->data;

					if (!strcmp(element->name, "name"))
						event->name = parser_get_string(element, NULL);
					else if (!strcmp(element->name, "item"))
						event->item = (Item*) parser_get_string(element, NULL);
					else if (!strcmp(element->name, "quantity"))
						event->quantity = parser_get_integer(element, NULL);
					else
						fprintf(stderr, "[:%i] Unrecognized field “%s”.\n",
							element->lineno, element->name);
				}
			}
			else
				fprintf(stderr,
					"[:%i] Event “Give Item” is not a list or string.\n",
					element->lineno);

			list_add(events, event);
		}
		else if (!strcmp(element->name, "set variable"))
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

				list_add(events, e);
			}
			else
				fprintf(stderr, "[:%i] “Set Variable” with no Name field.\n",
					element->lineno);
		}
		else if (!strcmp(element->name, "fire event"))
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

				list_add(events, e);
			}
			else
				fprintf(stderr, "[:%i] “Set Variable” with no Name field.\n",
					element->lineno);
		}
		else
			fprintf(stderr, "[:%i] Unknown event ignored: %s.\n",
				element->lineno, element->name);
	}

	for (l = *events; l; l = l->next)
	{
		Event* event = l->data;

		list_add(&game->events, event);
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

static void
fire_event(Game* game, Event* event)
{
	int i;

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
	else if (event->type == EVENT_CONDITION)
	{
		ConditionEvent* e = (ConditionEvent*) event;
		List* l;
		List* events;
		int failed = 0;

		for (l = e->items; l && !failed; l = l->next)
		{
			ItemStack* stack = l->data;

			if (get_count_from_inventory(game->player->inventory, stack->item) < stack->quantity)
				failed = 1;
		}

		for (l = e->variables; l; l = l->next)
		{
			List* sl;
			VariableCondition* c = l->data;
			Variable* variable = NULL;

			for (sl = game->variables; sl && !variable; sl = sl->next)
			{
				Variable* v = sl->data;

				if (!strcmp(v->name, c->variable))
					variable = v;
			}

			if (c->condition == VARIABLE_EXISTS)
			{
				if (!variable)
					failed = 1;
			}
			else if (c->condition == VARIABLE_NOT_EQUALS)
			{
				if (variable)
					failed = ! c->value != variable->value;
				/* NULL has to be different from everything, right? */
			}
		}

		if (failed)
			events = e->_else;
		else
			events = e->then;

		for (l = events; l; l = l->next)
			fire_event(game, l->data);
	}
	else if (event->type == EVENT_GIVE_ITEM)
	{
		GiveItemEvent* e = (GiveItemEvent*) event;
		int i;

		for (i = 0; i < e->quantity; i++)
		{
			give_item(game->player, e->item);
		}

		if (e->quantity == 1)
			printf("Received a %s!\n", e->item->name);
		else
			printf("Received %ix %s!\n", e->quantity, e->item->name);
		printf("\n");
	}
	else if (event->type == EVENT_REMOVE_ITEM)
	{
		RemoveItemEvent* e = (RemoveItemEvent*) event;

		remove_items(game->player, e->item, e->quantity);

		if (e->quantity == 1)
			printf("Lost a %s!\n", e->item->name);
		else
			printf("Lost %ix %s!\n", e->quantity, e->item->name);
		printf("\n");
	}
	else if (event->type == EVENT_SET_VARIABLE)
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
}

static void
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
		back(1);
		move(40);
		printf(WHITE "  (l) Leave\n" NOCOLOR);

		menu_separator();

		input = getch();
	}

	system("clear");
}

