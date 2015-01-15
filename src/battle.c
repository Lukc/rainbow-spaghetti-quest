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

				if (-mana_cost > player->mana)
					fg(5, 5, 5);
				else
					fg(0, 0, 0);
			}
			else
			{
				if (-mana_cost > player->mana)
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

				if (attack->health)
				{
					if (attack->health > 0)
						printf(BRIGHT GREEN);
					else
						printf(BRIGHT RED);
					
					printf("%+3iHP %-9s" NOCOLOR,
						attack->health, "");
				}
				else
					printf("%15s" NOCOLOR, "");
			}

			if (i == selection % 5)
			{
				bg(4, 4, 4);
				fg(1, 1, 1);
			}

			if (mana_cost < attack->mana)
				printf("%s", RED);
			else if (mana_cost > 0) /* Uh uh, not really a cost, uh? */
				printf("%s", BLUE);

			printf(" %+3iMP", mana_cost);

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

	if ((result = battle(game)) == 1)
	{
		player->kills++;
	}

	lower_skills_cooldown(game);

	system("clear");
}

/* vim: set ts=4 sw=4 cc=80 : */
