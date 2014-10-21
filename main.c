#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include <readline/readline.h>

#include "types.h"
#include "commands.h"
#include "colors.h"
#include "entities.h"
#include "battle.h"

int
main(int argc, char* argv[])
{
	Entity player, enemy;
	char *line;
	char **logs;
	Command commands[] = {
		{"battle",  "b", enter_battle, "Find a random enemy to beat to death."},
		{"shop",    "s", NULL,         "Buy new equipment to improve your stats!"},
		{"dungeon", "d", NULL,         "Enter a terrible dungeon and fight hordes of enemies!"},
		{NULL, NULL, NULL, NULL}
	};
	Class *classes;
	Battle battle;

	classes = load_classes("classes");

	/* For now, repeatability would be useful */
	srand(42);

	init_entity_from_class(&player, get_class_by_name(classes, "Warrior"));

	player.name = (char*) malloc(sizeof(char) * 80);
	snprintf(
		player.name, 80, "%s the %s",
		argc > 1 ? argv[1] : "Bob", player.class->name
	);

	battle.player = &player;
	battle.enemy = &enemy;
	battle.classes = classes;

	line = strdup("");
	while (line && strcmp(line, "quit"))
	{
		system("clear");

		logs = execute_commands(line, commands, &battle);

		player.health = get_max_health(&player);
		player.mana = get_max_mana(&player);

		print_entity(&player);
		printf("\n");

		print_logs(logs);

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

	remove_entity(&player);
	free(line);

	return 0;
}

/* vim: set ts=4 sw=4 cc=80 : */
