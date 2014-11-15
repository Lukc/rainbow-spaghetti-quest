
#ifndef SHOP_H
#define SHOP_H

#include "items.h"
#include "entities.h"
#include "commands.h"
#include "game.h"

typedef struct {
	Entity* player;
	Item* items;
} Shop;

Logs* enter_shop(Game*);

char* sell_item(Entity* player, Item* item);
char* equip_item(Entity* player, Item* item);
char* unequip_item(Entity* player, Item* item);

void print_equipment(Entity*);

#endif

