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
#include "events.h"

#include "battle/attack.h"
#include "battle/focus.h"
#include "battle/flee.h"
#include "battle/items.h"
#include "battle/mana_cost.h"

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

			if (item->slot >= 0)
				fg(WHITE);
			else if (item->on_use && item->on_use->strikes > 0)
				fg(YELLOW);
			else if (is_item_usable(item))
				fg(GREEN);

			printf("  - %s\n", item->name);
		}

		printf("\nPress any key to continue...\n\n");
	}

	getch();
}

/**
 * Prints the attacks’ selection menu of the battle interface.
 * @param list: The List* of AttackData* to display.
 */
static void
print_attacks(Entity* player, List* list, int selection)
{
	int i;
	int begin;
	int can_use;

	begin = selection - selection % 5;

	for (i = 0; i < begin; i++)
		list = list->next;

	for (i = 0; i < 5; i++)
	{
		AttackData* ad;
		Attack* attack;

		if (list)
		{
			ad = list->data;
			attack = ad->attack;

			can_use = can_use_attack(player, ad);

			if (i == selection % 5)
			{
				bg(WHITE);
				fg(BLACK);
			}
			else
			{
				if (can_use > 0)
					fg(WHITE);
				else
					bg(BLACK);
			}

			printf(" %-37s ", attack->name);

			move(27);

			if (can_use > 0)
			{
				if (ad->charge < attack->charge)
				{
					printf(" --  ");
					fg(YELLOW);
					printf("%i/%i", ad->charge, attack->charge);
					fg(WHITE);
					printf("  --");
				}
				else
				{
					if (attack->charge)
						fg(YELLOW);

					printf(" -- READY --");
				}
			}
			else if (can_use == E_COOLDOWN)
				printf(" -- CDN %i --", ad->cooldown);
			else if (can_use == E_NO_MANA)
				printf(" -- NO MP --");
			else if (can_use == E_NO_HEALTH)
				printf(" -- NO HP --");

			printf("\n");
			nocolor();

			list = list->next;
		}
		else
		{
			fg(BLACK);
			printf(" ------------ \n");
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
				bg(WHITE);
			}

			if ((item = player->inventory[i].item))
			{
				if (is_item_usable(item))
				{
					if (item->consumable)
						fg(GREEN);
					else
						fg(BLUE);
				}
				else
				{
					if (i == selection)
						fg(GRAY);
					else
						fg(BLACK);
				}

				if (player->inventory[i].quantity > 1)
					printf(" %ix %-37s\n",
						player->inventory[i].quantity, item->name);
				else
					printf(" %-39s\n", item->name);
			}
			else
			{
				int j;

				fg(BLACK);
				printf(" ");
				for (j = 0; j < 37; j++)
					printf("-");
				printf(" \n");
			}
		}
		else
			printf("\n");

		nocolor();
	}
}

static void
print_battle_logs(Game* game, Logs* logs)
{
	List* list;
	int i;

	print_entity_basestats(game->player);
	printf("\n");
	print_entity_basestats(game->enemy);
	menu_separator();

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

			printf("%s\n", (char*) list->data);

			list = list->next;
		}
	}
}

