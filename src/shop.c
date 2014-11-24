#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "shop.h"
#include "term.h"
#include "colors.h"

static char*
stat_color(int i)
{
	if (i == 0)
		return WHITE;
	else if (i > 0)
		return BRIGHT GREEN;
	else
		return BRIGHT RED;
}

static void
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

	printf(BRIGHT BLUE " > Selected item: %s\n" NOCOLOR, item->name);

	/* FIXME: Too many stats means stats are not displayed.
	 *        Also, try to display everything in a much nicer fashion */

	for (list = item->attacks; list; list = list->next)
	{
		Attack* attack = list->data;

		printf(BRIGHT WHITE "    provides a %i-%i %s attack\n" NOCOLOR,
			attack->damage, attack->strikes, type_to_string(attack->type));
	}

	if (item->health_on_use)
		printf("    %s%+i HP\n" NOCOLOR,
			stat_color(item->health_on_use), item->health_on_use);

	if (item->mana_on_use)
		printf("    %s%+i MP\n" NOCOLOR,
			stat_color(item->mana_on_use), item->mana_on_use);

	if (item->health_bonus)
		printf("    %s%+i max health\n" NOCOLOR,
			stat_color(item->health_bonus), item->health_bonus);

	if (item->mana_bonus)
		printf("    %s%+i max mana\n" NOCOLOR,
			stat_color(item->mana_bonus), item->mana_bonus);

	if (item->attack_bonus)
		printf("    %s%+i base attack\n" NOCOLOR,
			stat_color(item->attack_bonus), item->attack_bonus);

	if (item->defense_bonus)
		printf("    %s%+i base defense\n" NOCOLOR,
			stat_color(item->defense_bonus), item->defense_bonus);

	for (i = 0; i < TYPE_MAX; i++)
	{
		if (item->type_resistance[i])
		{
			printf("    %s%+i%% %s resistance\n" NOCOLOR,
				stat_color(item->type_resistance[i]),
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
	Item* old_item;
	int i;

	if (!selected_item)
		return NULL;

	if (get_count_from_inventory(player->inventory, selected_item) > 0)
	{
		slot = selected_item->slot;
		old_item = player->equipment[slot];

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

	printf(BRIGHT BLUE " > Current equipment:\n" NOCOLOR);

	for (i = 0; i < EQ_MAX; i++)
	{
		item = player->equipment[i];

		/* Clearing the area we’re gonna (probably) use */
		for (j = 0; j < 40 - printed; j++)
			printf(" ");
		printf("\n");
		back(1);

		printf(WHITE);
		printed = printf("  - %s: ", equipment_string(i));
		printf(NOCOLOR);
		for (j = 0; j < 22 - printed; j++)
			printf("-");
		printf(" ");

		if (item)
			printf(WHITE "%s\n" NOCOLOR, item->name);
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

	while (input == -42 || !isexit(input))
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
					printf(NOCOLOR "\033[47m");

				if (item->price <= player->gold)
					printf(BRIGHT GREEN);
				else
					printf(BLACK);

				printed = printf("  - %-48s (%s)",
					item->name, equipment_string(item->slot));

				for (j = 0; j < 68 - printed; j++)
					printf(" ");

				printed = printf("(%igp)", item->price);

				for (j = 0; j < 12 - printed; j++)
					printf(" ");

				printf("\n" NOCOLOR);
			}

			i++;
		}

		for (; i < 7; i++)
			printf("\n");

		menu_separator();

		printf(WHITE " Money: %i%-5s" NOCOLOR, player->gold, "gp");
		move(40);
		printf(WHITE " (b)  Buy\n" NOCOLOR);
		if (selected_item)
			printf(WHITE " In inventory: %i" NOCOLOR,
				get_count_from_inventory(player->inventory, selected_item));
		move(40);
		printf(WHITE
			" (e)  Equip\n" NOCOLOR);
		move(40);
		printf(WHITE
			" (s)  Sell\n" NOCOLOR);

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
