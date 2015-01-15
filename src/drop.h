
#ifndef DROP_H
#define DROP_H

#include "list.h"
#include "items.h"
#include "entities.h"

typedef struct Drop {
	Item* item;
	int rarity;
	int quantity;

	/* Temporary container used at load-time. */
	char* item_name;
} Drop;

List* give_drop(struct Entity*, List*);

#endif

