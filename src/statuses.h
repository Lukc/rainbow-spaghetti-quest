
#ifndef _STATUSES_H
#define _STATUSES_H

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

char* status_to_string(int);

int has_status(Entity*, int);

#endif

