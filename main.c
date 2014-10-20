#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include <readline/readline.h>

#include "types.h"
#include "commands.h"
#include "colors.h"
#include "entities.h"

char**
command_attack(void *p, void *e)
{
	Entity *player;
	Entity *enemy;
	int damageInflicted, damageReceived;
	char **logs;
	int logs_i;

	logs_i = 0;
	logs = (char**) malloc(sizeof(char*) * 6);

	player = p;
	enemy = e;

	damageInflicted = get_attack(player) - get_defense(enemy);
	enemy->health -= damageInflicted;

	logs[logs_i] = (char*) malloc(sizeof(char) * 128);
	snprintf(
		logs[logs_i], 128,
		"You inflicted %i points of damage to your enemy!",
		damageInflicted);
	logs_i++;

	if (enemy->health <= 0)
	{
		logs[logs_i] = (char*) malloc(sizeof(char) * 128);
		snprintf(
			logs[logs_i], 128,
			"You killed your enemy!");
		logs_i++;
	}
	else
	{
		damageReceived = get_attack(enemy) - get_defense(player);
		player->health -= damageReceived;

		logs[logs_i] = (char*) malloc(sizeof(char) * 128);
		snprintf(
			logs[logs_i], 128,
			"You received %i points of damage from your enemy!",
			damageReceived);
		logs_i++;

		if (player->health <= 0)
		{
			logs[logs_i] = (char*) malloc(sizeof(char) * 128);
			snprintf(
				logs[logs_i], 128,
				"Your enemy killed you...");
			logs_i++;
		}
	}

	logs[logs_i + 1] = NULL;

	return logs;
}

char**
command_special(void *p, void *e)
{
	char **logs;

	logs = (char**) malloc(sizeof(char*) * 2);
	logs[0] = strdup("Uh. Got too lazy to implement that one. Also, it duplicates code from the “attack” one…");
	logs[1] = NULL;

	return logs;
}

int
battle(Entity *player, Entity *enemy)
{
	char **logs;
	char *line;
	Command commands[] = {
		{"attack",   "a",   command_attack,   "A weak attack."},
		{"special",  "s",   command_special,  "A strong attack that consumes mana."},
		{"focus",    "f",   NULL,             "A defensive move that restores mana."},
		{"flee",     "l",   NULL,             "A desperate move to get out of battle."},
		{NULL, NULL, NULL, NULL}
	};

	system("clear");

	line = strdup("");
	while (line && strcmp(line, "quit"))
	{
		int i;

		system("clear");

		logs = execute_commands(line, commands, player, enemy);

		print_entity(enemy);
		printf("\n");
		print_entity(player);
		printf("\n");

		if (logs)
		{
			for (i = 0; logs[i] && i < 16; i++)
			{
				printf("%s\n", logs[i]);
				free(logs[i]);
			}

			free(logs);

			printf("\n");
		}

		printf("Options:\n");
		for (i = 0; commands[i].name; i++)
		{
			unsigned int j;

			printf("  - %s: ", commands[i].name);

			for (j = 0; j < 8 - strlen(commands[i].name); j++)
				printf(" ");

			printf("%s\n", commands[i].description);
		}

		if (player->health <= 0)
		{
			printf("\nYou are DEAD.\n");
			printf("\nPress any key to continue...\n\n");
			getchar();
			return -1;
		}
		if (enemy->health <= 0)
		{
			printf("\nYou are VICTORIOUS.\n");
			printf("\nPress any key to continue...\n\n");
			getchar();
			return 1;
		}

		line = readline(">> ");
	}

	/* ^D or "quit" entered */
	exit(0);

	return 0;
}

char**
enter_battle(void *p, void *e)
{
	Entity *player, *enemy;

	player = p;
	enemy = e;

	init_entity(enemy);

	enemy->health = 40;

	if (battle(player, enemy) == 1)
	{
		player->kills++;
	}


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
		{"battle", "b", enter_battle, "Find a random enemy to beat to death."},
		{NULL, NULL, NULL, NULL}
	};

	init_entity(&player);

	player.name = "Player";

	line = "";
	while (line && strcmp(line, "quit"))
	{
		int i;
		system("clear");

		logs = execute_commands(line, commands, &player, &enemy);

		player.health = get_max_health(&player);
		player.mana = get_max_mana(&player);

		print_entity(&player);
		printf("\n");

		if (logs)
		{
			for (i = 0; logs[i] && i < 16; i++)
			{
				printf("%s\n", logs[i]);
				free(logs[i]);
			}

			free(logs);

			printf("\n");
		}

		printf("Options:\n");
		for (i = 0; commands[i].name; i++)
		{
			unsigned int j;

			printf("  - %s: ", commands[i].name);

			for (j = 0; j < 8 - strlen(commands[i].name); j++)
				printf(" ");

			printf("%s\n", commands[i].description);
		}

		line = readline(">> ");
	}

	return 0;
}

/* vim: set ts=4 sw=4 cc=80 : */
