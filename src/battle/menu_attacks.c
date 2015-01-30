#include <stdio.h>

#include "menu_attacks.h"

#include "../term.h"
#include "../colors.h"
#include "../attack.h"

#include "attack.h"

/**
 * Prints the attacks’ selection menu of the battle interface.
 * @param list: The List* of AttackData* to display.
 */
void
battle_attacks_menu(Entity* player, List* list, int selection)
{
	int i;
	int begin;
	int can_use;

	begin = selection - selection % 5;

	for (i = 0; i < begin; i++)
		list = list->next;

	for (i = 0; i < 5; i++)
	{
		AttackData* ad;
		Attack* attack;

		if (list)
		{
			ad = list->data;
			attack = ad->attack;

			can_use = can_use_attack(player, ad);

			if (i == selection % 5)
			{
				bg(WHITE);
				fg(BLACK);
			}
			else
			{
				if (can_use > 0)
					fg(WHITE);
				else
					bg(BLACK);
			}

			printf(" %-37s ", attack->name);

			move(27);

			if (can_use > 0)
			{
				if (ad->charge < attack->charge)
				{
					printf(" --  ");
					fg(YELLOW);
					printf("%i/%i", ad->charge, attack->charge);
					fg(WHITE);
					printf("  --");
				}
				else
				{
					if (attack->charge)
						fg(YELLOW);

					printf(" -- READY --");
				}
			}
			else if (can_use == E_COOLDOWN)
				printf(" -- CDN %i --", ad->cooldown);
			else if (can_use == E_NO_MANA)
				printf(" -- NO MP --");
			else if (can_use == E_NO_HEALTH)
				printf(" -- NO HP --");

			printf("\n");
			nocolor();

			list = list->next;
		}
		else
		{
			fg(BLACK);
			printf(" ------------ \n");
		}
	}
}

/* vim: set ts=4 sw=4 cc=80 : */
