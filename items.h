
#ifndef ITEMS_H
#define ITEMS_H

#include "types.h"
#include "list.h"

typedef struct Item {
	char* name;

	int slot; /* values <0 for non-equipment? */
	int price;

	int health_bonus;
	int mana_bonus;

	int attack_bonus;
	int defense_bonus;

	int type_resistance[TYPE_MAX];

	List* attacks;
} Item;

#include "battle.h"

Item* load_item(char*);
List* load_items(char*);

Item* get_item_by_name(List*, char*);
int get_count_from_inventory(Item**, Item*);

#endif

/* vim: set ts=4 sw=4 cc=80 : */
