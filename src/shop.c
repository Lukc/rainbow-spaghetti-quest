#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "shop.h"
#include "term.h"
#include "colors.h"

static int
stat_color(int i)
{
	if (i == 0)
		return WHITE;
	else if (i > 0)
		return GREEN;
	else
		return RED;
}

void
print_item(Item* item)
{
	int i, l;
	List* list;

	for (l = 0; l < 10; l++)
	{
		for (i = 0; i < 80; i++)
			printf(" ");
		printf("\n");
	}
	back(10);

	fg(BLUE);
	printf(" > Selected item: %s\n", item->name);

	/* FIXME: Too many stats means stats are not displayed.
	 *        Also, try to display everything in a much nicer fashion */

	for (list = item->attacks; list; list = list->next)
	{
		Attack* attack = list->data;

		if (attack->strikes > 0)
		{
			fg(WHITE);
			printf("    provides a (%i-%i)x%i %s attack\n",
				attack->damage.min, attack->damage.max, attack->strikes,
				type_to_string(attack->type));
		}
		else
		{
			fg(WHITE);
			printf("    provides a support attack\n");
		}
	}

	if (item->consumable)
	{
		fg(WHITE);
		printf("    is consumed after use\n");
	}

	if (item->on_use)
	{
		if (item->on_use->strikes)
		{
			fg(WHITE);
			printf("    inflicts a (%i-%i)x%i %s attack\n",
				item->on_use->damage.min, item->on_use->damage.max,
				item->on_use->strikes, type_to_string(item->on_use->type));
		}

		if (item->on_use->health)
		{
			fg(stat_color(item->on_use->health));
			printf("    %+i HP\n", item->on_use->health);
		}

		if (item->on_use->mana)
		{
			fg(stat_color(item->on_use->mana));
			printf("    %+i MP\n", item->on_use->mana);
		}
	}

	if (item->health_bonus)
	{
		fg(stat_color(item->health_bonus));
		printf("    %+i max health\n", item->health_bonus);
	}

	if (item->mana_bonus)
	{
		fg(stat_color(item->mana_bonus));
		printf("    %+i max mana\n", item->mana_bonus);
	}

	if (item->attack_bonus)
	{
		fg(stat_color(item->attack_bonus));
		printf("    %+i base attack\n", item->attack_bonus);
	}

	if (item->defense_bonus)
	{
		fg(stat_color(item->defense_bonus));
		printf("    %+i base defense\n", item->defense_bonus);
	}

	for (i = 0; i < TYPE_MAX; i++)
	{
		if (item->type_resistance[i])
		{
			fg(stat_color(item->type_resistance[i]));
			printf("    %+i%% %s resistance\n",
				item->type_resistance[i], type_to_string(i));
		}
	}

	printf("\n");
}

static char*
buy_item(Entity* player, Item* selected_item)
{
	if (!selected_item)
		return NULL;

	if (player->gold >= selected_item->price)
	{
		switch (give_item(player, selected_item))
		{
			case -1:
				return "No space for item in inventory!";
			default:
				player->gold -= selected_item->price;
		}
	}
	else
	{
		return "You do not have enough bottlegold to buy this item.";
	}

	return NULL;
}

char*
equip_item(Entity* player, Item* selected_item)
{
	int slot;
	int i;

	if (!selected_item)
		return NULL;

	if (get_count_from_inventory(player->inventory, selected_item) > 0)
	{
		slot = selected_item->slot;

		for (i = 0; player->inventory[i].item != selected_item; i++)
			;;

		player->inventory[i].item = player->equipment[slot];
		player->equipment[slot] = selected_item;
	}
	else
	{
		return "You cannot equip an item you do not possess...";
	}

	return NULL;
}

char*
unequip_item(Entity* player, Item* item)
{
	int i;

	if (!item)
		return NULL;

	if (player->equipment[item->slot] == item)
	{
		for (i = 0; i < INVENTORY_SIZE && player->inventory[i].item; i++)
			;

		if (i == INVENTORY_SIZE)
		{
			return "No space left in inventory!";
		}

		player->inventory[i].item = player->equipment[item->slot];
		player->inventory[i].quantity = 1;
		player->equipment[item->slot] = NULL;
	}
	else
	{
		return "You cannot unequip an item that is not equiped...";
	}

	return NULL;
}

