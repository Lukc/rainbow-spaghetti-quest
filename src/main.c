#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>

#include "types.h"
#include "commands.h"
#include "colors.h"
#include "entities.h"
#include "items.h"
#include "battle.h"
#include "game.h"
#include "shop.h"
#include "places.h"
#include "term.h"
#include "inventory.h"
#include "travel.h"
#include "images.h"
#include "craft.h"
#include "skills.h"
#include "parser.h"

void
save(Game* game, char* filename)
{
	Entity* player = game->player;
	FILE* f;
	List* l;
	int i;

	f = fopen(filename, "w");

	if (!f)
	{
		system("clear");
		fprintf(stderr, "Error while opening save file:\n");
		perror(" >> fopen");

		fprintf(stderr, "Save file will be dumped to stdout. Just in case.\n");
		fprintf(stderr, "Press any key to continue.\n");
		getch();

		f = stdout;
	}

	fprintf(f, "Money: %i\n", player->gold);
	fprintf(f, "Location: %s\n", game->location->name);

	for (l = game->visited; l; l = l->next)
		fprintf(f, "Visited Place: %s\n", ((Place*) l->data)->name);

	for (i = 0; i < EQ_MAX; i++)
	{
		Item* eq = player->equipment[i];

		if (eq)
			fprintf(f, "Equipment: %s\n", eq->name);
	}

	for (i = 0; i < INVENTORY_SIZE; i++)
	{
		ItemStack* stack = player->inventory + i;

		if (stack->item)
		{
			fprintf(f,
				"Inventory: [\n\tItem: %s\n\tQuantity: %i\n]\n",
				player->inventory[i].item->name,
				player->inventory[i].quantity
			);
		}
	}

	for (i = 0; i < SKILL_MAX; i++)
	{
		fprintf(f,
			"%s cooldown: %i\n",
			skill_to_string(i),
			player->skills_cooldown[i]
		);
	}
}

static int
load_cooldown(Game* game, ParserElement* element, Logs* logs)
{
	int i;

	for (i = 0; i < SKILL_MAX; i++)
	{
		char* skill = skill_to_string(i);
		size_t len = strlen(skill);

		if (
			!strncmp(element->name, skill, len) &&
			!strcmp(element->name + len, " cooldown")
		)
		{
			game->player->skills_cooldown[i] = parser_get_integer(element, logs);

			return 1;
		}
	}

	return 0;
}

static void
load(Game* game, char* filename)
{
	List* elements;
	List* l;
	ParserElement* element;

	elements = parse_file(filename);

	if (!elements)
		return;

	for (l = elements; l; l = l->next)
	{
		char* field;

		element = l->data;
		field = element->name;

		if (!strcmp(field, "money"))
			game->player->gold = parser_get_integer(element, NULL);
		else if (!strcmp(field, "visited place"))
			list_add(&game->visited, get_place_by_name(game->places, parser_get_string(element, NULL)));
		else if (!strcmp(field, "location"))
			game->location = get_place_by_name(game->places, parser_get_string(element, NULL));
		else if (!strcmp(field, "inventory"))
		{
			List* sl = element->value;
			Item* item = NULL;
			int quantity = 0;
			int i;

			if (element->type == PARSER_LIST)
			{
				for (; sl; sl = sl->next)
				{
					element = sl->data;

					if (!strcmp(element->name, "item"))
						item = get_item_by_name(
							game->items, parser_get_string(element, NULL));
					else if (!strcmp(element->name, "quantity"))
						quantity = parser_get_integer(element, NULL);
				}

				if (item && quantity)
					/* Okay… having a loop for *THIS* definitely sucks. */
					for (i = 0; i < quantity; i++)
						give_item(game->player, item);
			}
			else
				fprintf(stderr, "Inventory item ignored because not a list.\n");
		}
		else if (!strcmp(field, "equipment"))
		{
			Item* item = get_item_by_name(game->items, parser_get_string(element, NULL));

			game->player->equipment[item->slot] = item;
		}
		else if (load_cooldown(game, element, NULL))
			;
		else
			fprintf(stderr, "Ignored field in save-file: %s\n", field);
	}
}

