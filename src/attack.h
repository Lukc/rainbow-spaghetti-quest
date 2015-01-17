
#ifndef ATTACK_H
#define ATTACK_H

#include "statuses.h"

typedef struct Attack {
	char *name;

	/* Positive values are mana/health given, negative ones are the cost of
	 * using the attack/item. */
	int mana;
	int health;

	/* Has to match something in src/types.h */
	int type;

	/* Amount of damage per strike and number of strikes per use of the
	 * Attack*. */
	int damage;
	int strikes;

	/* Number of turns you’ll have to wait ’till the attack becomes available
	 * again. */
	int cooldown;

	/* Statuses inflicted to the user or its adversary. */
	struct Status* inflicts_status;
	struct Status* self_inflicts_status;

	/* Statuses cured when using this attack. */
	List* cures_statuses; /* List* of Status* */

	/* Temporary containers used at load-time. */
	char* inflicts_status_name;
	char* self_inflicts_status_name;
	List* cures_status_names; /* List of char* */
} Attack;

typedef struct {
	Attack* attack;
	int cooldown;
} AttackData;

void free_attack(Attack*);

#endif

