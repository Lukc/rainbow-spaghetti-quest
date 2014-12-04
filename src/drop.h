
#ifndef DROP_H
#define DROP_H

#include "list.h"
#include "items.h"
#include "entities.h"

typedef struct Drop {
	Item* item;
	int rarity;
} Drop;

List* give_drop(struct Entity*, List*);

#endif

