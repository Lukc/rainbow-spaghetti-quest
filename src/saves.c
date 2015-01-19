#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include "string.h"
#include "characters.h"
#include "term.h"
#include "game.h"

static char*
get_savefile_name(int n)
{
	char* datadir;
	char* savesdir;
	char* savefile;

	datadir = getenv("XDG_DATA_HOME");
	if (!datadir)
	{
		char* homedir = getenv("HOME");

		if (homedir)
		{
			datadir = malloc(strlen(homedir) + strlen("/.local/share") + 1);
			sprintf(datadir, "%s/.local/share", homedir);
		}
		else
		{
			/* Uh? You’re screwed, man. */
			datadir = strdup(".");
		}
	}

	savesdir = malloc(strlen(datadir) + strlen("/rsq") + 1);
	sprintf(savesdir, "%s/rsq", datadir);

	if (mkdir(savesdir, 0755) == -1 && errno != EEXIST)
		perror("mkdir");

	/* 16 characters should be enough for pretty big numbers, right? */
	savefile = malloc(strlen(savesdir) + 2 + 16);
	sprintf(savefile, "%s/%i", savesdir, n);

	return savefile;
}

void
save(Game* game, int slot)
{
	Entity* player = game->player;
	char* filename;
	FILE* f;
	List* l;
	int i;

	filename = get_savefile_name(slot);

	f = fopen(filename, "w");

	if (!f)
	{
		system("clear");
		fprintf(stderr, "Error while opening save file:\n");
		perror(" >> fopen");

		fprintf(stderr, "Save file will be dumped to stdout. Just in case.\n");
		fprintf(stderr, "(but it is NOT going to be saved!)\n");
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

	for (l = game->skills; l; l = l->next)
	{
		Skill* skill = l->data;

		fprintf(f, "Skill: [\n\tName: %s\n", skill->name);

		if (skill->cooldown)
			fprintf(f,
				"\tCooldown: %i\n", skill->cooldown
			);

		if (skill->experience)
			fprintf(f,
				"\tExperience: %i\n", skill->experience
			);

		fprintf(f, "]\n");
	}

	for (l = game->variables; l; l = l->next)
	{
		Variable* v = l->data;

		fprintf(f, "Variable: [\n"
			"\tName: %s\n"
			"\tValue: %i\n"
			"]\n",
			v->name,
			v->value
		);
	}

	free(filename);
}

void
load(Game* game, int slot)
{
	List* elements;
	List* l;
	ParserElement* element;
	char* filename;

	filename = get_savefile_name(slot);

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
		else if (!strcmp(field, "skill"))
		{
			List* sl = element->value;
			char* name;
			int experience = 0;
			int cooldown = 0;

			for (; sl; sl = sl->next)
			{
				element = sl->data;

				if (!strcmp(element->name, "name"))
					name = parser_get_string(element, NULL);
				else if (!strcmp(element->name, "experience"))
					experience = parser_get_integer(element, NULL);
				else if (!strcmp(element->name, "cooldown"))
					cooldown = parser_get_integer(element, NULL);
				else
					fprintf(stderr, "[Skill:%i] Ignored property: %s.\n",
						element->lineno, element->name);
			}

			if (name)
			{
				Skill* skill = get_skill_by_name(game->skills, name);

				if (skill)
				{
					skill->experience = experience;
					skill->cooldown = cooldown;
				}
				else
					fprintf(stderr,
						"Uh. Broken save file? (invalid Skill->Name)\n");
			}
			else
				fprintf(stderr,
					"Uh. Broken save file? (missing Skill->Name)\n");
		}
		else if (!strcmp(field, "variable"))
		{
			List* sl = element->value;
			char* name = NULL;
			int value = 0;

			for (; sl; sl = sl->next)
			{
				element = sl->data;

				if (!strcmp(element->name, "name"))
					name = parser_get_string(element, NULL);
				else if (!strcmp(element->name, "value"))
					value = parser_get_integer(element, NULL);
				else
					fprintf(stderr, "[Variable:%i] Ignored property: %s.\n",
						element->lineno, element->name);
			}

			if (name)
			{
				Variable* v;

				v = malloc(sizeof(*v));
				v->name = name;
				v->value = value;

				list_add(&game->variables, v);
			}
		}
		else
			fprintf(stderr, "Ignored field in save-file: %s\n", field);
	}
}

/* vim: set ts=4 sw=4 cc=80 : */
