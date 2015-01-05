
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
	struct Status* self_inflicts_status;
	List* cures_statuses; /* List* of Status* */

	/* Fields required at load-time. */
	char* inflicts_status_name;
	char* self_inflicts_status_name;
	List* cures_status_names; /* List of char* */
} Attack;

void free_attack(Attack*);

#endif

