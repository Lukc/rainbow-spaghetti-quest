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
inflict_damage(Battle* data, Entity* attacker, Entity* defender)
{
	int damage_inflicted;
	int type_modifier;

	type_modifier =
		get_type_resistance(data, defender, get_attack_type(data, attacker));

	damage_inflicted =
		(int) ((get_attack(data, attacker) * (100. - type_modifier)) / 100.) -
		get_defense(data, defender);

	damage_inflicted = damage_inflicted <= 0 ? 1 : damage_inflicted;
	defender->health -= damage_inflicted;

	return damage_inflicted;
}

Logs*
command_flee(void* opt)
{
	Battle *battle_data;
	Logs* logs;
	char* log;

	logs = logs_new();

	log = (char*) malloc(sizeof(char) * 128);
	snprintf(
		log, 128,
		"You fled from your battle!");
	logs_add(logs, log);

	battle_data = opt;

	battle_data->flee = 1;

	return NULL;
}

Logs*
command_attack(void* opt)
{
	Battle *battle_data;
	Entity *player;
	Entity *enemy;
	int damage_inflicted, damage_received;
	Logs* logs;
	char* log;

	logs = logs_new();

	battle_data = opt;
	player = battle_data->player;
	enemy = battle_data->enemy;

	damage_inflicted = inflict_damage(battle_data, player, enemy);

	log = (char*) malloc(sizeof(char) * 128);
	snprintf(
		log, 128,
		"You inflicted %i points of damage to your enemy!",
		damage_inflicted);
	logs_add(logs, log);

	if (enemy->health <= 0)
	{
		logs_add(logs, strdup("Your enemy is dead."));
	}
	else
	{
		damage_received = inflict_damage(battle_data, enemy, player);

		log = (char*) malloc(sizeof(char) * 128);
		snprintf(
			log, 128,
			"You received %i points of damage from your enemy!",
			damage_received);
		logs_add(logs, log);

		if (player->health <= 0)
		{
			logs_add(logs, strdup("You have been defeated..."));
		}
	}

	return logs;
}

Logs*
command_focus(void *opts)
{
	Battle *battle_data;
	Entity *player;
	int points_gained;
	Logs* logs;
	char* log;

	logs = logs_new();

	battle_data = opts;
	player = battle_data->player;

	points_gained = player->class->mana_regen_on_focus;
	points_gained = player->mana + points_gained > get_max_mana(player) ?
		get_max_mana(player) - player->mana : points_gained;

	log = (char*) malloc(sizeof(char) * 128);
	snprintf(
		log, 128,
		"You regenerated %i points of mana.",
		points_gained);
	logs_add(logs, log);

	logs_add(logs, strdup("Strangely enough, the enemy does nothing to take advantage of your temporary weakness."));

	return logs;
}

int
battle(Battle *battle_data)
{
	Logs* logs;
	char *line;
	Command commands[] = {
		{"attack",   "a",   command_attack,   "A weak attack."},
		{"special",  "s",   NULL,             "A strong attack that consumes mana."},
		{"focus",    "f",   command_focus,    "A defensive move that restores mana."},
		{"flee",     "l",   command_flee,     "A desperate move to get out of battle."},
		{NULL, NULL, NULL, NULL}
	};
	Entity *player = battle_data->player;
	Entity *enemy = battle_data->enemy;

	battle_data->flee = 0;

	system("clear");

	line = strdup("");
	while (!line || (line && strcmp(line, "quit")))
	{
		if (line)
		{
			system("clear");

			logs = execute_commands(line, commands, battle_data);

			print_entity(battle_data, player);
			printf(BRIGHT RED "\n -- " WHITE "versus" RED " --\n\n" NOCOLOR);
			print_entity_basestats(battle_data, enemy);
			printf("\n");

			if (logs)
			{
				logs_print(logs);
				logs_free(logs);
			}

			print_commands(commands);

			if (battle_data->flee)
			{
				system("clear");
				printf("\nYou manage to get out of the battle without being "
						"hurt too badly.\n");
				printf("\nPress any key to continue...\n\n");
				getchar();
				free(line);
				return 0;
			}
			else if (player->health <= 0)
			{
				player->caps /= 2;

				system("clear");
				printf("\nYou are DEAD.\n");
				printf("\nYou lost half your bottle caps.\n");
				printf("\nPress any key to continue...\n\n");
				getchar();
				free(line);
				return -1;
			}
			else if (enemy->health <= 0)
			{
				player->caps += enemy->class->caps_on_kill;

				system("clear");
				printf("\nYou are VICTORIOUS.\n");
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

static Class*
get_random_enemy(List* list)
{
	Class* class;
	Class* valid_classes[1024];
	int count = 0;

	while (list)
	{
		class = (Class*) list->data;

		/* FIXME: Doesn’t this feel a bit hacky? We should add a ->spawnable
		 *        element or make their spawn location-dependant */
		if (class->id >= 10)
		{
			valid_classes[count++] = class;
		}

		list = list->next;
	}

	return valid_classes[rand() % count];
}

Logs*
enter_battle(void *opt)
{
	Battle* battle_data;
	Entity* player, *enemy;
	List* classes;
	int result;

	battle_data = opt;
	player = battle_data->player;
	enemy = battle_data->enemy;
	classes = battle_data->classes;

	/* FIXME: Get only a mob that’s made to spawn in random battles */
	init_entity_from_class(enemy, get_random_enemy(classes));

	if ((result = battle(battle_data)) == 1)
	{
		player->kills++;
	}

	system("clear");

	return NULL;
}

/* vim: set ts=4 sw=4 cc=80 : */
