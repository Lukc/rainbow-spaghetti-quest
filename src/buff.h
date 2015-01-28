
#ifndef BUFF_H
#define BUFF_H

typedef struct {
	char* name;

	/* Raw bonuses applied directly to base stats. o/ */
	int attack;
	int defense;
} Buff;

#include "entities.h"

int buff_is_empty(Buff*);

#endif

