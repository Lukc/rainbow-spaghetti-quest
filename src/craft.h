
#ifndef CRAFT_H
#define CRAFT_H

#include "game.h"
#include "items.h"

#define CRAFT_MAX_X 9
#define CRAFT_MAX_Y 7

typedef struct CraftingGrid {
	Item* items[CRAFT_MAX_X][CRAFT_MAX_Y];
} CraftingGrid;

void craft(Game*);

#endif

