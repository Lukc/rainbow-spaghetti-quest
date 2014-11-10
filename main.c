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

int
main(int argc, char* argv[])
{
	Entity player, enemy;
	char* line;
	Logs* logs;
	Command commands[] = {
		{"battle",  "b", enter_battle, "Find a random enemy to beat to death."},
		{"shop",    "s", enter_shop,   "Buy new equipment to improve your stats!"},
		{"dungeon", "d", NULL,         "Enter a terrible dungeon and fight hordes of enemies!"},
		{NULL, NULL, NULL, NULL}
	};
	Class *classes;
	Item *items;
	Battle battle;

	classes = load_classes("classes");
	items = load_items("items");

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
	battle.classes = classes;
	battle.items = items;

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

		print_commands(commands);

		free(line);
		line = readline(">> ");
	}

	free(line);

	return 0;
}

/* vim: set ts=4 sw=4 cc=80 : */
