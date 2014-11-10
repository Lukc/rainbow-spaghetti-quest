
#ifndef SHOP_H
#define SHOP_H

#include "items.h"
#include "entities.h"
#include "commands.h"

typedef struct {
	Entity* player;
	Item* items;
} Shop;

Logs* enter_shop(void*);

#endif

