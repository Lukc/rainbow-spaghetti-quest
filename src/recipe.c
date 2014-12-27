#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "recipe.h"

#include "commands.h"
#include "parser.h"

static void
load_ingredient(char* recipe, Ingredient* ig, List* list, Logs* logs)
{
	ParserElement* element;
	char* field;

	ig->item = NULL;
	ig->quantity = 1;

	for (; list; list = list->next)
	{
		element = list->data;
		field = element->name;

		if (!strcmp(field, "item"))
			ig->item = (Item*) parser_get_string(element, logs);
		else if (!strcmp(field, "quantity"))
			ig->quantity = parser_get_integer(element, logs);
		else
			fprintf(
				stderr, "[Recipe:%s/Ingredient:%i] Invalid field ignored: %s.\n",
				recipe, element->lineno, field
			);
	}
}

void
load_recipe(Game* game, List* elements)
{
	List* list;
	Recipe* recipe;
	Logs* logs;

	logs = logs_new();

	recipe = (Recipe*) malloc(sizeof(Recipe));
	memset(recipe, 0, sizeof(Recipe));

	for (list = elements; list; list = list->next)
	{
		char* field;
		ParserElement* element;

		element = list->data;
		field = element->name;

		if (!strcmp(field, "output"))
			recipe->output = (Item*) parser_get_string(element, logs);
		else if (!strcmp(field, "ingredient"))
		{
			Ingredient* ingredient;

			if (element->type != PARSER_LIST)
			{
				fprintf(
					stderr, "[Recipe:%s:%i] Ingredient defined improperly.\n",
					recipe->output ? (char*) recipe->output : "??",
					element->lineno
				);
			}
			else
			{
				ingredient = malloc(sizeof(Ingredient));

				load_ingredient(
					recipe->output ? (char*) recipe->output : "??",
					ingredient, element->value, logs);

				list_add(&recipe->ingredients, ingredient);
			}
		}
		else
			fprintf(stderr,
				"[Recipe:%s:%i] Unknown field “%s”.\n",
				recipe->output ? (char*) recipe->output : "??", element->lineno, field);
	}

	list_add(&game->recipes, recipe);

	if (!logs_empty(logs))
		logs_print(logs);

	logs_free(logs);
}

