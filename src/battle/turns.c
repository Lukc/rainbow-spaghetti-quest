#include <stdlib.h>
#include <stdio.h>

#include "turns.h"

#include "../colors.h"
#include "../entities.h"
#include "../commands.h"

void
end_turn(Entity* e, Logs* logs)
{
	List* l;
	StatusData* sd;
	Status* status;
	int health_lost = 0;

	for (l = e->statuses; l; l = l->next)
	{
		sd = l->data;
		status = sd->status;

		if (status->removes_health)
		{
			health_lost += get_max_health(e) / status->removes_health;
		}
	}

	if (health_lost)
	{
		char* log;

		e->health -= health_lost;

		log = malloc(sizeof(char) * 128);
		snprintf(log, 128, BRIGHT MAGENTA " <<< " RED "%+iHP"
			MAGENTA " (statuses)",
			-health_lost);
		logs_add(logs, log);
	}
}

