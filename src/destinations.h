
#ifndef DESTINATIONS_H
#define DESTINATIONS_H

#include "conditions.h"

typedef struct Destination {
	char* name;
	Place* place;
	Condition condition;
} Destination;

#endif

