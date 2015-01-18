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
				bg(4, 4, 4);
				printf(BLACK);
			}
			else
			{
				if (can_use > 0)
					printf(WHITE);
				else
					fg(1, 1, 1);
			}

			printf(" %-37s ", attack->name);

			move(27);

			if (can_use > 0)
				printf(" -- READY --");
			else if (can_use == E_COOLDOWN)
				printf(" -- CDN %i --", ad->cooldown);
			else if (can_use == E_NO_MANA)
				printf(" -- NO MP --");
			else if (can_use == E_NO_HEALTH)
				printf(" -- NO HP --");

			printf("\n" NOCOLOR);

			list = list->next;
		}
		else
		{
			fg(1, 1, 1);
			printf(" ------------ \n" NOCOLOR);
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
			}

			if ((item = player->inventory[i].item))
			{
				if (is_item_usable(item))
				{
					if (item->consumable)
						printf(GREEN);
					else
						printf(BLUE);
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

				fg(1, 1, 1);
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

			printf("%s\n", list->data);

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
			printf(BLUE);
		else if (attack->mana < mana)
			printf(RED);
		else
			printf(WHITE);

		printf("%+iMP", mana);
		printf(NOCOLOR);

		first = 0;
	}

	if (attack->health)
	{
		if (first)
			first = 0;
		else
			printf(", ");

		if (attack->health > 0)
			printf(GREEN);
		else
			printf(RED);

		printf("%+iHP", attack->health);
		printf(NOCOLOR);
	}

	if (attack->cooldown)
	{
		if (first)
			first = 0;
		else
			printf(", ");

		printf(WHITE);
		printf("cooldown: %i", attack->cooldown);
		printf(NOCOLOR);
	}

	if (attack->strikes)
	{
		if (first)
			first = 0;
		else
			printf(", ");

		printf(WHITE);
		printf("(%i-%i)x%i", attack->damage, attack->damage, attack->strikes);
		printf(" %s damage", type_to_string(attack->type));
	}

	if (attack->inflicts_status)
	{
		printf(NOCOLOR);
		printf(", ");
		printf(MAGENTA);
		printf("inflicts %s", attack->inflicts_status->name);
	}

	if (attack->self_inflicts_status)
	{
		printf(NOCOLOR);
		printf(", ");
		printf(MAGENTA);
		printf("self-inflicts %s", attack->self_inflicts_status->name);
	}

	if (attack->cures_statuses)
	{
		List* l;

		for (l = attack->cures_statuses; l; l = l->next)
		{
			Status* status = l->data;

			printf(NOCOLOR);
			printf(", ");
			printf(CYAN);
			printf("cures %s", status->name);
		}
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
						Attack* attack;
						
						ad = list_nth(player->attacks, attack_index);
						attack = ad->attack;

						if (can_use_attack(player, ad) < 1)
						{
							snprintf(info, 128,
								BRIGHT RED " >> " WHITE "Not enough mana/health or cooldown issue...");
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

		list_add(&e->attacks, ad);
	}

	list_free(attacks, NULL);
}

void
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
