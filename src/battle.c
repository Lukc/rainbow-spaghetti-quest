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

/**
 * @param list: List* of Item*
 */
static void
loot_screen(List* list)
{
	if (list)
	{
		getch();
		system("clear");
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

		printf("\nPress any key to continue...\n\n");
	}

	getch();
}

static void
end_turn(Entity* e, Logs* logs)
{
	List* l;
	StatusData* sd;
	Status* status;
	int health_lost = 0;

	for (l = e->statuses; l; l = l->next)
	{
		sd = l->data;
		status = sd->status;

		if (status->removes_health)
		{
			health_lost += get_max_health(e) / status->removes_health;
		}
	}

	if (health_lost)
	{
		char* log;

		e->health -= health_lost;

		log = malloc(sizeof(char) * 128);
		snprintf(log, 128, BRIGHT MAGENTA " <<< " RED "%+iHP"
			MAGENTA " (statuses)",
			-health_lost);
		logs_add(logs, log);
	}
}

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
print_attacks(Entity* player, List* list, int selection)
{
	int i;
	int begin;

	begin = selection - selection % 5;

	for (i = 0; i < begin; i++)
		list = list->next;

	for (i = 0; i < 5; i++)
	{
		Attack* attack = NULL;
		
		if (list)
		{
			int mana_cost;
			char* name;

			attack = list->data;

			mana_cost = get_mana_cost(player, attack);

			if (attack->name)
				name = strdup(attack->name);
			else
				name = strdup(type_to_attack_name(attack->type));

			if (strlen(name) > 13)
				name[14] = '\0';

			if (i == selection % 5)
			{
				bg(4, 4, 4);

				if (mana_cost > player->mana)
					fg(5, 5, 5);
				else
					fg(0, 0, 0);
			}
			else
			{
				if (mana_cost > player->mana)
					fg(1, 1, 1);
				else
					fg(4, 4, 4);
			}


			/* name   damage-strikes damage_type */
			/*  ... OR ...  */
			/* name */
			if (attack->strikes)
			{
				char string[128];
				char damage_string[7];
				size_t len;

				snprintf(string, 128, " %-20s", attack->name);

				len = snprintf(damage_string, 7, "%i-%i",
					attack->damage + get_attack_bonus(player), attack->strikes
				);

				string[21 - len] = ' ';
				snprintf(string + 22 - len, 128 - 22 + len,
					"%s", damage_string);

				printf("%s %-10s" NOCOLOR,
					string, type_to_string(attack->type));
			}
			else
			{
				printf(" %-14s   ", name);

				if (attack->gives_health)
				{
					if (attack->gives_health > 0)
						printf(BRIGHT GREEN);
					else
						printf(BRIGHT RED);
					
					printf("%+3iHP %-9s" NOCOLOR,
						attack->gives_health, "");
				}
				else
					printf("%15s" NOCOLOR, "");
			}

			if (i == selection % 5)
			{
				bg(4, 4, 4);
				fg(1, 1, 1);
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
			printf(BLACK " ------------ \n" NOCOLOR);
		}
	}
}

/**
 * Prints the items’ selection menu of the battle interface.
 *
 * @todo: Print the effects of using the item somewhere...
 */
static void
print_items_menu(Entity* player, int selection)
{
	int i;
	int begin;

	begin = selection - selection % 5;

	for (i = begin; i < begin + 5; i++)
	{
		if (i < INVENTORY_SIZE)
		{
			Item* item;

			if (i == selection)
			{
				bg(4, 4, 4);
				fg(0, 0, 0);
			}

			if ((item = player->inventory[i].item))
			{
				if (is_item_usable(item))
				{
					if (item->consumable)
						printf(GREEN);
				}
				else
				{
					if (i == selection)
						fg(2, 2, 2);
					else
						fg(1, 1, 1);
				}

				if (player->inventory[i].quantity > 1)
					printf(" %ix %-37s\n" NOCOLOR,
						player->inventory[i].quantity, item->name);
				else
					printf(" %-39s\n" NOCOLOR, item->name);
			}
			else
			{
				int j;

				printf(" ");
				for (j = 0; j < 37; j++)
					printf("-");
				printf(" \n" NOCOLOR);
			}
		}
		else
			printf("\n");
	}
}

static void
print_battle_logs(Game* game, Logs* logs)
{
	List* list;
	int i;

	print_entity_basestats(game->player);
	printf(BRIGHT RED "\n -- " WHITE "versus" RED " --\n\n" NOCOLOR);
	print_entity_basestats(game->enemy);
	printf("\n");

	if (logs)
		list = logs->head;
	else
		list = NULL;

	for (i = 0; i < 6; i++)
	{
		int j;

		/* Clearing each line of the logs area before using it. */
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

	if (player->health > 0)
	{
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

	end_turn(enemy, logs);
}

Logs*
command_use_item(Game* game, Entity* player, Item* item)
{
	Logs* logs;

	logs = logs_new();

	use_item(player, item, game->enemy, logs);

	end_turn(player, logs);

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
		end_turn(game->player, logs);

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

	end_turn(game->player, logs);

	ai_action(game, logs);

	return logs;
}

#define ATTACKS 0
#define ITEMS 1

#define LOGS 0
#define ENEMY 1
#define PLAYER 2

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
	int view = LOGS;
	int menu = ATTACKS; /* Whether the player is choosing an attack or item. */
	int attack_index = 0; /* Index of the selected attack in the list. */
	int item_index = 0;   /* Index of the selected item in the inventory */
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
				case 'd':
					view =
						view == LOGS ? ENEMY :
						view == ENEMY ? PLAYER :
						LOGS;
					break;
				case 'i':
					menu = !menu;
					break;
				case KEY_UP:
					if (menu == ATTACKS)
						attack_index = attack_index > 0 ? attack_index - 1 : 0;
					else
						item_index = item_index > 0 ? item_index - 1 : 0; 
					break;
				case KEY_DOWN:
					if (menu == ATTACKS)
						attack_index =
							attack_index < list_size(player_attacks) - 1 ?
								attack_index + 1 : attack_index;
					else
						item_index = item_index < INVENTORY_SIZE - 1 ?
							item_index + 1 : item_index;
					break;
				case 'u':
					view = LOGS;

					if (menu == ATTACKS)
					{
						Attack* attack =
							list_nth(player_attacks, attack_index);

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
					else if (menu == ITEMS)
					{
						Item* item;

						if (item_index < INVENTORY_SIZE &&
							(item = player->inventory[item_index].item))
						{
							if (is_item_usable(item))
							{
								if (logs)
									logs_free(logs);

								logs = command_use_item(game, player, item);

								if (item->consumable)
								{
									player->inventory[item_index].quantity -= 1;

									if (player->inventory[item_index].quantity <= 0)
										player->inventory[item_index].item = NULL;
								}
							}
							else
								snprintf(
									info, 128,
									BRIGHT RED " >> " WHITE
										"Item cannot be used."
								);
						}
						else
							snprintf(
								info, 128,
								BRIGHT RED " >> " WHITE
									"Inventory slot empty."
							);
					}
					break;
				default:
					snprintf(
						info, 128,
						BRIGHT RED " >> " WHITE "Unrecognized key..."
					);
			}

			for (i = 0; i < 16; i++)
			{
				int j;
				for (j = 0; j < 80; j++)
					printf(" ");
				printf("\n");
			}
			back(16);

			if (view == LOGS)
				print_battle_logs(game, logs);
			else if (view == ENEMY)
				print_entity(game->enemy);
			else if (view == PLAYER)
				print_entity(game->player);

			menu_separator();

			for (i = 0; i < 5; i++)
			{
				int j;
				for (j = 0; j < 80; j++)
					printf(" ");
				printf("\n");
			}
			back(5);

			/* FIXME: The info line is currently not updated when
			 *        you flee/lose/win. */
			if (game->flee)
			{
				printf("You manage to leave your fight without being "
					"hurt too badly.\n");
				printf("\nPress any key to continue...\n\n");
				getch();
				return 0;
			}
			else if (player->health <= 0)
			{
				player->gold /= 2;

				printf("You are DEAD.\n");
				printf("\nYou lost half your gold.\n");
				printf("\nPress any key to continue...\n\n");
				getch();
				return -1;
			}
			else if (enemy->health <= 0)
			{
				player->gold += enemy->class->gold_on_kill;

				list = give_drop(player, enemy->class->drop);

				printf("You are VICTORIOUS.\n");
				printf("\nYou gain %dgp!\n",
					enemy->class->gold_on_kill);
				printf("\nPress any key to continue...\n\n");

				loot_screen(list);

				return 1;
			}
			else
			{
				/* The battle continues! o/ */
				if (menu == ATTACKS)
					print_attacks(player, player_attacks, attack_index);
				else if (menu == ITEMS)
					print_items_menu(player, item_index);

				back(5);
				move(40);
				printf(WHITE "  (u) %-14s" NOCOLOR,
					menu == ATTACKS ? "attack" : "use item");
				move(60);
				fg(4, 3, 0);
				printf("  (v) %-14s\n",
					menu == ATTACKS ? "view attack" : "view items");
				move(40);
				printf(WHITE "  (f) focus" NOCOLOR);
				move(60);
				printf(WHITE "  (d) view %-9s\n",
					view == LOGS ? "enemy" :
					view == ENEMY ? "player" : "battle");

				move(40);
				if (menu == ITEMS)
					printf(WHITE "%-40s" NOCOLOR, "  (i) view actions");
				else
					printf(WHITE "  (i) view items" NOCOLOR);
				move(60);
				printf(WHITE "  (l) flee\n");

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

			input = getch();
		}
		else
			input = getch();
	}

	exit(0);

	return 0;
}

#undef LOGS
#undef ENEMY

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

	lower_skills_cooldown(game);

	system("clear");

	return NULL;
}

/* vim: set ts=4 sw=4 cc=80 : */
