#include <stdio.h>

#include "attack_stats.h"

#include "../term.h"
#include "../colors.h"

#include "mana_cost.h"

void
battle_print_attack_stats(Attack* attack, Entity* player)
{
	int first = 1;

	menu_separator();

	printf("%80s", "");
	move(0);

	if (attack->mana)
	{
		int mana = get_mana_cost(player, attack);
		if (attack->mana > 0)
			fg(BLUE);
		else if (attack->mana < mana)
			fg(RED);
		else
			fg(WHITE);

		printf("%+iMP", mana);
		nocolor();

		first = 0;
	}

	if (attack->health)
	{
		if (first)
			first = 0;
		else
			printf(", ");

		if (attack->health > 0)
			fg(GREEN);
		else
			fg(RED);

		printf("%+iHP", attack->health);
		nocolor();
	}

	if (attack->cooldown)
	{
		if (first)
			first = 0;
		else
			printf(", ");

		fg(WHITE);
		printf("cooldown: %i", attack->cooldown);
		nocolor();
	}

	if (attack->strikes)
	{
		if (first)
			first = 0;
		else
			printf(", ");

		fg(WHITE);
		printf("(%i-%i)x%i", attack->damage.min, attack->damage.max, attack->strikes);
		printf(" %s damage", type_to_string(attack->type));
	}

	if (attack->inflicts_status)
	{
		nocolor();
		printf(", ");
		fg(MAGENTA);
		printf("inflicts %s", attack->inflicts_status->name);
	}

	if (attack->self_inflicts_status)
	{
		nocolor();
		printf(", ");
		fg(MAGENTA);
		printf("self-inflicts %s", attack->self_inflicts_status->name);
	}

	if (attack->cures_statuses)
	{
		List* l;

		for (l = attack->cures_statuses; l; l = l->next)
		{
			Status* status = l->data;

			nocolor();
			printf(", ");
			fg(CYAN);
			printf("cures %s", status->name);
		}
	}

	if (attack->charge)
	{
		nocolor();
		printf(", ");
		fg(YELLOW);

		if (attack->charge == 1)
			printf("charges 1 turn");
		else
			printf("charges %i turns", attack->charge);
	}

	if (!buff_is_empty(&attack->self_buff))
	{
		nocolor();
		printf(", ");

		fg(BLUE);
		printf("buffs you");
	}

	if (!buff_is_empty(&attack->enemy_buff))
	{
		nocolor();
		printf(", ");

		fg(BLUE);
		printf("inflicts debuffs");
	}

	printf("\n");
}

/* vim: set ts=4 sw=4 cc=80 : */

