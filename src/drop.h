
#ifndef DROP_H
#define DROP_H

#include "list.h"
#include "items.h"
#include "entities.h"

typedef struct Drop {
	Item* item;
	char* item_name; /* Temporary container used at load-time */
	int rarity;
} Drop;

List* give_drop(struct Entity*, List*);

#endif

