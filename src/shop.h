
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

void print_item(Item*);
char* sell_item(Entity*, Item*);
char* equip_item(Entity*, Item*);
char* unequip_item(Entity*, Item*);

void print_equipment(Entity*);

#endif

