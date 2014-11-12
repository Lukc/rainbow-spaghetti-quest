#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include <readline/readline.h>

#include "types.h"
#include "commands.h"
#include "colors.h"
#include "entities.h"
#include "items.h"
#include "battle.h"
#include "shop.h"
#include "places.h"

Logs*
travel(void* opt)
{
	Battle* data = opt;
	List* list;
	char* input;
	int i;

	printf("\nPlaces you can go to:\n");

	i = 0;
	for (list = data->location->destinations; list; list = list->next)
	{
		printf("  <%i>  %s\n", i, ((Place*) list->data)->name);
		i++;
	}

	printf("\nPlease select a destination.\n");

	while ((input = readline(">> ")))
	{
		if (isdigit(input[0]))
		{
			Place* place;

			if ((place = list_nth(data->location->destinations, atoi(input))))
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
	}

	system("clear");

	return NULL;
}

Logs*
inventory(void* opt)
{
	Battle* data = opt;
	List* list;
	Entity* player = data->player;
	int i;

	print_equipment(data, player);

	for (i = 0; i < INVENTORY_SIZE; i++)
	{
		if (player->inventory[i])
		{
			Item* item = player->inventory[i];

			printf(" - %2i  %s\n", i, item->name);
		}
	}

	while (getchar() > 0)
		;;

	return NULL;
}

int
main(int argc, char* argv[])
{
	Entity player, enemy;
	char* line;
	Logs* logs;
	Command commands[] = {
		{"battle",  "b", enter_battle, "Beat a random enemy to death!"},
		{"shop",    "s", enter_shop,   "Buy new equipment to improve your stats!"},
		{"inventory", "i", inventory,  "Craft thingies or sell old equipment!"},
		{"dungeon", "d", NULL,         "Enter a terrible dungeon and fight hordes of enemies!"},
		{"travel",  "t", travel,       "Travel to other places and explore the world!"},
		{NULL, NULL, NULL, NULL}
	};
	List* classes;
	List* items;
	Battle battle;
	List* world;

	classes = load_classes("classes");
	items = load_items("items");

	battle.classes = classes;
	battle.items = items;

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

	/* WHAT? The FIRST place found? :o */
	battle.location = world->data;

	line = strdup("");
	while (line && strcmp(line, "quit"))
	{
		system("clear");

		logs = execute_commands(line, commands, &battle);

		player.health = get_max_health(&player);
		player.mana = get_max_mana(&player);

		print_entity(&battle, &player);
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

		print_commands(commands);

		free(line);
		line = readline(">> ");
	}

	free(line);

	return 0;
}

/* vim: set ts=4 sw=4 cc=80 : */
