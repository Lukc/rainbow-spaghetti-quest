
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

	int unique;
	int consumable;

	int health_on_use;
	int mana_on_use;

	int health_on_focus;
	int mana_on_focus;

	List* attacks;
} Item;

typedef struct ItemStack {
	int quantity;
	Item* item;
} ItemStack;

#include "game.h"
#include "entities.h"

Item* load_item(Game*, char*);
List* load_items(Game*, char*);

Item* get_item_by_name(List*, char*);
int get_count_from_inventory(ItemStack*, Item*);

int give_item(struct Entity*, Item*);

int is_item_usable(Item*);

void print_items_menu(struct Entity*, int);

#endif

/* vim: set ts=4 sw=4 cc=80 : */
