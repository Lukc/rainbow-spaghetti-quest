#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include <readline/readline.h>

#include "battle.h"
#include "types.h"
#include "commands.h"
#include "colors.h"
#include "entities.h"

/**
 * @return The amount of net damage inflicted to “defender”.
 */
static int
attack(Entity* attacker, Attack* attack, Entity* defender)
{
	int damage_inflicted;
	int type_modifier;

	type_modifier = get_type_resistance(defender, attack->type);

	/* Calculating for a single strike */
	damage_inflicted =
		(int) (((get_attack_bonus(attacker) + attack->damage) *
				(100. - type_modifier)) / 100.) -
		get_defense_bonus(defender);

	/* Taking care of negative damages... */
	damage_inflicted = damage_inflicted < 0 ? 0 : damage_inflicted;

	/* Taking the number of strikes into account now */
	damage_inflicted = damage_inflicted * attack->strikes;

	defender->health -= damage_inflicted;

	return damage_inflicted;
}

static void
ai_action(Battle* data, Logs* logs)
{
	int damage_received;
	char* log;
	Entity* player;
	Entity* enemy;
	List* available_attacks;

	player = data->player;
	enemy = data->enemy;

	available_attacks = get_all_attacks(enemy);

	damage_received = attack(
		enemy,
		(Attack*) list_nth(
			available_attacks,
			rand() % list_size(available_attacks)
		),
		player
	);

	log = (char*) malloc(sizeof(char) * 128);
	snprintf(log, 128,
		"You received %i points of damage from your enemy!",
		damage_received);
	logs_add(logs, log);

	if (player->health <= 0)
	{
		logs_add(logs, strdup("You have been defeated..."));
	}
}

Logs*
command_flee(Battle* battle_data)
{
	Logs* logs;
	char* log;

	logs = logs_new();

	log = (char*) malloc(sizeof(char) * 128);
	snprintf(
		log, 128,
		"You fled from your battle!");
	logs_add(logs, log);

	battle_data->flee = 1;

	return NULL;
}

Logs*
command_attack(Battle* battle_data, Attack* player_attack)
{
	Entity* player;
	Entity* enemy;
	int damage_inflicted;
	Logs* logs;
	char* log;

	logs = logs_new();

	player = battle_data->player;
	enemy = battle_data->enemy;

	damage_inflicted = attack(player, player_attack, enemy);

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
		ai_action(battle_data, logs);
	}

	return logs;
}

Logs*
command_focus(Battle* battle_data)
{
	Entity* player;
	int points_gained;
	Logs* logs;
	char* log;

	logs = logs_new();

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

	ai_action(battle_data, logs);

	return logs;
}

int
battle(Battle *battle_data)
{
	Logs* logs;
	char *line;
	Entity *player = battle_data->player;
	Entity *enemy = battle_data->enemy;
	List* player_attacks;
	List* list;
	int i;

	battle_data->flee = 0;

	system("clear");

	line = strdup("");
	while (!line || (line && strcmp(line, "quit")))
	{
		if (line)
		{
			player_attacks = get_all_attacks(player);

			system("clear");

			logs = NULL;

			if (!strcmp(line, "")) {}
			else if (!strcmp(line, "f") || !strcmp(line, "focus"))
				logs = command_focus(battle_data);
			else if (!strcmp(line, "l") || !strcmp(line, "flee"))
				logs = command_flee(battle_data);
			else
			{
				if (list_size(player_attacks) > atoi(line))
				{
					Attack* attack = list_nth(player_attacks, atoi(line));

					logs = command_attack(battle_data, attack);
				}
				else
				{
					/* FIXME: Be mean to the player. */
				}
			}

			print_entity(player);
			printf(BRIGHT RED "\n -- " WHITE "versus" RED " --\n\n" NOCOLOR);
			print_entity_basestats(enemy);
			printf("\n");

			if (logs)
			{
				logs_print(logs);
				logs_free(logs);
			}

			printf("Options:\n");

			i = 0;
			for (list = player_attacks; list; list = list->next)
			{
				Attack* attack = list->data;

				printf(WHITE "  - (%i)%14i-%i %s\n" NOCOLOR, i,
					attack->damage + get_attack_bonus(player),
					attack->strikes, type_string(attack->type));

				i++;
			}

			printf(
				WHITE
				"  - (f) focus:      A defensive move that restores mana.\n"
				"  - (l) flee:       A desperate move to get out of battle.\n"
				NOCOLOR
			);

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

		valid_classes[count++] = class;

		list = list->next;
	}

	return valid_classes[rand() % count];
}

Logs*
enter_battle(void *opt)
{
	Battle* battle_data;
	Class* enemy_class = NULL;
	Entity* player, *enemy;
	List* classes;
	int result;

	battle_data = opt;
	player = battle_data->player;
	enemy = battle_data->enemy;
	classes = battle_data->classes;

	if (battle_data->location->random_enemies)
		enemy_class =
			get_random_enemy(battle_data->location->random_enemies);
	else
		return NULL;

	init_entity_from_class(enemy, enemy_class);

	if ((result = battle(battle_data)) == 1)
	{
		player->kills++;
	}

	system("clear");

	return NULL;
}

/* vim: set ts=4 sw=4 cc=80 : */
