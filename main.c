#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include <readline/readline.h>

#include "types.h"
#include "commands.h"
#include "colors.h"
#include "entities.h"

/* GLOBAL ALERT */
Class classes[] = {
	{1,  "Warrior",     30, 20, 35, 5, 3},
	{42, "Stray Kitty", 7,  1,  2,  4, 1},
	{43, "Stray Puppy", 8,  1,  3,  3, 2},
	{0,  NULL, 0, 0, 0, 0, 0}
};

char**
command_attack(void *p, void *e)
{
	Entity *player;
	Entity *enemy;
	int damageInflicted, damageReceived;
	char **logs;
	int logs_i;

	logs_i = 0;
	logs = (char**) malloc(sizeof(char*) * 5);

	player = p;
	enemy = e;

	damageInflicted = get_attack(player) - get_defense(enemy);
	damageInflicted = damageInflicted <= 0 ? 1 : damageInflicted;
	enemy->health -= damageInflicted;

	logs[logs_i] = (char*) malloc(sizeof(char) * 128);
	snprintf(
		logs[logs_i], 128,
		"You inflicted %i points of damage to your enemy!",
		damageInflicted);
	logs_i++;

	if (enemy->health <= 0)
	{
		logs[logs_i] = strdup("You killed your enemy.");
		logs_i++;
	}
	else
	{
		damageReceived = get_attack(enemy) - get_defense(player);
		damageReceived = damageReceived <= 0 ? 1 : damageReceived;
		player->health -= damageReceived;

		logs[logs_i] = (char*) malloc(sizeof(char) * 128);
		snprintf(
			logs[logs_i], 128,
			"You received %i points of damage from your enemy!",
			damageReceived);
		logs_i++;

		if (player->health <= 0)
		{
			logs[logs_i] = strdup("Your enemy killed you...");
			logs_i++;
		}
	}

	logs[logs_i] = NULL;

	return logs;
}

int
battle(Entity *player, Entity *enemy)
{
	char **logs = NULL;
	char *line;
	Command commands[] = {
		{"attack",   "a",   command_attack,   "A weak attack."},
		{"special",  "s",   NULL,             "A strong attack that consumes mana."},
		{"focus",    "f",   NULL,             "A defensive move that restores mana."},
		{"flee",     "l",   NULL,             "A desperate move to get out of battle."},
		{NULL, NULL, NULL, NULL}
	};

	system("clear");

	line = strdup("");
	while (line && strcmp(line, "quit"))
	{
		system("clear");

		logs = execute_commands(line, commands, player, enemy);

		print_entity(player);
		printf(BRIGHT RED "\n -- " WHITE "versus" RED " --\n\n" NOCOLOR);
		print_entity(enemy);
		printf("\n");

		print_logs(logs);

		print_commands(commands);

		if (player->health <= 0)
		{
			printf("You are DEAD.\n");
			printf("\nPress any key to continue...\n\n");
			getchar();
			free(line);
			return -1;
		}
		if (enemy->health <= 0)
		{
			player->caps += enemy->class->caps_on_kill;

			printf("You are VICTORIOUS.\n");
			printf("\nYou gain %d bottle caps!\n", enemy->class->caps_on_kill);
			printf("\nPress any key to continue...\n\n");
			free(line);
			getchar();
			return 1;
		}

		free(line);
		line = readline(">> ");
	}

	/* ^D or "quit" entered */
	free(line);
	exit(0);

	return 0;
}

char**
enter_battle(void *p, void *e)
{
	Entity *player, *enemy;

	player = p;
	enemy = e;

	init_entity_from_class(enemy, &classes[1 + rand() % 2]);

	if (battle(player, enemy) == 1)
	{
		player->kills++;
	} else {
		player->caps /= 2;
	}

	remove_entity(enemy);

	system("clear");

	return NULL;
}

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

	/* For now, repeatability would be useful */
	srand(42);

	init_entity_from_class(&player, &classes[0]);

	player.name = (char*) malloc(sizeof(char) * 80);
	snprintf(
		player.name, 80, "%s the %s",
		argc > 1 ? argv[1] : "Bob", player.class->name
	);

	line = strdup("");
	while (line && strcmp(line, "quit"))
	{
		system("clear");

		logs = execute_commands(line, commands, &player, &enemy);

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
