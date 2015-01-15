
#ifndef EVENTS_H
#define EVENTS_H

#include "game.h"
#include "list.h"
#include "conditions.h"

/* All Event structures must contain at least the following fields, in the
 * following order. You could see this as some sort of manually-managed
 * inheritance. */
typedef struct {
	int type;
	char* name;
} Event;

enum EVENT_TYPE {
	EVENT_MESSAGE,
	EVENT_CHOICE,
	EVENT_CONDITION,
	EVENT_GIVE_ITEM,
	EVENT_REMOVE_ITEM,
	EVENT_SET_VARIABLE,
	EVENT_FIRE
};

void load_events(Game*, List**, List*);
void fire_event (Game*, Event*);
void fire_events(Game*, List*);

#endif