char*
sell_item(Entity* player, Item* item)
{
	int i;

	if (!item)
		return NULL;

	for (i = 0; i < INVENTORY_SIZE && player->inventory[i].item != item; i++)
		;

	if (i == INVENTORY_SIZE)
		return "You cannot sell an item you do not possess...";

	player->inventory[i].quantity -= 1;
	if (player->inventory[i].quantity <= 0)
		player->inventory[i].item = NULL;

	player->gold += 2 * item->price / 3;

	return NULL;
}

void
print_equipment(Entity* player)
{
	int i, j, printed;
	Item* item;

	fg(BLUE);
	printf(" > Current equipment:\n");

	for (i = 0; i < EQ_MAX; i++)
	{
		item = player->equipment[i];

		/* Clearing the area we’re gonna (probably) use */
		for (j = 0; j < 40 - printed; j++)
			printf(" ");
		printf("\n");
		back(1);

		fg(WHITE);
		printed = printf("  - %s: ", equipment_string(i));
		nocolor();
		for (j = 0; j < 22 - printed; j++)
			printf("-");
		printf(" ");

		if (item)
		{
			fg(WHITE);
			printf("%s\n", item->name);
		}
		else
			printf("(nothing)\n");
	}
}

Logs*
enter_shop(Game* game)
{
	List* items, * list;
	Entity* player;
	char* err = NULL;
	char input = -42;
	int selection = 0;
	int i;

	Item* selected_item;
	Item* item;

	system("clear");

	items = game->location->shop_items;
	player = game->player;

	selected_item = items->data;

	/* Shouldn’t happen, making sure anyway. */
	if (!items)
		return NULL;

	while (input == -42 || input != 'l')
	{
		err = NULL;

		switch (input)
		{
			case 'e':
				err = equip_item(player, selected_item);
				break;
			/*
			 * WHO’d want to do that? :ooo
			case 'u':
				err = unequip_item(player, selected_item);
				break;
			*/
			case 'b':
				err = buy_item(player, selected_item);
				break;
			case 's':
				err = sell_item(player, selected_item);
				break;
			case KEY_DOWN:
				selection = selection >= list_size(items) - 1 ?
					selection : selection + 1;
				selected_item = list_nth(items, selection);
				break;
			case KEY_UP:
				selection = selection <= 0 ?
					selection : selection - 1;
				selected_item = list_nth(items, selection);
				break;
			case -42:
				break;
			default:
				if (isdigit(input))
				{
					input = input - '0';

					if (input < 0)
						err = "Invalid index!";
					else if (input >= list_size(items))
							err = "Invalid index!";
					else
					{
						selection = input;
						selected_item = list_nth(items, input);
					}
				}
				else
				{
					err = "Unrecognized key.";
				}
		}

		back_to_top();

		print_item(selected_item);

		back_to_top();

		for (i = 0; i < 10; i++)
			printf("\n");

		menu_separator();

		i = 0;
		for (list = items; list; list = list->next)
		{
			int printed, j;

			if (selection >= 7 ?
				i > selection - 7 && i <= selection : i < 7)
			{
				item = (Item*) list->data;

				if (i == selection)
				{
					bg(WHITE);
					fg(BLACK);
				}

				if (item->price <= player->gold)
					fg(GREEN);
				else if (get_count_from_inventory(player->inventory, item))
					/* Sellable. That’s an action, let’s not print the entry
					 * as “disabled black”.*/
					fg(YELLOW);
				else
					fg(BLACK);

				printed = printf("  - %-48s (%s)",
					item->name, item->slot == EQ_WEAPON_RANGED ?
						"ranged" : equipment_string(item->slot));

				for (j = 0; j < 68 - printed; j++)
					printf(" ");

				printed = printf("(%igp)", item->price);

				for (j = 0; j < 12 - printed; j++)
					printf(" ");

				printf("\n");
			}

			i++;
		}

		for (; i < 7; i++)
			printf("\n");

		menu_separator();

		fg(WHITE);
		printf(" Money: %i%-5s", player->gold, "gp");
		move(40);
		printf(" (b)  Buy\n");
		if (selected_item)
		{
			fg(WHITE);
			printf(" In inventory: %i",
				get_count_from_inventory(player->inventory, selected_item));
		}
		move(40);
		fg(WHITE);
		printf(" (e)  Equip\n");
		move(40);
		printf("%-20s%-20s\n", " (s)  Sell", " (l)  Leave");

		menu_separator();

		if (err)
		{
			printf("%s", err);
			back(1);
			printf("\n");
		}

		err = NULL;

		input = getch();
	}

	system("clear");

	return NULL;
}

/* vim: set ts=4 sw=4 cc=80 : */
