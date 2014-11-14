
#ifndef ATTACK_H
#define ATTACK_H

typedef struct Attack {
	char *name; /* FIXME: Necessary? */
	int mana_cost;
	int type;
	int damage;
	int strikes;
} Attack;

#endif

