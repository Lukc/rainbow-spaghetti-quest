#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include <readline/readline.h>

#include "battle.h"
#include "types.h"
#include "commands.h"
#include "colors.h"
#include "entities.h"

static int
inflict_damage(Entity *attacker, Entity *defender)
{
	int damage_inflicted;

	damage_inflicted = get_attack(attacker) - get_defense(defender);
	damage_inflicted = damage_inflicted <= 0 ? 1 : damage_inflicted;
	defender->health -= damage_inflicted;

	return damage_inflicted;
}

char**
command_attack(void *opt)
{
	Battle *battle_data;
	Entity *player;
	Entity *enemy;
	int damage_inflicted, damage_received;
	char **logs;
	int logs_i;

	logs_i = 0;
	logs = (char**) malloc(sizeof(char*) * 5);

	battle_data = opt;
	player = battle_data->player;
	enemy = battle_data->enemy;

	damage_inflicted = inflict_damage(player, enemy);

	logs[logs_i] = (char*) malloc(sizeof(char) * 128);
	snprintf(
		logs[logs_i], 128,
		"You inflicted %i points of damage to your enemy!",
		damage_inflicted);
	logs_i++;

	if (enemy->health <= 0)
	{
		logs[logs_i] = strdup("You killed your enemy.");
		logs_i++;
	}
	else
	{
		damage_received = inflict_damage(enemy, player);

		logs[logs_i] = (char*) malloc(sizeof(char) * 128);
		snprintf(
			logs[logs_i], 128,
			"You received %i points of damage from your enemy!",
			damage_received);
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

char**
command_focus(void *opts)
{
	Battle *battle_data;
	Entity *player;
	char **logs;
	int logs_i;
	int points_gained;

	battle_data = opts;
	player = battle_data->player;

	logs_i = 0;
	logs = (char**) malloc(sizeof(char*) * 4);

	points_gained = player->class->mana_regen_on_focus;
	points_gained = player->mana + points_gained > get_max_mana(player) ?
		get_max_mana(player) - player->mana : points_gained;

	logs[logs_i] = (char*) malloc(sizeof(char) * 128);
	snprintf(
		logs[logs_i], 128,
		"You regenerated %i points of mana.",
		points_gained);
	logs_i++;

	logs[logs_i] = strdup("Strangely enough, the enemy does nothing to take advantage of your temporary weakness.");
	logs_i++;

	logs[logs_i] = NULL;

	return logs;
}

int
battle(Battle *battle_data)
{
	char **logs = NULL;
	char *line;
	Command commands[] = {
		{"attack",   "a",   command_attack,   "A weak attack."},
		{"special",  "s",   NULL,             "A strong attack that consumes mana."},
		{"focus",    "f",   command_focus,    "A defensive move that restores mana."},
		{"flee",     "l",   NULL,             "A desperate move to get out of battle."},
		{NULL, NULL, NULL, NULL}
	};
	Entity *player = battle_data->player;
	Entity *enemy = battle_data->enemy;

	system("clear");

	line = strdup("");
	while (!line || (line && strcmp(line, "quit")))
	{
		if (line)
		{
			system("clear");

			logs = execute_commands(line, commands, battle_data);

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
		else
			line = readline("");
	}

	/* ^D or "quit" entered */
	if (line)
		free(line);
	exit(0);

	return 0;
}

char**
enter_battle(void *opt)
{
	Battle* battle_data;
	Entity* player, *enemy;
	Class* classes;

	battle_data = opt;
	player = battle_data->player;
	enemy = battle_data->enemy;
	classes = battle_data->classes;

	/* FIXME: Get only a mob that’s made to spawn in random battles */
	init_entity_from_class(enemy, &classes[rand() % 2]);

	if (battle(battle_data) == 1)
	{
		player->kills++;
	} else {
		player->caps /= 2;
	}

	remove_entity(enemy);

	system("clear");

	return NULL;
}

/* vim: set ts=4 sw=4 cc=80 : */
