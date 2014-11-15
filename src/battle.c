#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

#include "battle.h"
#include "types.h"
#include "commands.h"
#include "colors.h"
#include "term.h"
#include "entities.h"

/**
 * Checks whether someone has enough mana to use an attack or not.
 */
static int
can_use_attack(Entity* attacker, Attack* attack)
{
	return attacker->mana >= attack->mana_cost;
}

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

	attacker->mana -= attack->mana_cost;
	defender->health -= damage_inflicted;

	return damage_inflicted;
}

/**
 * Grants back mana (and possibly health or other stats).
 *
 * FIXME: Grooming. Also, be carefull for stuff not to go over max values.
 * FIXME: Fully rewrite.
 */
static void
focus(Entity* entity, Logs* logs)
{
	int mana_gained;
	char* log;

	/* FIXME: Items’ ->health/mana_on_focus are ignored. */
	mana_gained = entity->class->mana_regen_on_focus;
	mana_gained = entity->mana + mana_gained > get_max_mana(entity) ?
		get_max_mana(entity) - entity->mana : mana_gained;

	entity->health += entity->class->health_regen_on_focus;
	entity->mana += mana_gained;

	log = (char*) malloc(sizeof(char) * 128);
	snprintf(
		log, 128,
		"%s regenerated %i points of mana.",
		entity->name,
		mana_gained);
	logs_add(logs, log);
}

static void
use_item(Entity* entity, Item* item, Logs* logs)
{
	/* FIXME: Hum… make sure it doesn’t go too far out of boundaries. */
	entity->health += item->health_on_use;
	entity->mana += item->mana_on_use;
}

static void
ai_action(Battle* data, Logs* logs)
{
	int damage_received;
	char* log;
	Entity* player;
	Entity* enemy;
	List* available_attacks;
	Attack* selected_attack;

	player = data->player;
	enemy = data->enemy;

	available_attacks = get_all_attacks(enemy);
	selected_attack = list_nth(
		available_attacks,
		rand() % list_size(available_attacks));

	if (can_use_attack(enemy, selected_attack))
	{
		damage_received = attack(
			enemy,
			selected_attack,
			player
		);

		log = (char*) malloc(sizeof(char) * 128);
		snprintf(log, 128,
			"You have been %s for %iHP!",
			type_to_damage_string(selected_attack->type), damage_received);
		logs_add(logs, log);
	}
	else
		focus(enemy, logs);

	if (player->health <= 0)
	{
		logs_add(logs, strdup("You have been defeated..."));
	}
}

Logs*
command_use_item(Battle* data, Entity* player, Item* item)
{
	Logs* logs;

	logs = logs_new();

	use_item(player, item, logs);

	ai_action(data, logs);

	return logs;
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

	if (!can_use_attack(player, player_attack))
	{
		logs_add(logs, strdup("Not enough mana!"));

		return logs;
	}

	damage_inflicted = attack(player, player_attack, enemy);

	log = (char*) malloc(sizeof(char) * 128);
	snprintf(
		log, 128,
		"You %s your enemy for %iHP!",
		type_to_damage_string(player_attack->type), damage_inflicted);
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
	Logs* logs;

	logs = logs_new();

	focus(battle_data->player, logs);

	ai_action(battle_data, logs);

	return logs;
}

#define ATTACKS 0
#define ITEMS 1

