#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

#include "game.h"
#include "battle.h"
#include "types.h"
#include "commands.h"
#include "colors.h"
#include "term.h"
#include "entities.h"

/**
 * Prints the attacks’ selection menu of the battle interface.
 * @param list: The List* of Attack* to display.
 */
static void
print_attacks(Entity* player, List* list)
{
	int i;

	for (i = 0; i < 5; i++)
	{
		Attack* attack = NULL;
		
		if (list)
		{
			attack = list->data;

			char* name = attack->name;
			char* color = WHITE;

			if (!name)
				name = type_to_attack_name(attack->type);

			if (attack->mana_cost > player->mana)
				color = BLACK;

			printf("%s", color);

			printf("  (%i) %-9s %3i-%i %-10s" NOCOLOR, i,
				name,
				attack->damage + get_attack_bonus(player),
				attack->strikes, type_to_string(attack->type));

			if (attack->mana_cost)
				printf(" %+3iMP", -attack->mana_cost);
			else
				printf("      ");

			printf("\n");

			list = list->next;
		}
		else
		{
			printf(
				BLACK "  (%i) --------- \n" NOCOLOR, i);
		}
	}
}

/**
 * Prints the items’ selection menu of the battle interface.
 * @param page: Integer representing the “page” of the inventory to display.
 *  Each “page” is a group of 5 successive entries from the inventory.
 *
 * @todo: Print the Health/Mana bonuses of using the item.
 */
