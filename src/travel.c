#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "travel.h"
#include "destinations.h"
#include "term.h"
#include "colors.h"

static void
first_visit(Game* game)
{
	List* list;
	int i;

	for (list = game->location->on_first_visit; list; list = list->next)
	{
		char** image = list->data;

		system("clear");

		for (i = 0; image[i]; i++)
		{
			/* Ugly hack to print stuff on the last line. */
			printf("%s", image[i]);

			if (i != 0)
			{
				back(1);
				printf("\n");
			}

			if (image[i+1])
				printf("\n");
		}

		getch();
	}

	system("clear");
}

/**
 * Fires the first_visit events if needed.
 */
void
check_first_visit(Game* game)
{
	if (!has_visited(game, game->location) &&
		game->location->on_first_visit)
	{
		first_visit(game);

		list_add(&game->visited, game->location);
	}
}

int
can_travel_to(Game* game, Destination* destination)
{
	List* list;
	Item* item;

	for (list = destination->needed_items; list; list = list->next)
	{
		item = list->data;

		if (!get_count_from_inventory(game->player->inventory, item))
			return 0;
	}

	return 1;
}

void
travel(Game* game)
{
	List* list;
	Destination* destination;
	char* info = NULL;
	char input = -42;
	int i;

	system("clear");

	while (!isexit(input))
	{
		if (input == -42)
			;
		else if (isdigit(input))
		{
			input = input - '0';

			if ((destination = list_nth(game->location->destinations, input)))
			{
				if (can_travel_to(game, destination))
				{
					game->location = destination->place;

					check_first_visit(game);

					system("clear");
					return;
				}
				else
				{
					info = "You can't go there... yet!";
				}
			}
			else
			{
				info = "Hey! Invalid input!";
			}
		}
		else
		{
			info = "Hey! Invalid input!";
		}

		printf(BRIGHT BLUE "\n >> Places you can go to:\n\n" NOCOLOR);

		i = 0;
		for (list = game->location->destinations; list; list = list->next)
		{
			destination = list->data;

			if (can_travel_to(game, destination))
				printf(WHITE "  (%i)  %s\n" NOCOLOR,
					i, destination->name);
			else
				printf(BLACK "  (%i)  %s\n" NOCOLOR,
					i, destination->name);

			i++;
		}

		for (; i < 17; i++)
			printf("\n");

		printf("\nPlease select a destination.\n");

		menu_separator();

		if (info)
		{
			printf("%s", info);
			back(1);
			printf("\n");
		}

		input = getch();
		system("clear");
	}

	system("clear");

	return;
}

/* vim: set ts=4 sw=4 cc=80 : */

