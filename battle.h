
#ifndef BATTLE_H
#define BATTLE_H

#include "list.h"

typedef struct Battle {
	struct Entity* player;
	struct Entity* enemy;
	List* classes;
	List* items;
	struct Place* location;

	int flee;
} Battle;

#include "entities.h"
#include "items.h"
#include "commands.h"
#include "places.h"

struct Logs* enter_battle(void*);

#endif

