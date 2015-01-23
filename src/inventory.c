#include <stdlib.h>
#include <stdio.h>

#include "strings.h"
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
	int viewing_item = 0;
	int printed;
	int i, j, k;

	system("clear");

	while (input != 'l')
	{
		error = NULL;

		if (viewing_item)
			viewing_item = 0;

		if (input == 'e')
		{
			Item* item = player->inventory[selection].item;

			if (item)
			{
				if (item->slot != -1)
				{
					/* FIXME: We only need to clear the top-left part of
					 *        the screen. */
					system("clear");

					equip_item(player, player->inventory[selection].item);
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
		else if (input == 'v')
		{
			if (player->inventory[selection].item)
				viewing_item = 1;
		}
		else if (input == 's')
		{
			if (game->location->shop_items)
			{
				Item* item;

				if ((item = player->inventory[selection].item))
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
			selection = selection < INVENTORY_SIZE - 10 ?
				selection + 10 : selection;
		else if (input == KEY_LEFT)
			selection = selection >= 10 ?
				selection - 10 : selection;

		print_equipment(player);
		printf("\n");

		back_to_top();
		for (i = 0; i < 7; i++)
		{
			move(40);
			printf("│");

			if (i == 0)
			{
				fg(BLUE);
				printf(" >> %s", player->name);
				nocolor();
			}
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

		if (viewing_item)
		{
			print_item(player->inventory[selection].item);

			back_to_top();
			/* This function is total bullshit… how much work does each use
			 * require?*/
			for (i = 0; i < 18; i++)
				printf("\n");
		}
		else
			for (i = 0; i < INVENTORY_SIZE / 3; i++)
			{
				/* Alignment. Ah ah! */
				printf(" ");

				for (k = 0; k < 3; k++)
				{
					if (selection == i + k * 10)
						printf("\033[47m");

					if (player->inventory[i + k * 10].item)
					{
						Item* item = player->inventory[i + k * 10].item;
						int quantity = player->inventory[i + k * 10].quantity;

						if (item->slot >= 0)
							printed = printf("  %s", strcut(item->name, 24));
						else
							printed = printf("  %ix %s", quantity,
								strcut(item->name, quantity < 10 ? 22 : 21));
					}
					else
						printed = printf("  (empty)");

					for (j = 0; j < 26 - printed; j++)
						printf(" ");

					nocolor();
				}

			printf("\n");

			nocolor();
		}

		menu_separator();

		fg(WHITE);
		printf("  (e)  Equip stuff.\n");

		back(1);
		move(40);
		printf("Money:     %-8i\n", player->gold);

		if (!game->location->shop_items)
			fg(BLACK);
		else
			fg(WHITE);
		printf("  (s)  Sell stuff.\n");

		back(1);
		move(40);
		if (game->location->shop_items && player->inventory[selection].item)
		{
			fg(WHITE);
			printf("Sell price %-8i\n",
				player->inventory[selection].item->price * 2 / 3);
		}
		else
		{
			for (i = 0; i < 40; i++)
				printf(" ");
			printf("\n");
		}

		fg(WHITE);
		printf("  (l)  Leave\n");

		back(1);
		move(40);
		printf("  (v)  View item.\n");

		nocolor();

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
