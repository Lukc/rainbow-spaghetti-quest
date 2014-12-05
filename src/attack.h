
#ifndef ATTACK_H
#define ATTACK_H

#include "statuses.h"

typedef struct Attack {
	char *name;
	int mana_cost;
	int type;
	int damage;
	int strikes;
	struct Status* inflicts_status;
} Attack;

void free_attack(Attack*);

#endif

