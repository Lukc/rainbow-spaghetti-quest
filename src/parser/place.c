#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../destinations.h"
#include "../enemies.h"

#include "place.h"
#include "condition.h"
#include "drop.h"

void
parser_load_place(Game* game, Place* place)
{
	List* sl;
	List* next;
	List** prev;

	prev = &place->destinations;
	for (sl = place->destinations; sl; sl = next)
	{
		Destination* dest;
		Place* dest_place;

		next = sl->next;

		dest = sl->data;
		if (!strcmp(dest->name, place->name))
		{
			*prev = sl->next;

			free(dest->name);
			free(dest);

			/* Movements from A to A are silently ignored. */
			continue;
		}

		dest_place = get_place_by_name(game->places, dest->name);

		if (dest_place)
		{
			dest->place = dest_place;

			parser_load_condition(game, &dest->condition);
		}
		else
		{
			fprintf(stderr, "Unknown place: %s\n", dest->name);
			exit(1);
		}

		prev = &sl->next;
	}

	for (sl = place->skill_drops; sl; sl = sl->next)
	{
		char* name;
		SkillDrops* sd = sl->data;

		name = (char*) sd->skill;

		sd->skill = get_skill_by_name(game->skills, name);

		if (sd->skill)
		{
			List* ssl;

			for (ssl = sd->drops; ssl; ssl = ssl->next)
			{
				parser_load_drop(game, (Drop*) ssl->data);
			}
		}
		else
		{
			fprintf(stderr, "[Place/%s] Undefined skill: %s.\n",
				place->name, name);
			exit(1);
		}
	}

	for (sl = place->shop_item_names; sl; sl = sl->next)
	{
		char* name = sl->data;
		Item* item = get_item_by_name(game->items, name);

		if (item)
			list_add(&place->shop_items, item);
		else
			fprintf(stderr, "[Place:%s/Shop Items] Unknown item: %s.\n",
				place->name, name);
	}

	for (sl = place->random_enemies; sl; sl = sl->next)
	{
		RandomEnemy* r = sl->data;
		char* name = (char*) r->class;
		Class* class = get_class_by_name(game->classes, name);

		if (class)
		{
			r->class = class;

			free(name);
		}
		else
		{
			fprintf(stderr, "[Place:%s/Enemies] Unknown class: %s.\n",
				place->name, name);

			free(name);

			exit(0);
		}
			
	}
}

/* vim: set ts=4 sw=4 cc=80 : */

