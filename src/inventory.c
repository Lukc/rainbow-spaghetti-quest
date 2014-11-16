#include <stdlib.h>
#include <stdio.h>

#include "colors.h"
#include "entities.h"
#include "shop.h"
#include "places.h"
#include "term.h"

void
inventory(Game* game)
{
	Entity* player = game->player;
	char* error;
	int input = -42;
	int selection = 0;
	int printed;
	int i, j, k;

	system("clear");

	while (!isexit(input))
	{
		error = NULL;

		if (input == 'e')
		{
			Item* item = player->inventory[selection];

			if (item)
			{
				if (item->slot != -1)
				{
					/* FIXME: We only need to clear the top-left part of
					 *        the screen. */
					system("clear");

					equip_item(player, player->inventory[selection]);
				}
				else
				{
					error = "Can’t equip that!";
				}
			}
			else
			{
				error = "No item to equip!";
			}
		}
		else if (input == 's')
		{
			if (game->location->shop_items)
			{
				Item* item;

				if ((item = player->inventory[selection]))
					if (!item->unique)
						sell_item(player, item);
					else
						error = "Cannot sell unique items!";
				else
					error = "Nothing to sell...";
			}
			else
				error = "Can’t sell in places where there’s no shop!";
		}
		else if (input == KEY_DOWN)
			selection =
				selection % (INVENTORY_SIZE / 3) == INVENTORY_SIZE / 3 - 1 ?
				selection : selection + 1;
		else if (input == KEY_UP)
			selection = selection % (INVENTORY_SIZE / 3) == 0 ?
				selection : selection - 1;
		else if (input == KEY_RIGHT)
			selection = selection < INVENTORY_SIZE - 8 ?
				selection + 8 : selection;
		else if (input == KEY_LEFT)
			selection = selection >= 8 ?
				selection - 8 : selection;

		print_equipment(player);
		printf("\n");

		back_to_top();
		for (i = 0; i < 7; i++)
		{
			move(40);
			printf("│");

			if (i == 0)
				printf(BRIGHT BLUE " >> %s" NOCOLOR, player->name);
			else if (i == 1)
				printf("  <att %i> <def %i>",
					get_attack_bonus(player), get_defense_bonus(player));
			else if (i == 2)
				printf("  <HP %i> <MP %i>",
					get_max_health(player), get_max_mana(player));
			else
				printf("  %-10s: %-4i   %-10s: %-4i",
					type_to_string(i - 3), get_type_resistance(player, i - 3),
					type_to_string(i + 1), get_type_resistance(player, i + 1));

			printf("\n");
		}

		menu_separator();

		move(40);
		back(1);
		printf("┴\n");

		for (i = 0; i < INVENTORY_SIZE / 3; i++)
		{
			/* Alignment. Ah ah! */
			printf(" ");

			for (k = 0; k < 3; k++)
			{
				if (selection == i + k * 8)
					printf("\033[47m");

				if (player->inventory[i + k * 8])
				{
					Item* item = player->inventory[i + k * 8];

					/* FIXME: Make sure item’s name isn’t too long. */
					printed = printf("  %s", item->name);
				}
				else
					printed = printf("  (empty)");

				for (j = 0; j < 26 - printed; j++)
					printf(" ");

				printf(NOCOLOR);
			}

			printf("\n");

			printf(NOCOLOR);
		}

		menu_separator();

		printf(WHITE);
		printf("  (e)  Equip stuff.\n");

		back(1);
		move(40);
		printf(WHITE " Money:     %-8i\n" NOCOLOR, player->caps);

		if (!game->location->shop_items)
			printf(BLACK);
		else
			printf(WHITE);
		printf("  (s)  Sell stuff.\n");

		back(1);
		move(40);
		if (game->location->shop_items && player->inventory[selection])
		{
			printf(WHITE " Sell price %-8i\n" NOCOLOR,
				player->inventory[selection]->price * 2 / 3);
		}
		else
		{
			for (i = 0; i < 40; i++)
				printf(" ");
			printf("\n");
		}

		printf(YELLOW "  (v)  View item.\n" NOCOLOR);

		printf(NOCOLOR);

		menu_separator();

		/* Clearing the info line. */
		for (i = 0; i < 80; i++)
			printf(" ");
		back(1);
		printf("\n");

		if (error)
			printf("%s", error);

		back(1);
		printf("\n");

		input = getch();

		back_to_top();
	}

	system("clear");
}

/* vim: set ts=4 sw=4 cc=80 : */
