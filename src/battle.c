#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

#include "game.h"
#include "enemies.h"
#include "battle.h"
#include "types.h"
#include "commands.h"
#include "colors.h"
#include "term.h"
#include "entities.h"

/**
 * @fixme: deduplicate (mostly, redundant string operations)
 */

static int
get_mana_cost(Entity* e, Attack* attack)
{
	List* l;
	int cost = attack->mana_cost;

	if (cost < 0)
		return e->mana - cost > get_max_mana(e) ?
			- get_max_mana(e) + e->mana : cost;

	for (l = e->statuses; l; l = l->next)
	{
		StatusData* status = l->data;

		if (status->status->increases_mana_costs)
			cost *= 2;
	}

	return cost;
}

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
			int mana_cost;
			char* name;
			char* color = WHITE;

			attack = list->data;

			mana_cost = get_mana_cost(player, attack);

			if (attack->name)
				name = strdup(attack->name);
			else
				name = strdup(type_to_attack_name(attack->type));

			if (strlen(name) > 10)
				name[11] = '\0';

			if (mana_cost > player->mana)
				color = BLACK;

			printf("%s", color);

			if (attack->strikes)
				printf("(%i) %-11s %3i-%i %-10s" NOCOLOR, i + 1,
					name,
					attack->damage + get_attack_bonus(player),
					attack->strikes, type_to_string(attack->type));
			else
			{
				printf("(%i) %-11s  ", i + 1, name);

				if (attack->gives_health > 0)
					printf(BRIGHT GREEN);
				else
					printf(BRIGHT RED);
				
				printf("%+3iHP %-9s" NOCOLOR,
					attack->gives_health, "");
			}

			if (mana_cost > attack->mana_cost)
				printf("%s", RED);
			else if (mana_cost < 0)
				printf("%s", BLUE);

			printf(" %+3iMP", -mana_cost);

			printf("\n" NOCOLOR);

			free(name);

			list = list->next;
		}
		else
		{
			printf(BLACK "  (%i) --------- \n" NOCOLOR, i + 1);
		}
	}
}

/**
 * Checks whether someone has enough mana to use an attack or not.
 */
static int
can_use_attack(Entity* attacker, Attack* attack)
{
	return attacker->mana >= get_mana_cost(attacker, attack);
}

/**
 * Takes care of A attacking B with a specified attack.
 * Adds a corresponding log entry in “logs”.
 */
static void
attack(Entity* attacker, Attack* attack, Entity* defender, Logs* logs)
{
	int strikes;
	int damage_inflicted = 0;
	int mana_cost;
	int type_modifier;
	char* log;
	char attack_string[64];
	char status_string[64];
	char self_status_string[64];
	char healing_string[64];
	char injury_string[64];
	char mana_string[64];
	char* cure_string = "";

	attack_string[0] = '\0';
	self_status_string[0] = '\0';
	status_string[0] = '\0';
	healing_string[0] = '\0';
	injury_string[0] = '\0';
	mana_string[0] = '\0';

	mana_cost = get_mana_cost(attacker, attack);

	if (attack->self_inflicts_status)
	{
		inflict_status(attacker, attack->self_inflicts_status);

		snprintf(self_status_string, 64, MAGENTA " <<< %s", attack->self_inflicts_status->name);
	}

	if (attack->strikes)
	{
		List* l;

		strikes = attack->strikes;

		for (l = attacker->statuses; l; l = l->next)
		{
			StatusData* data = l->data;
			Status* status = data->status;

			if (mana_cost > 0)
			{
				if (status->reduces_magical_strikes)
					strikes /= 2;
			}
			else
				if (status->reduces_physical_strikes)
					strikes /= 2;
		}

		type_modifier = get_type_resistance(defender, attack->type);

		/* Calculating for a single strike */
		damage_inflicted =
			(int) (((get_attack_bonus(attacker) + attack->damage) *
					(100. - type_modifier)) / 100.) -
			get_defense_bonus(defender);

		/* Taking care of negative damages... */
		damage_inflicted = damage_inflicted < 0 ? 0 : damage_inflicted;

		/* Taking the number of strikes into account now */
		damage_inflicted = damage_inflicted * strikes;

		defender->health -= damage_inflicted;

		snprintf(attack_string, 64,
			RED " >>>" WHITE " %s " RED "-%iHP " WHITE "<%i-%i %s>",
			defender->name,
			damage_inflicted,
			get_attack_bonus(attacker) + attack->damage,
			strikes, type_to_string(attack->type)
		);
	}

	if (attack->inflicts_status)
	{
		inflict_status(defender, attack->inflicts_status);

		snprintf(
			status_string, 64,
			MAGENTA " >>> %s",
			attack->inflicts_status->name
		);
	}

	if (attack->cures_statuses)
	{
		List* l;

		for (l = attack->cures_statuses; l; l = l->next)
		{
			Status* status = l->data;

			cure_status(attacker, status);
		}

		cure_string = BRIGHT WHITE " -" CYAN "Cured!" WHITE "-";
	}

	if (attack->gives_health)
	{
		List* l;
		int can_recover = 1;

		if (attack->gives_health > 0)
		{
			for (l = attacker->statuses; l && can_recover; l = l->next)
			{
				StatusData* data = l->data;
				Status* status = data->status;

				if (status->prevents_recovery)
					can_recover = 0;
			}

			if (can_recover)
			{
				give_health(attacker, attack->gives_health);

				snprintf(
					healing_string, 64,
					GREEN " <<< +%iHP" WHITE,
					attack->gives_health
				);
			}
			else
				snprintf(
					healing_string, 64,
					MAGENTA " <<< -recovery prevented-"
				);
		}
		else
		{
			attacker->health += attack->gives_health;

			snprintf(
				injury_string, 64,
				RED " <<< %+iHP", attack->gives_health
			);
		}
	}

	snprintf(mana_string, 64, BLUE " <<< %s%+iMP",
		mana_cost > 0 ? GRAY : "", -mana_cost);

	attacker->mana -= mana_cost;

	log = (char*) malloc(sizeof(char) * 128);
	snprintf(log, 128,
		BRIGHT WHITE "  %s%s%s%s%s%s",
		injury_string,
		healing_string,
		attack_string,
		status_string,
		cure_string,
		mana_string
	);
	logs_add(logs, log);
}

