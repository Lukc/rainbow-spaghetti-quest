
#ifndef ITEMS_EVENTS_H
#define ITEMS_EVENTS_H

#include "../events.h"
#include "../parser.h"

typedef struct {
	int type;
	char* name;

	Item* item;
	int quantity;
} GiveItemEvent;

typedef GiveItemEvent RemoveItemEvent;

Event* load_item_event(ParserElement*);
void fire_give_item_event(Game*, Event*);
void fire_remove_item_event(Game*, Event*);

#endif

