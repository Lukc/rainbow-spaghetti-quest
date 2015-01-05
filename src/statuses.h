
#ifndef _STATUSES_H
#define _STATUSES_H

#include "list.h"
#include "entities.h"

enum STATUSES {
	STATUS_CONFUSED,
	STATUS_FROZEN,
	STATUS_BURNED,
	STATUS_POISONED,
	STATUS_PARALYZED,
	STATUS_BLEEDING,

	STATUS_MAX
};

/* Container for a status’ effects. */
typedef struct Status {
	char* name;
	char* affliction_name;
	short divides_attack;
	short divides_defense;
	short increases_mana_costs;
	short reduces_physical_strikes;
	short reduces_magical_strikes;
	short prevents_recovery;
	short removed_on_focus;
} Status;

/* Container for the duration and strength of a status on an Entity. */
typedef struct {
	Status* status;
	int strength;
} StatusData;

void load_status(Game*, List*);

Status* get_status_by_name(List*, char*);

int  has_status     (struct Entity*, Status*);
int  inflict_status (struct Entity*, Status*);
void cure_status    (struct Entity*, Status*);

#endif

