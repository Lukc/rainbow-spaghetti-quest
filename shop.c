#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <readline/readline.h>

#include "colors.h"
#include "items.h"
#include "commands.h"
#include "battle.h"
#include "shop.h"

static int
count_items(Item* items)
{
	int n = 0;

	while (items[n].id)
		n++;

	return n;
}

Logs*
enter_shop(void* opt)
{
	Battle* data;
	Item* items;
	Entity* player;
	char* err = NULL;
	char* line;
	int i;

	int selected_index = -1;

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
			if (get_count_from_inventory(player->inventory, items[selected_index].id) > 0)
			{
				int slot, id;
				int oldItem;
				int i;

				slot = items[selected_index].slot;
				id = items[selected_index].id;
				oldItem = player->equipment[slot];

				for (i = 0; player->inventory[i] != id; i++)
					;;

				player->inventory[i] = player->equipment[slot];
				player->equipment[slot] = id;
			}
			else
			{
				err = "You cannot equip an item you do not possess";
			}
		}
		else if (!strcmp(line, "buy") || !strcmp(line, "b"))
		{
			if (player->caps > items[selected_index].price)
			{
				for (i = 0; i < INVENTORY_SIZE && player->inventory[i] != -1; i++)
					;;

				if (i == INVENTORY_SIZE)
				{
					err = "No space in inventory.";
				}
				else
				{
					player->caps -= items[selected_index].price;
					player->inventory[i] = items[selected_index].id;
				}
			}
			else
			{
				err = "You do not have enough bottlecaps to buy this item.";
			}
		}
		else
		{
			selected_index = atoi(line);

			if (selected_index < 0 || selected_index >= count_items(items))
				selected_index = -1;
		}

		system("clear");

		printf("Current equipment:\n");
		for (i = 0; i < EQ_MAX; i++)
		{
			int j, printed;
			Item* item = get_item_from_id(data, player->equipment[i]);

			printed = printf("  - %s: ", equipment_string(i));
			for (j = 0; j < 20 - printed; j++)
				printf("-");
			printf(" ");

			if (item)
				printf("%s\n", item->name);
			else
				printf("(nothing)\n");
		}
		printf("\n");

		if (selected_index != -1)
		{
			/* FIXME: Print the fucking stats of the item (att/def bonus, …) */
			printf("Selected item: %s\n", items[selected_index].name);
			printf(" Already possessed: %i.\n", get_count_from_inventory(player->inventory, items[selected_index].id));
			printf("\n");
		}

		for (i = 0; i < count_items(items); i++)
		{
			printf(
				WHITE "  - (%i)" NOCOLOR ":  "
				BRIGHT BLUE "%s" NOCOLOR " %s(%i caps)\n" NOCOLOR
				,
				i, items[i].name,
				player->caps >= items[i].price ? BRIGHT GREEN : BRIGHT RED,
				items[i].price
			);
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
