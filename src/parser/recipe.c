#include <stdlib.h>
#include <stdio.h>

#include "recipe.h"

void
parser_load_recipe(Game* game, Recipe* recipe)
{
	List* sl;

	recipe->output = get_item_by_name(game->items, (char*) recipe->output);

	if (!recipe->output)
	{
		fprintf(stderr,
			"[Recipe:??] Item “%s” does not exist!\n", (char*) recipe->output);

		exit(1);
	}

	if (recipe->skill)
	{
		char* name = (char*) recipe->skill;

		if (!(recipe->skill = get_skill_by_name(game->skills, name)))
		{
			fprintf(stderr, "[Recipe/%s] Undefined skill: %s.\n",
				recipe->output->name, name);
			exit(1);
		}
	}

	for (sl = recipe->ingredients; sl; sl = sl->next)
	{
		Ingredient* ig = sl->data;
		Item* item = get_item_by_name(game->items, (char*) ig->item);

		if (item)
			ig->item = item;
		else
		{
			fprintf(stderr, "[Recipe:%s] Unknown item: %s\n",
				recipe->output->name, (char*) ig->item);

			exit(1);
		}
	}
}

