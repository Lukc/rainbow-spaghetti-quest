
#ifndef ATTACK_H
#define ATTACK_H

typedef struct Attack {
	char *name;
	int mana_cost;
	int type;
	int damage;
	int strikes;
	int inflicts_status;
} Attack;

#endif

