
#ifndef ITEMS_H
#define ITEMS_H

#include "types.h"

typedef struct Item {
	int id;
	char* name;

	int slot; /* values <0 for non-equipment? */
	int price;

	int attack_bonus;
	int defense_bonus;

	int defense[TYPE_MAX];

	int attack_type;
} Item;

#include "battle.h"

Item load_item(char*);
Item* load_items(char*);

Item* get_item_from_id(Battle*, int);
int get_count_from_inventory(int*, int);

#endif

/* vim: set ts=4 sw=4 cc=80 : */
