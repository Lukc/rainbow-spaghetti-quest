
#ifndef BATTLE_H
#define BATTLE_H

#include "entities.h"
#include "items.h"

typedef struct {
	Entity *player;
	Entity *enemy;
	Class *classes;
	Item *items;
} Battle;

char** enter_battle(void*);

#endif

