#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "craft.h"
#include "colors.h"
#include "term.h"

/* FIXME: Been missing a few characters :((( */
static void
line(int first)
{
	int x;

	if (first == 1)
		printf(" ");
	else if (first == -1)
		printf(" ");
	else
		printf("├");

	if (first == 1)
		for (x = 0; x < CRAFT_MAX_X - 1; x++)
			printf("────┬");
	else if (first == -1)
		for (x = 0; x < CRAFT_MAX_X - 1; x++)
			printf("────┴");
	else
		for (x = 0; x < CRAFT_MAX_X - 1; x++)
			printf("────┼");

	printf("────");

	if (first == 1)
		printf(" ");
	else if (first == -1)
		printf(" ");
	else
		printf("┤");

	printf("\n");
}

/**
 * Writes about 11=1+2*5 lines.
 */
static void
display_grid(CraftingGrid* grid, int x, int y)
{
	int i, j;

	for (j = 0; j < CRAFT_MAX_Y; j++)
	{
		line(j == 0);

		printf("│");

		for (i = 0; i < CRAFT_MAX_X; i++)
		{
			if (x == i && y == j)
				printf(BLACK "\033[47m");

			if (grid->items[i][j])
				printf(" ++ ");
			else
				printf("    ");

			if (x == i && y == j)
				printf(NOCOLOR);

			printf("│");
		}

		printf("\n");
	}

	line(-1);
}

void
craft(Game* game)
{
	CraftingGrid grid;
	int input = -42;
	int page = 0;
	char* error;
	int x, y;

	for (x = 0; x < CRAFT_MAX_X; x++)
		for (y = 0; y < CRAFT_MAX_Y; y++)
			grid.items[x][y] = NULL;

	x = CRAFT_MAX_X / 2;
	y = CRAFT_MAX_Y / 2;

	system("clear");

	while (!isexit(input))
	{
		error = NULL;

		switch (input)
		{
			case -42:
				break;
			case KEY_UP:
				y = y > 0 ? y - 1 : 0;
				break;
			case KEY_DOWN:
				y = y < CRAFT_MAX_Y - 1 ? y + 1 : CRAFT_MAX_Y - 1;
				break;
			case KEY_LEFT:
				x = x > 0 ? x - 1 : 0;
				break;
			case KEY_RIGHT:
				x = x < CRAFT_MAX_X - 1 ? x + 1 : CRAFT_MAX_X - 1;
				break;
			case '+':
				page = page >= INVENTORY_SIZE / 5 ? page : page + 1;
				break;
			case '-':
				page = page == 0 ? 0 : page - 1;
				break;
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
				input = input - '1';
				grid.items[x][y] =
					game->player->inventory[page * 5 + input].item;
				break;
			default:
				error = "Unrecognized key.";
		}

		back_to_top();

		menu_separator();

		display_grid(&grid, x, y);

		menu_separator();

		print_items_menu(game->player, page);

		back(5);
		move(40);
		printf("  (+) next\n");
		move(40);
		printf("  (-) previous\n");
		printf("\n\n\n");

		menu_separator();

		if (error)
		{
			printf("%s", error);
			back(1);
			printf("\n");
		}

		input = getch();
	}

	system("clear");
}

/* vim: set ts=4 sw=4 : */