static void
print_menu(Game* game)
{
	Place* location = game->location;

	menu_separator();

	printf(
		WHITE
		"%s  (b) Random battle\n" NOCOLOR
		"%s  (s) Enter shop\n" NOCOLOR
		WHITE "  (i) Open inventory\n"
		BLACK
			"  (d) Enter dungeon\n"
		WHITE
		"  (t) Travel\n"
		NOCOLOR,
		location->random_enemies ? WHITE : BLACK,
		location->shop_items ? WHITE : BLACK
	);

	back(5);
	move(40);
	fg(2, 1, 0);
	printf(" (c) <WIP> Craft items\n" NOCOLOR);
	move(40);
	fg(2, 1, 0);
	printf(" (k) <WIP> Use skills\n" NOCOLOR);
	move(40);
	printf(BLACK " (q) Show quests\n" NOCOLOR);
	printf("\n");
	move(40);
	printf(BLACK " (S) Save game\n" NOCOLOR);

	menu_separator();
}

int
main(int argc, char* argv[])
{
	Entity player, enemy;
	char* error;
	int input;
	int i;
	Logs* logs;
	Game game;

	memset(&game, 0, sizeof(game));

	load_game(&game, "data");

	/* For now, repeatability would be useful */
	srand(42);

	init_entity_from_class(&player, get_class_by_name(game.classes, "Warrior"));

	player.name = (char*) malloc(sizeof(char) * 80);
	snprintf(
		player.name, 14, "%s the Cat",
		argc > 1 ? argv[1] : "Joe"
	);

	game.player = &player;
	game.enemy = &enemy;

	game.location = get_place_by_name(game.places, "Felinopolis");
	game.visited = NULL;

	load(&game, "rsq.save");

	getch();

	system("clear");
	input = -42;
	while (input == -42 || !isexit(input))
	{
		error = NULL;

		check_first_visit(&game);

		back_to_top();

		switch(input)
		{
			case 'b':
				logs = enter_battle(&game);
				break;
			case 's':
				if (game.location->shop_items)
					logs = enter_shop(&game);
				else
					error = "There is no shop at your current location!";
				break;
			case 'i':
				logs = NULL;
				inventory(&game);
				break;
			case 't':
				logs = NULL;
				travel(&game);
				break;
			case 'c':
				logs = NULL;
				craft(&game);
				break;
			case 'k':
				logs = NULL;
				skills(&game);
				break;

			/* Unimplemented stuff */
			case 'S':
			case 'd':
			case 'q':
				logs = NULL;
				break;
			case -42:
				break;
			default:
				error = "Unrecognized key.";
		}

		logs = NULL;

		player.health = get_max_health(&player);
		player.mana = get_max_mana(&player);
		remove_statuses(&player);

		if (game.location->image)
		{
			int out_of_boundaries = 0;
			char** image = game.location->image;

			for (i = 0; i < 16; i++)
				if (!out_of_boundaries && image[i])
					printf("%s\n", image[i]);
				else
				{
					out_of_boundaries = 1;

					printf("\n");
				}
		}
		else
		{
			print_entity_basestats(&player);
			printf("\n");

			if (logs)
			{
				logs_print(logs);
				logs_free(logs);
			}

			printf(
				"Gold: %i\n"
				"\n"
				,
				player.gold
			);

			printf("Current location: %s\n\n", game.location->name);

			for (i = 0; i < 8; i++)
				printf("\n");
		}

		print_menu(&game);

		if (error)
		{
			printf("%s", error);
			back(1);
			printf("\n");
		}

		input = getch();
	}

	system("stty sane");

	/* Saving game here? */
	save(&game, "rsq.save");

	return 0;
}

/* vim: set ts=4 sw=4 cc=80 : */
