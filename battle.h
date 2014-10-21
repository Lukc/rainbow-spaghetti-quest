
#ifndef BATTLE_H
#define BATTLE_H

#include "entities.h"

typedef struct {
	Entity *player;
	Entity *enemy;
	Class *classes;
} Battle;

char** enter_battle(void*);

#endif

