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
#include "shop.h"
#include "places.h"
#include "term.h"
#include "inventory.h"

Logs*
travel(void* opt)
{
	Battle* data = opt;
	List* list;
	char input = -42;
	int i;

	system("clear");

	while (!isexit(input))
	{
		printf("\nPlaces you can go to:\n");

		i = 0;
		for (list = data->location->destinations; list; list = list->next)
		{
			printf(WHITE " (%i)  %s\n" NOCOLOR,
				i, ((Place*) list->data)->name);

			i++;
		}

		printf("\nPlease select a destination.\n");

		if (input == -42)
			;
		else if (isdigit(input))
		{
			Place* place;

			input = input - '0';

			if ((place = list_nth(data->location->destinations, input)))
			{
				/* If not, we’ll be damn screwed... */
				if (place)
					data->location = place;

				system("clear");
				return NULL;
			}
			else
			{
				printf("Hey! Invalid input!\n");
			}
		}
		else
		{
			printf("Hey! Invalid input!\n");
		}

		input = getch();
		system("clear");
	}

	system("clear");

	return NULL;
}

static void
print_menu(Battle* data)
{
	Place* location = data->location;

	menu_separator();

	printf(
		WHITE
		"%s  (b)  Find someone to beat to death!\n" NOCOLOR
		"%s  (s)  Buy new equipment to improve your stats!\n" NOCOLOR
		WHITE "  (i)  Choose your katanas and golden armors!\n"
		YELLOW
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
	Battle battle;
	List* world;
	
	items = load_items("items");
	battle.items = items;

	classes = load_classes(&battle, "classes");
	battle.classes = classes;

	world = load_places(&battle, "places");

	/* For now, repeatability would be useful */
	srand(42);

	init_entity_from_class(&player, get_class_by_name(classes, "Warrior"));

	player.name = (char*) malloc(sizeof(char) * 80);
	snprintf(
		player.name, 80, "%s the %s",
		argc > 1 ? argv[1] : "Kaleth", player.class->name
	);

	battle.player = &player;
	battle.enemy = &enemy;

	battle.location = get_place_by_name(world, "Felinopolis");

	system("clear");
	input = -42;
	while (input == -42 || !isexit(input))
	{
		error = NULL;

		back_to_top();

		switch(input)
		{
			case 'b':
				logs = enter_battle((void*) &battle);
				break;
			case 's':
				if (battle.location->shop_items)
					logs = enter_shop(&battle);
				else
					error = "There is no shop at your current location!";
				break;
			case 'i':
				logs = NULL;
				inventory(&battle);
				break;
			case 'd':
				logs = NULL;
				break;
			case 't':
				logs = travel((void*) &battle);
				break;
			case -42:
				break;
			default:
				error = "Unrecognized key.";
		}

		/*logs = execute_commands(line, commands, &battle);*/
		logs = NULL;

		player.health = get_max_health(&player);
		player.mana = get_max_mana(&player);

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

		printf("Current location: %s\n\n", battle.location->name);

		for (i = 0; i < 8; i++)
			printf("\n");

		print_menu(&battle);

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
