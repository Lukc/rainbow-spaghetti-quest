#include <stdio.h>

#include "menu_items.h"

#include "../term.h"
#include "../colors.h"
#include "../items.h"

/**
 * Prints the items’ selection menu of the battle interface.
 */
void
battle_items_menu(Entity* player, int selection)
{
	int i;
	int begin;

	begin = selection - selection % 5;

	for (i = begin; i < begin + 5; i++)
	{
		if (i < INVENTORY_SIZE)
		{
			Item* item;

			if (i == selection)
			{
				bg(WHITE);
			}

			if ((item = player->inventory[i].item))
			{
				if (is_item_usable(item))
				{
					if (item->consumable)
						fg(GREEN);
					else
						fg(BLUE);
				}
				else
				{
					if (i == selection)
						fg(GRAY);
					else
						fg(BLACK);
				}

				if (player->inventory[i].quantity > 1)
					printf(" %ix %-37s\n",
						player->inventory[i].quantity, item->name);
				else
					printf(" %-39s\n", item->name);
			}
			else
			{
				int j;

				fg(BLACK);
				printf(" ");
				for (j = 0; j < 37; j++)
					printf("-");
				printf(" \n");
			}
		}
		else
			printf("\n");

		nocolor();
	}
}

/* vim: set ts=4 sw=4 cc=80 : */