/**
 * Gives back mana (and possibly health or other stats).
 *
 * @fixme: Be clearer about recovery being prevented, when it is.
 */
static void
focus(Entity* entity, Logs* logs)
{
	int mana_gained, health_gained;
	int can_recover = 1;
	int i;
	char health_string[64];
	char mana_string[64];
	char cure_string[64];
	int cured = 0;
	char* log;
	List* l;
	List* next;

	for (l = entity->statuses; l && can_recover; l = l->next)
	{
		StatusData* data = l->data;
		Status* status = data->status;

		if (status->prevents_recovery)
			can_recover = 0;
	}

	for (l = entity->statuses; l; l = next)
	{
		StatusData* data = l->data;
		Status* status = data->status;

		/* l might be freed due to cure_status. Getting next element now. */
		next = l->next;

		if (status->removed_on_focus)
			if (has_status(entity, status))
			{
				cured = 1;
				cure_status(entity, status);
			}
	}

	if (cured)
		snprintf(cure_string, 64, BRIGHT WHITE " -" CYAN "Cured!" WHITE "-");
	else
		cure_string[0] = '\0';

	log = (char*) malloc(sizeof(char) * 128);
	snprintf(
		log, 128,
		BRIGHT WHITE "%s focuses"
		NOCOLOR,
		entity->name);
	logs_add(logs, log);

	if (can_recover)
	{
		health_gained = entity->class->health_regen_on_focus;

		for (i = 0; i < EQ_MAX; i++)
			if (entity->equipment[i])
				health_gained += entity->equipment[i]->health_on_focus;

		if (entity->health + health_gained > get_max_health(entity))
			health_gained = get_max_health(entity) - entity->health;

		snprintf(
			health_string, 64,
			GREEN " +%iHP" WHITE,
			health_gained
		);

		entity->health += health_gained;
	}
	else
	{
		health_gained = 0;

		snprintf(
			health_string, 64,
			MAGENTA BRIGHT "<recovery prevented>" NOCOLOR WHITE
		);
	}

	mana_gained = entity->class->mana_regen_on_focus;

	for (i = 0; i < EQ_MAX; i++)
		if (entity->equipment[i])
			mana_gained += entity->equipment[i]->mana_on_focus;

	if (entity->mana + mana_gained > get_max_mana(entity))
		mana_gained = get_max_mana(entity) - entity->mana;

	entity->mana += mana_gained;

	if (mana_gained)
		snprintf(
			mana_string, 64,
			BRIGHT BLUE " +%iMP" WHITE,
			mana_gained
		);
	else
		mana_string[0] = '\0';

	log = (char*) malloc(sizeof(char) * 128);
	snprintf(
		log, 128,
		BRIGHT WHITE "   " BLUE "<<< %s%s%s"
		NOCOLOR,
		health_string, mana_string, cure_string);
	logs_add(logs, log);
}

static void
use_item(Entity* entity, Item* item, Entity* enemy, Logs* logs)
{
	char* log;

	log = (char*) malloc(sizeof(char) * 128);
	snprintf(
		log, 128,
		BRIGHT WHITE "%s uses “%s” <" YELLOW "inventory" WHITE ">"
		NOCOLOR,
		entity->name, item->name
	);
	logs_add(logs, log);

	attack(entity, item->on_use, enemy, logs);
}

static void
ai_action(Game* game, Logs* logs)
{
	Entity* player;
	Entity* enemy;
	List* available_attacks;
	Attack* selected_attack;
	char* log;

	player = game->player;
	enemy = game->enemy;

	available_attacks = get_all_attacks(enemy);
	selected_attack = list_nth(
		available_attacks,
		rand() % list_size(available_attacks));

	if (can_use_attack(enemy, selected_attack))
	{
		log = (char*) malloc(sizeof(char) * 128);
		snprintf(log, 128,
			BRIGHT WHITE "%s used “%s”",
			enemy->name,
			selected_attack->name
		);
		logs_add(logs, log);

		attack(
			enemy,
			selected_attack,
			player,
			logs
		);
	}
	else
		focus(enemy, logs);
}

