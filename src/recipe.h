
#ifndef RECIPE_H
#define RECIPE_H

#include "list.h"
#include "items.h"
#include "game.h"

typedef struct Recipe {
	Item* output;
	List* ingredients; /* List* of Ingredient* */
	Skill* skill; /* char* before loading is complete */
	int level;

	/* Parser-loader data. */
	List* ingredients_names; /* List* of char* */
} Recipe;

typedef struct Ingredient {
	Item* item;
	int quantity;
} Ingredient;

void load_recipe(Game*, List*);

#endif

