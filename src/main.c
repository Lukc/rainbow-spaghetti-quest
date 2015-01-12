#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>

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
#include "craft.h"
#include "skills.h"
#include "parser.h"
#include "characters.h"
#include "saves.h"

static void
print_menu(Game* game)
{
	Place* location = game->location;

	menu_separator();

	if (location->random_enemies)
		printf(WHITE);
	else
		fg(1, 1, 1);
	printf("  (b) Random battle\n" NOCOLOR);

	if (location->shop_items)
		printf(WHITE);
	else
		fg(1, 1, 1);
	printf("  (s) Enter shop\n" NOCOLOR);
	printf(WHITE "  (i) Open inventory\n");
	fg(1, 1, 1);
	printf("  (d) Enter dungeon\n" NOCOLOR);
	printf(WHITE "  (t) Travel\n" NOCOLOR);

	back(5);
	move(40);
	printf(WHITE " (c) Craft items\n" NOCOLOR);
	move(40);
	printf(WHITE " (k) Use skills\n" NOCOLOR);
	move(40);
	if (location->characters)
		printf(WHITE);
	else
		fg(1, 1, 1);
	printf(" (q) Show quests\n" NOCOLOR);
	printf("\n");
	move(40);
	printf(WHITE);
	printf(" (S) Save" NOCOLOR);
	move(53);
	fg(5, 2, 0);
	printf(" (L) Load");
	move(66);
	printf(WHITE);
	printf(" (Q) Quit\n" NOCOLOR);

	menu_separator();
}

int
main(int argc, char* argv[])
{
	Entity player, enemy;
	char* error;
	int input;
	int i;
	Game game;

	memset(&game, 0, sizeof(game));

	load_game(&game, "data");

	/* For now, repeatability would be useful */
	srand(42);

	init_entity_from_class(&player, get_class_by_name(game.classes, "Warrior"));
	printf(" >> Player will be a %s!\n", get_class_by_name(game.classes, "Warrior")->name);

	player.name = (char*) malloc(sizeof(char) * 80);
	snprintf(
		player.name, 14, "%s the Cat",
		argc > 1 ? argv[1] : "Joe"
	);

	game.player = &player;
	game.enemy = &enemy;

	game.location = get_place_by_name(game.places, "Felinopolis");
	game.visited = NULL;

	load(&game, 0);

	getch();

	system("clear");
	input = KEY_CLEAR;
	while (input != 'Q')
	{
		error = NULL;

		check_first_visit(&game);

		back_to_top();

		switch(input)
		{
			case KEY_CLEAR:
				break;
			case 'b':
				enter_battle(&game);
				break;
			case 's':
				if (game.location->shop_items)
					enter_shop(&game);
				else
					error = "There is no shop at your current location!";
				break;
			case 'i':
				inventory(&game);
				break;
			case 't':
				travel(&game);
				break;
			case 'c':
				craft(&game);
				break;
			case 'k':
				skills(&game);
				break;
			case 'q':
				quests(&game);
				break;

			/* Unimplemented stuff */
			case 'S':
				save(&game, 0);
				error = BRIGHT GREEN " >> " WHITE "Game saved!";
				break;
			case 'L':
				load(&game, 0);
				error = BRIGHT GREEN " >> " WHITE "Game loaded! (but stuff is probably broken, now)";
				break;
			case 'd':
				break;
			default:
				error = BRIGHT RED " >> " WHITE "Unrecognized key.";
		}

		player.health = get_max_health(&player);
		player.mana = get_max_mana(&player);
		remove_statuses(&player);

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

			printf(
				"Gold: %i\n"
				"\n"
				,
				player.gold
			);

			printf("Current location: %s\n\n", game.location->name);

			for (i = 0; i < 8; i++)
				printf("\n");
		}

		print_menu(&game);

		printf("%80s", "");
		move(0);
		if (error)
			printf("%s", error);
		back(1);
		printf("\n");

		input = getch();
	}

	system("stty sane");

	/* Auto-save? Should it be removed? */
	save(&game, 0);

	unload_game(&game);

	return 0;
}

/* vim: set ts=4 sw=4 cc=80 : */
