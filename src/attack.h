
#ifndef ATTACK_H
#define ATTACK_H

#include "statuses.h"

typedef struct Attack {
	char *name;
	int mana_cost;
	int gives_health;
	int type;
	int damage;
	int strikes;
	struct Status* inflicts_status;
	List* cures_statuses; /* List* of Status* */

	/* Fields required at load-time. */
	List* cures_status_names; /* List of char* */
} Attack;

void free_attack(Attack*);

#endif

