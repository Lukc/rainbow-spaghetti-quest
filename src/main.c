#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>

#include "types.h"
#include "commands.h"
#include "colors.h"
#include "entities.h"
#include "items.h"
#include "battle.h"
#include "game.h"
#include "shop.h"
#include "places.h"
#include "term.h"
#include "inventory.h"
#include "travel.h"
#include "images.h"

static void
print_menu(Game* game)
{
	Place* location = game->location;

	menu_separator();

	printf(
		WHITE
		"%s  (b)  Find someone to beat to death!\n" NOCOLOR
		"%s  (s)  Buy new equipment to improve your stats!\n" NOCOLOR
		WHITE "  (i)  Choose your katanas and golden armors!\n"
		BLACK
			"  (d)  Enter a terrible dungeon and fight hordes of enemies!\n"
		WHITE
		"  (t)  Travel to far far away places and explore the world!\n"
		NOCOLOR,
		location->random_enemies ? WHITE : BLACK,
		location->shop_items ? WHITE : BLACK
	);

	menu_separator();
}

int
main(int argc, char* argv[])
{
	Entity player, enemy;
	char* error;
	int input;
	int i;
	Logs* logs;
	List* classes;
	List* items;
	Game game;
	List* world;
	
	items = load_items("items");
	game.items = items;

	classes = load_classes(&game, "classes");
	game.classes = classes;

	world = load_places(&game, "places");

	/* For now, repeatability would be useful */
	srand(42);

	init_entity_from_class(&player, get_class_by_name(classes, "Warrior"));

	player.name = (char*) malloc(sizeof(char) * 80);
	snprintf(
		player.name, 14, "%s the Cat",
		argc > 1 ? argv[1] : "Joe"
	);

	game.player = &player;
	game.enemy = &enemy;

	game.location = get_place_by_name(world, "Felinopolis");
	game.visited = NULL;

	system("clear");
	input = -42;
	while (input == -42 || !isexit(input))
	{
		error = NULL;

		check_first_visit(&game);

		back_to_top();

		switch(input)
		{
			case 'b':
				logs = enter_battle(&game);
				break;
			case 's':
				if (game.location->shop_items)
					logs = enter_shop(&game);
				else
					error = "There is no shop at your current location!";
				break;
			case 'i':
				logs = NULL;
				inventory(&game);
				break;
			case 'd':
				logs = NULL;
				break;
			case 't':
				logs = NULL;
				travel(&game);
				break;
			case -42:
				break;
			default:
				error = "Unrecognized key.";
		}

		/*logs = execute_commands(line, commands, &game);*/
		logs = NULL;

		player.health = get_max_health(&player);
		player.mana = get_max_mana(&player);

		if (game.location->image)
		{
			int out_of_boundaries = 0;
			char** image = game.location->image;

			for (i = 0; i < 16; i++)
				if (!out_of_boundaries && image[i])
					printf("%s\n", image[i]);
				else
				{
					out_of_boundaries = 1;

					printf("\n");
				}
		}
		else
		{
			print_entity_basestats(&player);
			printf("\n");

			if (logs)
			{
				logs_print(logs);
				logs_free(logs);
			}

			printf(
				"Bottle caps: %i\n"
				"\n"
				,
				player.caps
			);

			printf("Current location: %s\n\n", game.location->name);

			for (i = 0; i < 8; i++)
				printf("\n");
		}

		print_menu(&game);

		if (error)
		{
			printf("%s", error);
			back(1);
			printf("\n");
		}

		input = getch();
	}

	system("stty sane");

	return 0;
}

/* vim: set ts=4 sw=4 cc=80 : */
