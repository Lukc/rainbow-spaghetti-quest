#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <readline/readline.h>

#include "colors.h"
#include "items.h"
#include "commands.h"
#include "battle.h"
#include "shop.h"

static void
print_item(Item* item)
{
	int i;

	printf("Selected item: %s\n", item->name);

	if (item->attack_bonus)
		printf("  %i base attack\n", item->attack_bonus);

	if (item->defense_bonus)
		printf("  %i base defense\n", item->defense_bonus);

	for (i = 0; i < TYPE_MAX; i++)
	{
		if (item->type_resistance[i])
		{
			printf("  %i%% %s resistance\n", item->type_resistance[i], type_string(i));
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
		return "You cannot equip an item you do not possess";
	}

	return NULL;
}

static void
print_equipment(Battle* data, Entity* player)
{
	int i, j, printed;
	Item* item;

	printf("Current equipment:\n");

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
		else if (!strcmp(line, "buy") || !strcmp(line, "b"))
		{
			err = buy_item(player, selected_item);
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
				"  Already possessed: %i.\n\n",
				get_count_from_inventory(player->inventory, item->id));
		}

		i = 0;
		for (list = items; list; list = list->next)
		{
			item = (Item*) list->data;

			printf(
				WHITE "  - (%i)" NOCOLOR ":  "
				BRIGHT BLUE "%s" NOCOLOR " %s(%i caps)\n" NOCOLOR
				,
				i, item->name,
				player->caps >= item->price ? BRIGHT GREEN : BRIGHT RED,
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