int
battle(Battle *battle_data)
{
	Logs* logs;
	char input = -42;
	Entity *player = battle_data->player;
	Entity *enemy = battle_data->enemy;
	List* player_attacks;
	List* list;
	int view = ATTACKS;
	int page = 0; /* Related to view. */
	int i;

	battle_data->flee = 0;

	system("clear");

	while (1)
	{
		if (!isexit(input))
		{
			player_attacks = get_all_attacks(player);

			back_to_top();

			logs = NULL;

			switch (input)
			{
				case 'f':
					logs = command_focus(battle_data);
					break;
				case 'l':
					logs = command_flee(battle_data);
					break;
				case 'i':
					view = !view;

					if (view == ITEMS)
						page = 0;
					break;
				case '+':
					page = page >= INVENTORY_SIZE / 5 ? page : page + 1;
					break;
				case '-':
					page = page == 0 ? 0 : page - 1;
					break;
				default:
					if (input >= '0' && input < '5')
					{
						input = input - '0';
						if (view == ATTACKS)
							if (list_size(player_attacks) > input)
							{
								Attack* attack =
									list_nth(player_attacks, input);

								logs = command_attack(battle_data, attack);
							}
							else
							{
								/* FIXME: Be mean to the player. */
							}
						else if (view == ITEMS)
						{
							Item* item;
							int index = input + page * 5;

							if (index < INVENTORY_SIZE &&
								(item = player->inventory[index]))
							{
								logs = command_use_item(
									battle_data, player, item);

								if (item->consumable)
									player->inventory[index] = NULL;
							}
							else
							{
								/* FIXME: "No such item in inventory..." */
							}
						}
					}
					else
						; /* FIXME: Be mean. */
			}

			print_entity_basestats(player);
			printf(BRIGHT RED "\n -- " WHITE "versus" RED " --\n\n" NOCOLOR);
			print_entity_basestats(enemy);
			printf("\n");

			if (logs)
				list = logs->head;
			else
				list = NULL;

			for (i = 0; i < 6; i++)
			{
				int j;

				/* Clearing logs area, line by line. */
				for (j = 0; j < 80; j++)
					printf(" ");
				printf("\n");

				if (list)
				{
					back(1);

					printf("%s\n", list->data);

					list = list->next;
				}
			}

			if (logs)
				logs_free(logs);

			menu_separator();

			for (i = 0; i < 5; i++)
			{
				int j;

				/* Clearing attacks/items area, line by line. */
				for (j = 0; j < 40; j++)
					printf(" ");
				printf("\n");
			}
			back(5);

			if (view == ATTACKS)
			{
				list = player_attacks;
				for (i = 0; i < 5; i++)
				{
					Attack* attack = NULL;
					
					if (list)
					{
						attack = list->data;

						char* name = attack->name;

						if (!name)
							name = type_to_attack_name(attack->type);

						printf(WHITE "  (%i) %-9s %3i-%i %s\n" NOCOLOR, i,
							name,
							attack->damage + get_attack_bonus(player),
							attack->strikes, type_string(attack->type));

						list = list->next;
					}
					else
					{
						printf(
							BLACK "  (%i) --------- \n" NOCOLOR, i);
					}
				}
			}
			else if (view == ITEMS)
			{
				for (i = 0; i < 5; i++)
				{
					int index = i + page * 5;

					if (index < INVENTORY_SIZE)
					{
						Item* item;

						if ((item = player->inventory[index]))
						{
							if (is_item_usable(item))
							{
								if (item->consumable)
									printf(GREEN);
								else
									printf(WHITE);
							}
							else
								printf(BLACK);

							printf("  (%i) %-9s\n" NOCOLOR,
								i, item->name);
						}
						else
							printf(
								BLACK "  (%i) --------- \n" NOCOLOR, i);
					}
					else
						printf("\n");
				}
			}

			back(5);
			move(40);
			printf(WHITE "  (f) focus\n" NOCOLOR);
			move(40);
			printf(WHITE "  (l) flee\n" NOCOLOR);
			move(40);
			printf(YELLOW "  (i) use item\n" NOCOLOR);

			if (view == ITEMS)
			{
				move(40);
				printf(YELLOW "  (+) next\n" NOCOLOR);
				move(40);
				printf(YELLOW "  (-) previous\n" NOCOLOR);
			}
			else
				printf("\n\n");

			menu_separator();

			if (battle_data->flee)
			{
				system("clear");
				printf("\nYou manage to get out of the battle without being "
						"hurt too badly.\n");
				printf("\nPress any key to continue...\n\n");
				getch();
				return 0;
			}
			else if (player->health <= 0)
			{
				player->caps /= 2;

				system("clear");
				printf("\nYou are DEAD.\n");
				printf("\nYou lost half your bottle caps.\n");
				printf("\nPress any key to continue...\n\n");
				getch();
				return -1;
			}
			else if (enemy->health <= 0)
			{
				player->caps += enemy->class->caps_on_kill;

				list = give_drop(player, enemy);

				system("clear");
				printf("\nYou are VICTORIOUS.\n");
				printf("\nYou gain %d bottle caps!\n",
					enemy->class->caps_on_kill);
				if (list)
				{
					printf("\nObtained loot:\n");

					for (; list; list = list->next)
						printf("  - %s\n", ((Item*) list->data)->name);
				}

				printf("\nPress any key to continue...\n\n");
				getch();
				return 1;
			}

			input = getch();
		}
		else
			input = getch();
	}

	exit(0);

	return 0;
}

#undef ITEMS
#undef ATTACKS

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