Logs*
command_use_item(Game* game, Entity* player, Item* item)
{
	Logs* logs;

	logs = logs_new();

	use_item(player, item, game->enemy, logs);

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
	char* log;

	logs = logs_new();

	player = game->player;
	enemy = game->enemy;

	log = (char*) malloc(sizeof(char) * 128);
	snprintf(log, 128,
		BRIGHT WHITE "%s uses “%s”",
		player->name,
		player_attack->name
	);
	logs_add(logs, log);

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
	char input = KEY_CLEAR;
	char info[128];
	Entity *player = game->player;
	Entity *enemy = game->enemy;
	List* player_attacks;
	List* list;
	int view = ATTACKS;
	int page = 0; /* Related to view. */
	int i;

	game->flee = 0;

	system("clear");

	logs = NULL;

	while (1)
	{
		if (!isexit(input))
		{
			info[0] = '\0';

			player_attacks = get_all_attacks(player);

			back_to_top();

			switch (input)
			{
				case KEY_CLEAR:
				case ' ':
					break;
				case 'f':
					if (logs)
						logs_free(logs);

					logs = command_focus(game);
					break;
				case 'l':
					if (logs)
						logs_free(logs);

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
					if (input > '0' && input <= '5')
					{
						input = input - '1';
						if (view == ATTACKS)
							if (list_size(player_attacks) > input)
							{
								Attack* attack =
									list_nth(player_attacks, input);

								if (!can_use_attack(player, attack))
								{
									snprintf(info, 128,
										BRIGHT RED " >> " WHITE "Not enough mana...");
								}
								else
								{
									if (logs)
										logs_free(logs);

									logs = command_attack(game, attack);
								}
							}
							else
							{
								snprintf(
									info, 128, BRIGHT RED " >> " WHITE "No such attack..."
								);
							}
						else if (view == ITEMS)
						{
							Item* item;
							int index = input + page * 5;

							if (index < INVENTORY_SIZE &&
								(item = player->inventory[index].item))
							{
								if (logs)
									logs_free(logs);

								logs = command_use_item(game, player, item);

								if (item->consumable)
								{
									player->inventory[index].quantity -= 1;

									/* FIXME: Okay, make a separate function
									 * for items’ use. */
									if (player->inventory[index].quantity <= 0)
								   		player->inventory[index].item = NULL;
								}
							}
							else
								snprintf(
									info, 128,
									BRIGHT RED " >> " WHITE "No such item in inventory..."
								);
						}
					}
					else
						snprintf(
							info, 128,
							BRIGHT RED " >> " WHITE "Unrecognized key..."
						);
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
				print_items_menu(player, page);
			}

			back(5);
			move(40);
			printf(WHITE "  (f) focus\n" NOCOLOR);
			move(40);
			printf(WHITE "  (l) flee\n" NOCOLOR);

			if (view == ITEMS)
			{
				move(40);
				printf(WHITE "%-40s\n" NOCOLOR, "  (i) actions");
				move(40);
				printf(WHITE "%-40s\n" NOCOLOR, "  (+) next");
				move(40);
				printf(WHITE "%-40s\n" NOCOLOR, "  (-) previous");
			}
			else
			{
				move(40);
				printf(WHITE "  (i) use item\n" NOCOLOR);
				move(40);
				printf("%40s\n", "");
				move(40);
				printf("%40s\n", "");
			}

			menu_separator();

			for (i = 0; i < 80; i++)
				printf(" ");
			back(1);
			printf("\n");
			printf("%s", info);
			back(1);
			printf("\n");

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
				printf("\nYou lost half your gold.\n");
				printf("\nPress any key to continue...\n\n");
				getch();
				return -1;
			}
			else if (enemy->health <= 0)
			{
				player->gold += enemy->class->gold_on_kill;

				list = give_drop(player, enemy->class->drop);

				system("clear");
				printf("\nYou are VICTORIOUS.\n");
				printf("\nYou gain %dgp!\n",
					enemy->class->gold_on_kill);
				if (list)
				{
					printf("\nYou were able to loot the following items:\n");

					for (; list; list = list->next)
					{
						Item* item = list->data;

						printf(BRIGHT);
						if (item->slot >= 0)
							printf(WHITE);
						else if (item->on_use && item->on_use->strikes > 0)
							printf(YELLOW);
						else if (is_item_usable(item))
							printf(GREEN);

						printf("  - %s\n" NOCOLOR, item->name);
					}
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
	RandomEnemy* r;
	List* l = list;
	int max = 0;
	int selection;

	while (l)
	{
		r = l->data;

		max += r->frequency;

		l = l->next;
	}

	selection = rand() % max;
	for (l = list; l; l = l->next)
	{
		r = l->data;

		if (selection <= r->frequency - 1)
			return r->class;
		else
			selection -= r->frequency;
	}

	/* Should not happen. */
	return NULL;
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

	lower_skills_cooldown(player);

	system("clear");

	return NULL;
}

/* vim: set ts=4 sw=4 cc=80 : */
