#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "craft.h"
#include "colors.h"
#include "term.h"
#include "recipe.h"
#include "shop.h"

static int
recipe_craftable(Game* game, Recipe* recipe)
{
	ItemStack* inventory = game->player->inventory;
	List* l;

	for (l = recipe->ingredients; l; l = l->next)
	{
		Ingredient* ig = l->data;

		if (get_count_from_inventory(inventory, ig->item) < ig->quantity)
			return 0;
	}

	return 1;
}

/**
 * Assuming all necessary resources are in the player’s inventory.
 */
static void
craft_item(Game* game, Recipe* recipe)
{
	Entity* player = game->player;
	List* l;

	for (l = recipe->ingredients; l; l = l->next)
	{
		Ingredient* ig = l->data;

		remove_items(player, ig->item, ig->quantity);
	}

	give_item(player, recipe->output);
}

static List*
available_recipes(Game* game)
{
	List* out = NULL;
	List* l;

	for (l = game->recipes; l; l = l->next)
	{
		Recipe* recipe = l->data;

		if (recipe_craftable(game, recipe))
		{
			list_add(&out, recipe);
		}
	}

	return out;
}

void
craft(Game* game)
{
	int input = -42;
	char* log;
	List* recipes;
	List* l;
	Item* selected_item = NULL;
	Recipe* selected_recipe = NULL;
	int selected_index = -1;
	int i;

	recipes = available_recipes(game);

	if (recipes)
	{
		selected_recipe = recipes->data;
		selected_item = selected_recipe->output;
		selected_index = 0;
	}

	system("clear");

	while (input != 'l')
	{
		log = NULL;

		switch (input)
		{
			case -42:
			case KEY_CLEAR:
				break;
			case KEY_UP:
				if (recipes != NULL)
				{
					selected_index = selected_index > 0 ?
						selected_index - 1 : selected_index;
					selected_recipe = list_nth(recipes, selected_index);
					selected_item = selected_recipe->output;
				}
				break;
			case KEY_DOWN:
				if (recipes != NULL)
				{
					selected_index = selected_index < list_size(recipes) - 1 ?
						selected_index + 1 : selected_index;
					selected_recipe = list_nth(recipes, selected_index);
					selected_item = selected_recipe->output;
				}
				break;
			case 'b':
				if (selected_recipe)
				{
					craft_item(game, selected_recipe);

					log = malloc(sizeof(char) * 128);
					snprintf(log, 128,
						BRIGHT GREEN " >> " WHITE
							"Successfully crafted a %s!" NOCOLOR,
						selected_recipe->output->name
					);

					list_free(recipes);
					recipes = available_recipes(game);

					if (recipes)
					{
						selected_index = selected_index < list_size(recipes) ?
							selected_index : list_size(recipes) - 1 ;
						selected_recipe = list_nth(recipes, selected_index);
						selected_item = selected_recipe->output;
					}
					else
					{
						selected_item = NULL;
						selected_recipe = NULL;
						selected_index = -1;

						system("clear");
					}
				}
				break;
			default:
				log = strdup("Unrecognized key.");
		}

		back_to_top();

		for (i = 0; i < 10; i++)
			printf("%80s\n", "");

		if (selected_item)
		{
			back_to_top();

			print_item(selected_item);

			back_to_top();

			for (i = 0; i < 10; i++)
				printf("\n");
		}

		menu_separator();

		l = recipes;
		if (selected_index >= 8)
			for (i = 0; i < (selected_index - selected_index % 8); i++)
				l = l->next;
	
		if (l)
		{
			for (i = 0; i < 8; i++)
			{
				if (l)
				{
					Recipe* recipe = l->data;

					if (i == selected_index % 8)
						printf("\033[47m" BLACK);
					else
						printf(WHITE);

					printf(" %-39s\n" NOCOLOR , recipe->output->name);

					l = l->next;
				}
				else
					printf("%40s\n", "");
			}

			back(8);

			l = selected_recipe->ingredients;
			for (i = 0; i < 8; i++)
			{
				move(40);

				printf("│ ");

				if (l)
				{
					Ingredient* ig = l->data;

					printf(WHITE);

					if (ig->quantity == 1)
						printf("%-38s\n", ig->item->name);
					else
					{
						int n, j;
						n = printf("%s (x%i)", ig->item->name, ig->quantity);
						for (j = n; j < 38; j++)
							printf(" ");
						printf("\n");
					}

					printf(NOCOLOR);

					l = l->next;
				}
				else
					printf("%38s\n", "");
			}
		}
		else
		{
			printf("%40s\n%-40s\n", "", "  No recipe!");
			for (i = 0; i < 6; i++)
				printf("%40s\n", "");
		}

		menu_separator();

		move(40);
		if (selected_item)
			printf(WHITE);
		else
			printf(BLACK);
		printf(" (b) Build\n" NOCOLOR);
		move(40);
		printf(WHITE " (l) Leave\n" NOCOLOR);

		menu_separator();

		printf("%-80s", "");
		back(1);
		if (log)
		{
			printf("\n%s", log);
			back(1);
			free(log);
		}
		printf("\n");

		input = getch();
	}

	list_free(recipes);

	system("clear");
}

/* vim: set ts=4 sw=4 : */
