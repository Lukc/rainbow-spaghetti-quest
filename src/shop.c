#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "colors.h"
#include "items.h"
#include "commands.h"
#include "battle.h"
#include "shop.h"
#include "term.h"

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

	printf(WHITE "    is a %s\n" NOCOLOR, equipment_string(item->slot));

	for (list = item->attacks; list; list = list->next)
	{
		Attack* attack = list->data;

		printf(BRIGHT WHITE "    provides a %i-%i %s attack\n" NOCOLOR,
			attack->damage, attack->strikes, type_string(attack->type));
	}

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
				item->type_resistance[i], type_string(i));
		}
	}

	printf("\n");
}

static char*
buy_item(Entity* player, Item* selected_item)
{
	int i;

	if (!selected_item)
		return NULL;

	if (player->caps >= selected_item->price)
	{
		for (i = 0; i < INVENTORY_SIZE && player->inventory[i]; i++)
			;;

		if (i == INVENTORY_SIZE)
		{
			/* FIXME: This sucks... */
			return "No space left in inventory.";
		}
		else
		{
			player->caps -= selected_item->price;
			player->inventory[i] = selected_item;
		}
	}
	else
	{
		return "You do not have enough bottlecaps to buy this item.";
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

		for (i = 0; player->inventory[i] != selected_item; i++)
			;;

		player->inventory[i] = player->equipment[slot];
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
		for (i = 0; i < INVENTORY_SIZE && player->inventory[i]; i++)
			;;

		if (i == INVENTORY_SIZE)
		{
			return "No space left in inventory!";
		}

		player->inventory[i] = player->equipment[item->slot];
		player->equipment[item->slot] = NULL;
	}
	else
	{
		return "You cannot unequip an item that is not equiped...";
	}

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

char*
sell_item(Entity* player, Item* item)
{
	int i;
	int count;

	if (!item)
		return NULL;

	count = get_count_from_inventory(player->inventory, item);

	if (count < 1)
	{
		return "You cannot sell an item you do not possess...";
	}

	for (i = 0; i < INVENTORY_SIZE && player->inventory[i] != item; i++)
		;;

	player->inventory[i] = NULL;

	player->caps += 2 * item->price / 3;

	return NULL;
}

Logs*
enter_shop(Battle* data)
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

	items = data->location->shop_items;
	player = data->player;

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
					printf("\033[47m");

				if (item->price <= player->caps)
					fg(1, 5, 0);
				else
					fg(1, 1, 1);

				printed = printf("  - %s", item->name);

				for (j = 0; j < 60 - printed; j++)
					printf(" ");

				printed = printf("%i",
					get_count_from_inventory(player->inventory, item));

				for (j = 0; j < 8 - printed; j++)
					printf(" ");

				printed = printf("(%ic)", item->price);

				for (j = 0; j < 12 - printed; j++)
					printf(" ");

				printf("\n" NOCOLOR);
			}

			i++;
		}

		for (; i < 7; i++)
			printf("\n");

		menu_separator();

		printf(WHITE " Money: %i" NOCOLOR, player->caps);
		move(40);
		printf(WHITE " (b)  Buy\n" NOCOLOR);
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