
#ifndef ITEMS_H
#define ITEMS_H

typedef struct {
	int id;
	char* name;

	int slot; /* values <0 for non-equipment? */
	int price;

	int attack_bonus;
	int defense_bonus;
} Item;

Item load_item(char*);
Item* load_items(char*);

#endif

/* vim: set ts=4 sw=4 cc=80 : */
