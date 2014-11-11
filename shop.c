#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <readline/readline.h>

#include "colors.h"
#include "items.h"
#include "commands.h"
#include "battle.h"
#include "shop.h"

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
	int i;

	printf(BRIGHT BLUE " > Selected item: %s\n" NOCOLOR, item->name);

	if (item->slot == EQ_WEAPON)
		printf(WHITE "    is a %s weapon\n" NOCOLOR,
			type_string(item->attack_type));
	else
		printf(WHITE "    is a %s\n" NOCOLOR, equipment_string(item->slot));

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

	if (player->caps >= selected_item->price)
	{
		for (i = 0; i < INVENTORY_SIZE && player->inventory[i] != -1; i++)
			;;

		if (i == INVENTORY_SIZE)
		{
			/* FIXME: This sucks... */
			return "No space left in inventory.";
		}
		else
		{
			player->caps -= selected_item->price;
			player->inventory[i] = selected_item->id;
		}
	}
	else
	{
		return "You do not have enough bottlecaps to buy this item.";
	}

	return NULL;
}

static char*
equip_item(Entity* player, Item* selected_item)
{
	int slot, id;
	int oldItem;
	int i;

	if (get_count_from_inventory(player->inventory, selected_item->id) > 0)
	{
		slot = selected_item->slot;
		id = selected_item->id;
		oldItem = player->equipment[slot];

		for (i = 0; player->inventory[i] != id; i++)
			;;

		player->inventory[i] = player->equipment[slot];
		player->equipment[slot] = id;
	}
	else
	{
		return "You cannot equip an item you do not possess...";
	}

	return NULL;
}

static char*
unequip_item(Entity* player, Item* item)
{
	int i;

	if (player->equipment[item->slot] == item->id)
	{
		for (i = 0; i < INVENTORY_SIZE && player->inventory[i] != 0; i++)
			;;

		if (i == INVENTORY_SIZE)
		{
			return "No space left in inventory!";
		}

		player->inventory[i] = player->equipment[item->slot];
		player->equipment[item->slot] = 0;
	}
	else
	{
		return "You cannot unequip an item that is not equiped...";
	}

	return NULL;
}

static void
print_equipment(Battle* data, Entity* player)
{
	int i, j, printed;
	Item* item;

	printf(BRIGHT BLUE " > Current equipment:\n" NOCOLOR);

	for (i = 0; i < EQ_MAX; i++)
	{
		item = get_item_from_id(data, player->equipment[i]);

		printf(WHITE);
		printed = printf("  - %s: ", equipment_string(i));
		printf(NOCOLOR);
		for (j = 0; j < 20 - printed; j++)
			printf("-");
		printf(" ");

		if (item)
			printf(WHITE "%s\n" NOCOLOR, item->name);
		else
			printf("(nothing)\n");
	}

	printf("\n");
}

static char*
sell_item(Entity* player, Item* item)
{
	int i;
	int count = get_count_from_inventory(player->inventory, item->id);

	if (count < 1)
	{
		return "You cannot sell an item you do not possess...";
	}

	for (i = 0; i < INVENTORY_SIZE && player->inventory[i] != item->id; i++)
		;;

	player->inventory[i] = 0;

	player->caps += 2 * item->price / 3;

	return NULL;
}

Logs*
enter_shop(void* opt)
{
	Battle* data;
	List* items, * list;
	Entity* player;
	char* err = NULL;
	char* line;
	int i;

	Item* selected_item = NULL;
	Item* item;

	data = opt;
	items = data->items;
	player = data->player;

	line = strdup("");
	while (line)
	{
		if (!strcmp(line, "quit"))
		{
			free(line);
			exit(0);
		}
		else if (!strcmp(line, "equip") || !strcmp(line, "e"))
		{
			err = equip_item(player, selected_item);
		}
		else if (!strcmp(line, "unequip") || !strcmp(line, "u"))
		{
			err = unequip_item(player, selected_item);
		}
		else if (!strcmp(line, "buy") || !strcmp(line, "b"))
		{
			err = buy_item(player, selected_item);
		}
		else if (!strcmp(line, "sell") || !strcmp(line, "s"))
		{
			err = sell_item(player, selected_item);
		}
		else
		{
			int input = atoi(line);

			if (input < 0)
				err = "Invalid index!";

			list = items;
			for (i = 0; i < input && list; i++)
			{
				list = list->next;
			}

			if (!list)
				err = "Invalid index!";
			else
				selected_item = (Item*) list->data;
		}

		system("clear");

		print_equipment(data, player);

		if (selected_item)
		{
			print_item(selected_item);
			printf(
				WHITE " > Already possessed: %i.\n\n" NOCOLOR,
				get_count_from_inventory(player->inventory, selected_item->id));
		}

		printf(WHITE " > Items sold:\n" NOCOLOR);
		i = 0;
		for (list = items; list; list = list->next)
		{
			item = (Item*) list->data;

			printf(
				WHITE "  - (%i)" NOCOLOR ":  %s%-30s" NOCOLOR " %s(%i caps)\n" NOCOLOR
				,
				i,
				player->caps >= item->price ? BRIGHT BLUE : RED,
				item->name,
				player->caps >= item->price ? BRIGHT GREEN : RED,
				item->price
			);

			i++;
		}
		printf("\n");

		if (err)
		{
			printf("%s\n\n", err);
		}

		printf(
			"Options:\n"
			WHITE
			"  - (#) (id of item to examine)\n"
			"  - (b) buy:       Buy the examined item.\n"
			"  - (e) equip:     Equip the examined item (if possessed).\n"
			"  - (u) unequip:   "
				"Remove the examined item from your equipment (if equiped).\n"
			"  - (s) sell:      Sell the examined item (if possessed).\n"
			NOCOLOR
			"\n"
		);

		err = NULL;
		free(line);

		line = readline(" >> ");
	}

	system("clear");
	free(line);

	return NULL;
}

/* vim: set ts=4 sw=4 cc=80 : */