static void
print_items(Entity* player, int page)
{
	int i;

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

/**
 * Checks whether someone has enough mana to use an attack or not.
 */
static int
can_use_attack(Entity* attacker, Attack* attack)
{
	return attacker->mana >= attack->mana_cost;
}

/**
 * Takes care of A attacking B with a specified attack.
 * Adds a corresponding log entry in “logs”.
 */
static void
attack(Entity* attacker, Attack* attack, Entity* defender, Logs* logs)
{
	int damage_inflicted;
	int type_modifier;
	char* log;

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

	log = (char*) malloc(sizeof(char) * 128);
	snprintf(log, 128,
		BRIGHT WHITE "%s " RED ">>>"
		WHITE " %s (%i-%i %s):  " RED "%iHP",
		attacker->name, defender->name,
		attack->damage, attack->strikes, type_to_string(attack->type),
		damage_inflicted);
	logs_add(logs, log);
}

/**
 * Gives back mana (and possibly health or other stats).
 */
static void
focus(Entity* entity, Logs* logs)
{
	int mana_gained, health_gained;
	int i;
	char* log;

	health_gained = entity->class->health_regen_on_focus;

	for (i = 0; i < EQ_MAX; i++)
		if (entity->equipment[i])
			health_gained += entity->equipment[i]->health_on_focus;

	health_gained =
		entity->health + health_gained > get_max_health(entity) ?
			get_max_health(entity) : health_gained;

	mana_gained = entity->class->mana_regen_on_focus;

	for (i = 0; i < EQ_MAX; i++)
		if (entity->equipment[i])
			mana_gained += entity->equipment[i]->mana_on_focus;

	mana_gained =
		entity->mana + mana_gained > get_max_mana(entity) ?
			get_max_mana(entity) : mana_gained;

	entity->mana += mana_gained;
	entity->health += health_gained;

	log = (char*) malloc(sizeof(char) * 128);
	snprintf(
		log, 128,
		BRIGHT WHITE "%s focused >> " GREEN "%+-3iHP  " BLUE "%+-3iMP"
		NOCOLOR,
		entity->name, health_gained, mana_gained);
	logs_add(logs, log);
}

static void
use_item(Entity* entity, Item* item, Logs* logs)
{
	char* log;

	entity->health += item->health_on_use;
	entity->mana += item->mana_on_use;

	if (entity->health > get_max_health(entity))
		entity->health = get_max_health(entity);

	if (entity->mana > get_max_mana(entity))
		entity->mana = get_max_mana(entity);

	log = (char*) malloc(sizeof(char) * 128);
	snprintf(
		log, 128,
		BRIGHT WHITE "%s used a %s >> " GREEN "%+-3iHP  " BLUE "%+-3iMP"
		NOCOLOR,
		entity->name, item->name,
		item->health_on_use, item->mana_on_use);
	logs_add(logs, log);
}

static void
ai_action(Game* game, Logs* logs)
{
	Entity* player;
	Entity* enemy;
	List* available_attacks;
	Attack* selected_attack;

	player = game->player;
	enemy = game->enemy;

	available_attacks = get_all_attacks(enemy);
	selected_attack = list_nth(
		available_attacks,
		rand() % list_size(available_attacks));

	if (can_use_attack(enemy, selected_attack))
		attack(
			enemy,
			selected_attack,
			player,
			logs
		);
	else
		focus(enemy, logs);
}

Logs*
command_use_item(Game* game, Entity* player, Item* item)
{
	Logs* logs;

	logs = logs_new();

	use_item(player, item, logs);

	ai_action(game, logs);

	return logs;
}

Logs*
command_flee(Game* game)
{
	Logs* logs;
	char* log;

	logs = logs_new();

	log = (char*) malloc(sizeof(char) * 128);
	snprintf(
		log, 128,
		"You fled from battle!");
	logs_add(logs, log);

	game->flee = 1;

	return NULL;
}

Logs*
command_attack(Game* game, Attack* player_attack)
{
	Entity* player;
	Entity* enemy;
	Logs* logs;

	logs = logs_new();

	player = game->player;
	enemy = game->enemy;

	if (!can_use_attack(player, player_attack))
	{
		logs_add(logs, strdup("Not enough mana!"));

		return logs;
	}

	attack(player, player_attack, enemy, logs);

	if (enemy->health <= 0)
	{
		logs_add(logs, strdup("Your enemy is dead."));
	}
	else
	{
		ai_action(game, logs);
	}

	return logs;
}

Logs*
command_focus(Game* game)
{
	Logs* logs;

	logs = logs_new();

	focus(game->player, logs);

	ai_action(game, logs);

	return logs;
}

#define ATTACKS 0
#define ITEMS 1

int
battle(Game *game)
{
	Logs* logs;
	char input = -42;
	Entity *player = game->player;
	Entity *enemy = game->enemy;
	List* player_attacks;
	List* list;
	int view = ATTACKS;
	int page = 0; /* Related to view. */
	int i;

	game->flee = 0;

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
					logs = command_focus(game);
					break;
				case 'l':
					logs = command_flee(game);
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

								logs = command_attack(game, attack);
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
									game, player, item);

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
				print_attacks(player, player_attacks);
			}
			else if (view == ITEMS)
			{
				print_items(player, page);
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

			if (game->flee)
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
				player->gold /= 2;

				system("clear");
				printf("\nYou are DEAD.\n");
				printf("\nYou lost half your bottle gold.\n");
				printf("\nPress any key to continue...\n\n");
				getch();
				return -1;
			}
			else if (enemy->health <= 0)
			{
				player->gold += enemy->class->gold_on_kill;

				list = give_drop(player, enemy);

				system("clear");
				printf("\nYou are VICTORIOUS.\n");
				printf("\nYou gain %d bottle gold!\n",
					enemy->class->gold_on_kill);
				if (list)
				{
					printf("\nYou were able to loot the following items:\n");

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
enter_battle(Game* game)
{
	Class* enemy_class = NULL;
	Entity* player, *enemy;
	List* classes;
	int result;

	player = game->player;
	enemy = game->enemy;
	classes = game->classes;

	if (game->location->random_enemies)
		enemy_class =
			get_random_enemy(game->location->random_enemies);
	else
		return NULL;

	init_entity_from_class(enemy, enemy_class);

	if ((result = battle(game)) == 1)
	{
		player->kills++;
	}

	system("clear");

	return NULL;
}

/* vim: set ts=4 sw=4 cc=80 : */
