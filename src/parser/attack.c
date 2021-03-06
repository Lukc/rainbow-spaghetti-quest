#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "attack.h"
#include "buff.h"

/**
 * Makes an Attack* usable in-game.
 *
 * Strings representing statuses are converted into pointers.
 */
void
parser_load_attack(Game* game, Attack* attack)
{
	List* list;

	if (attack->inflicts_status_name)
	{
		attack->inflicts_status =
			get_status_by_name(game->statuses,
				attack->inflicts_status_name);

		if (!attack->inflicts_status)
		{
			fprintf(stderr, "[Attack:%s] Inflicts unknown status: %s!\n",
				attack->name, attack->inflicts_status_name);
		}
	}

	if (attack->self_inflicts_status_name)
	{
		attack->self_inflicts_status =
			get_status_by_name(game->statuses,
				attack->self_inflicts_status_name);

		if (!attack->self_inflicts_status)
		{
			fprintf(stderr, "[Attack:%s] Self inflicts unknown status: %s!\n",
				attack->name, attack->self_inflicts_status_name);
		}

		free(attack->self_inflicts_status_name);
	}

	for (list = attack->cures_status_names; list; list = list->next)
	{
		char* name = list->data;
		Status* status = get_status_by_name(game->statuses, name);

		if (status)
			list_add(&attack->cures_statuses, status);
		else
			fprintf(stderr, "[Attack:%s] Cures unknown status: %s\n",
				attack->name, name);

		free(name);
	}
}

Attack*
parser_get_attack(ParserElement* element)
{
	List* list;
	Attack* attack;

	if (element->type != PARSER_LIST)
	{
		fprintf(stderr, "<%s:%i> %s is not a list.\n",
			element->filename, element->lineno, element->name);
		return NULL;
	}
	else
	{
		attack = (Attack*) malloc(sizeof(Attack));
		memset(attack, 0, sizeof(Attack));
		attack->inflicts_status = NULL;

		for (list = element->value; list; list = list->next)
		{
			element = list->data;

			if (!strcmp(element->name, "damage"))
			{
				if (element->type == PARSER_INTEGER)
				{
					attack->damage.min = parser_get_integer(element, NULL);
					attack->damage.max = attack->damage.min;
				}
				else if (element->type == PARSER_LIST)
				{
					List* l;

					for (l = element->value; l; l = l->next)
					{
						element = l->data;

						if (!strcmp(element->name, "min"))
							attack->damage.min = parser_get_integer(element, NULL);
						else if (!strcmp(element->name, "max"))
							attack->damage.max = parser_get_integer(element, NULL);
						else
							fprintf(stderr,
								"<%s:%i> Damage field contains an unknown element: %s\n",
								element->filename, element->lineno, element->name);
					}
				}
				else
				{
					fprintf(stderr,
						"<%s:%i> “damage” field is not an integer or a list.\n",
						element->filename, element->lineno);
					fprintf(stderr, " -> Attack will be useless.\n");
				}
			}
			else if (!strcmp(element->name, "strikes"))
				attack->strikes = parser_get_integer(element, NULL);
			else if (!strcmp(element->name, "cooldown"))
				attack->cooldown = parser_get_integer(element, NULL);
			else if (!strcmp(element->name, "name"))
				attack->name = parser_get_string(element, NULL);
			else if (!strcmp(element->name, "cures"))
				list_add(&attack->cures_status_names,
					parser_get_string(element, NULL));
			else if (!strcmp(element->name, "charge"))
				attack->charge = parser_get_integer(element, NULL);
			else if (!strcmp(element->name, "health"))
				attack->health = parser_get_integer(element, NULL);
			else if (!strcmp(element->name, "mana"))
				attack->mana = parser_get_integer(element, NULL);
			else if (!strcmp(element->name, "inflicts"))
				attack->inflicts_status_name = parser_get_string(element, NULL);
			else if (!strcmp(element->name, "self inflicts"))
				attack->self_inflicts_status_name = parser_get_string(element, NULL);
			else if (!strcmp(element->name, "buff"))
			{
				/* Buff* applied to self. */
				Buff* buff = parser_get_buff(element);

				if (buff)
				{
					memcpy(&attack->self_buff, buff, sizeof(*buff));

					free(buff);
				}
			}
			else if (!strcmp(element->name, "debuff"))
			{
				/* Buff* inflicted to enemy. */
				Buff* buff = parser_get_buff(element);

				if (buff)
				{
					memcpy(&attack->enemy_buff, buff, sizeof(*buff));

					free(buff);
				}
			}
			else if (!strcmp(element->name, "type"))
			{
				if (element->type == PARSER_STRING)
				{
					char* type = element->value;

					if (type)
					{
						attack->type = string_to_type(type);

						if (attack->type == -1)
						{
							fprintf(stderr, "<%s:%i> Invalid type: “%s”.\n",
								element->filename, element->lineno, type);

							free_attack(attack);

							return NULL;
						}
					}
				}
				else
					fprintf(stderr, "<%s:%i> “Type” field is not a string.\n",
						element->filename, element->lineno);
			}
			else
			{
				fprintf(stderr, "<%s:%i> Unknown field ignored: %s.\n",
					element->filename, element->lineno, element->name);
			}
		}

		return attack;
	}
}

