#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "conditions.h"
#include "parser.h"
#include "items.h"
#include "statuses.h"

int
condition_check(Game* game, Condition* c)
{
	List* l;
	int failed = 0;

	for (l = c->items; l && !failed; l = l->next)
	{
		ItemStack* stack = l->data;

		if (get_count_from_inventory(game->player->inventory, stack->item) < stack->quantity)
			failed = 1;
	}

	for (l = c->has_statuses; l && !failed; l = l->next)
	{
		Status* status = l->data;

		if (!has_status(game->player, status))
			failed = 1;
	}

	for (l = c->variables; l && !failed; l = l->next)
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
		else
		{
			if (variable)
				switch (c->condition)
				{
					case VARIABLE_EQUALS:
						failed = ! variable->value == c->value;
						break;
					case VARIABLE_LOWER:
						failed = ! variable->value < c->value;
						break;
					case VARIABLE_GREATER:
						failed = ! variable->value > c->value;
						break;
					case VARIABLE_LOWER_OR_EQUAL:
						failed = ! variable->value <= c->value;
						break;
					case VARIABLE_GREATER_OR_EQUAL:
						failed = ! variable->value >= c->value;
						break;
					default:
						/* To avoid cc warning, but shouldn’t happen. */
						break;
				}
			else
				/* Can’t compare with non-existant values. Also, maybe
				 * we should print an error or something. */
				failed = 1;
		}
	}

	return !failed;
}

void
load_condition(Condition* condition, List* elements)
{
	ParserElement* e;

	memset(condition, 0, sizeof(*condition));

	for (; elements; elements = elements->next)
	{
		e = elements->data;

		if (!strcmp(e->name, "variable"))
		{
			VariableCondition* cond;

			if (e->type == PARSER_STRING)
			{
				cond = malloc(sizeof(*cond));
				cond->condition = VARIABLE_EXISTS;
				cond->variable = parser_get_string(e, NULL);

				list_add(&condition->variables, cond);
			}
			else if (e->type == PARSER_LIST)
			{
				int type = -1;
				char* variable = NULL;
				int value = 0;
				List* l;

				for (l = e->value; l; l = l->next)
				{
					ParserElement* elem = l->data;

					if (!strcmp(elem->name, "name"))
						variable = parser_get_string(elem, NULL);
					else if (!strcmp(elem->name, "equals"))
					{
						type = VARIABLE_EQUALS;

						value = parser_get_integer(elem, NULL);
					}
					else if (!strcmp(elem->name, "not equals"))
					{
						type = VARIABLE_NOT_EQUALS;

						value = parser_get_integer(elem, NULL);
					}
					else if (!strcmp(elem->name, "lower"))
					{
						type = VARIABLE_LOWER;

						value = parser_get_integer(elem, NULL);
					}
					else if (!strcmp(elem->name, "greater"))
					{
						type = VARIABLE_GREATER;

						value = parser_get_integer(elem, NULL);
					}
					else if (!strcmp(elem->name, "lower or equal"))
					{
						type = VARIABLE_LOWER_OR_EQUAL;

						value = parser_get_integer(elem, NULL);
					}
					else if (!strcmp(elem->name, "greater or equal"))
					{
						type = VARIABLE_GREATER_OR_EQUAL;

						value = parser_get_integer(elem, NULL);
					}
					else if (!strcmp(elem->name, "exists"))
					{
						type = VARIABLE_EXISTS;

						value = parser_get_integer(elem, NULL);
					}
				}

				if (type != -1)
				{
					cond = malloc(sizeof(*cond));
					cond->condition = type;
					cond->variable = variable;
					cond->value = value;

					list_add(&condition->variables, cond);
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

			list_add(&condition->items, is);
		}
		else if (!strcmp(e->name, "has status"))
			list_add(&condition->has_statuses, parser_get_string(e, NULL));
		else
			fprintf(stderr, "[:%i] Unrecognized field: %s.\n",
				e->lineno, e->name);
	}
}

