
#ifndef SHOP_H
#define SHOP_H

#include "items.h"
#include "entities.h"

typedef struct {
	Entity* player;
	Item* items;
} Shop;

char** enter_shop(void*);

#endif