static void
print_attack_stats(Attack* attack, Entity* player)
{
	int first = 1;

	menu_separator();

	printf("%80s", "");
	move(0);

	if (attack->mana)
	{
		int mana = get_mana_cost(player, attack);
		if (attack->mana > 0)
			fg(BLUE);
		else if (attack->mana < mana)
			fg(RED);
		else
			fg(WHITE);

		printf("%+iMP", mana);
		nocolor();

		first = 0;
	}

	if (attack->health)
	{
		if (first)
			first = 0;
		else
			printf(", ");

		if (attack->health > 0)
			fg(GREEN);
		else
			fg(RED);

		printf("%+iHP", attack->health);
		nocolor();
	}

	if (attack->cooldown)
	{
		if (first)
			first = 0;
		else
			printf(", ");

		fg(WHITE);
		printf("cooldown: %i", attack->cooldown);
		nocolor();
	}

	if (attack->strikes)
	{
		if (first)
			first = 0;
		else
			printf(", ");

		fg(WHITE);
		printf("(%i-%i)x%i", attack->damage.min, attack->damage.max, attack->strikes);
		printf(" %s damage", type_to_string(attack->type));
	}

	if (attack->inflicts_status)
	{
		nocolor();
		printf(", ");
		fg(MAGENTA);
		printf("inflicts %s", attack->inflicts_status->name);
	}

	if (attack->self_inflicts_status)
	{
		nocolor();
		printf(", ");
		fg(MAGENTA);
		printf("self-inflicts %s", attack->self_inflicts_status->name);
	}

	if (attack->cures_statuses)
	{
		List* l;

		for (l = attack->cures_statuses; l; l = l->next)
		{
			Status* status = l->data;

			nocolor();
			printf(", ");
			fg(CYAN);
			printf("cures %s", status->name);
		}
	}

	if (attack->charge)
	{
		nocolor();
		printf(", ");
		fg(YELLOW);

		if (attack->charge == 1)
			printf("charges 1 turn");
		else
			printf("charges %i turns", attack->charge);
	}

	printf("\n");
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
							attack_index < list_size(player->attacks) - 1 ?
								attack_index + 1 : attack_index;
					else
						item_index = item_index < INVENTORY_SIZE - 1 ?
							item_index + 1 : item_index;
					break;
				case 'u':
					view = LOGS;

					if (menu == ATTACKS)
					{
						AttackData* ad;
						
						ad = list_nth(player->attacks, attack_index);

						if (can_use_attack(player, ad) < 1)
						{
							snprintf(info, 128,
								" >> Not enough mana/health or cooldown issue...");
						}
						else
						{
							if (logs)
								logs_free(logs);

							logs = command_attack(game, ad);
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
									info, 128, " >> Item cannot be used."
								);
						}
						else
							snprintf(
								info, 128, " >> Inventory slot empty."
							);
					}
					break;
				default:
					snprintf(
						info, 128, " >> Unrecognized key..."
					);
			}

			if (enemy->class->start_turn_events)
				fire_events(game, enemy->class->start_turn_events);

			for (i = 0; i < 16; i++)
			{
				int j;
				for (j = 0; j < 80; j++)
					printf(" ");
				printf("\n");
			}
			back(16);

			if (view == LOGS)
			{
				print_battle_logs(game, logs);

				if (menu == ATTACKS)
				{
					AttackData* ad = list_nth(player->attacks, attack_index);
					print_attack_stats(ad->attack, player);
				}
				else
				{
					Item* item = player->inventory[item_index].item;

					if (item && is_item_usable(item))
						print_attack_stats(
							player->inventory[item_index].item->on_use,
							player
						);
					else
					{
						menu_separator();
						printf("%-80s\n", "Item cannot be used in battle.");
					}
				}
			}
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
					print_attacks(player, player->attacks, attack_index);
				else if (menu == ITEMS)
					print_items_menu(player, item_index);

				fg(WHITE);

				back(5);
				move(40);
				printf("  (u) %-14s",
					menu == ATTACKS ? "attack" : "use item");
				move(60);
				fg(YELLOW);
				printf("  (v) %-14s\n",
					menu == ATTACKS ? "view attack" : "view items");
				move(40);
				fg(WHITE);
				printf("  (f) focus");
				move(60);
				printf("  (d) view %-9s\n",
					view == LOGS ? "enemy" :
					view == ENEMY ? "player" : "battle");

				move(40);
				if (menu == ITEMS)
					printf("%-40s", "  (i) view actions");
				else
					printf("  (i) view items");
				move(60);
				printf("  (l) flee\n");

				move(40);
				printf("%40s\n", "");

				move(40);
				printf("%40s", "");
				move(40);
				printf("    %i/%iHP", player->health, get_max_health(player));
				move(60);
				printf("    %i/%iMP\n", player->mana, get_max_mana(player));
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
	SpawnData* spawn;
	List* l = list;
	int max = 0;
	int selection;

	while (l)
	{
		spawn = l->data;

		max += spawn->frequency;

		l = l->next;
	}

	selection = rand() % max;
	for (l = list; l; l = l->next)
	{
		spawn = l->data;

		if (selection <= spawn->frequency - 1)
			return spawn->class;
		else
			selection -= spawn->frequency;
	}

	/* Should not happen. */
	return NULL;
}

void
prepare_for_battle(Entity* e)
{
	List* attacks;
	AttackData* ad;

	e->attacks = NULL;
	attacks = list_rev_and_free(get_all_attacks(e));

	for (; attacks; attacks = attacks->next)
	{
		Attack* attack = attacks->data;

		ad = malloc(sizeof(*ad));

		ad->attack = attack;
		ad->cooldown = 0;
		ad->charge = 0;

		list_add(&e->attacks, ad);
	}

	list_free(attacks, NULL);
}

void
enter_battle(Game* game)
{
	Class* enemy_class = NULL;
	Entity* player, *enemy;
	int result;

	player = game->player;
	enemy = game->enemy;

	if (game->location->random_enemies)
		enemy_class =
			get_random_enemy(game->location->random_enemies);
	else
		/* Should not happen, to begin with. */
		return;

	init_entity_from_class(enemy, enemy_class);

	prepare_for_battle(player);
	prepare_for_battle(enemy);

	if ((result = battle(game)) == 1)
	{
		player->kills++;
	}

	lower_skills_cooldown(game);

	system("clear");
}

/* vim: set ts=4 sw=4 cc=80 : */
