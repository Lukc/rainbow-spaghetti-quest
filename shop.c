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

char**
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

			if (selected_index < 0 || selected_index > count_items(items))
				selected_index = -1;
		}

		system("clear");

		printf("Current equipment:\n");
		for (i = 0; i < EQ_MAX; i++)
		{
			printf("  - %s: %i\n", equipment_string(i), player->equipment[i]);
		}
		printf("\n");

		if (selected_index != -1)
		{
			/* FIXME: Print the fucking stats of the item (att/def bonus, …) */
			printf("Selected item: %s\n", items[selected_index].name);
			printf(" Already possessed: %i.\n", -1);
			printf("\n");
		}

		for (i = 0; i < count_items(items); i++)
		{
			printf(" - %i:  %s (%i caps)\n", i, items[i].name, items[i].price);
		}
		printf("\n");

		if (err)
		{
			printf("%s\n\n", err);
		}

		printf(
			"Options:\n"
			WHITE
			"  - (#) (number of item to examine)\n"
			"  - (b) buy:   Buy the examined item.\n"
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
