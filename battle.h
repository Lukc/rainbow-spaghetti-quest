
#ifndef BATTLE_H
#define BATTLE_H

typedef struct Battle {
	struct Entity *player;
	struct Entity *enemy;
	struct Class *classes;
	struct Item *items;

	int flee;
} Battle;

#include "entities.h"
#include "items.h"
#include "commands.h"

struct Logs* enter_battle(void*);

#endif

