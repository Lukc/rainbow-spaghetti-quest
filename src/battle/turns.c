#include <stdlib.h>
#include <stdio.h>

#include "turns.h"

#include "../colors.h"
#include "../entities.h"
#include "../term.h"

void
begin_turn(Entity* e, Queue* logs)
{
	List* l;

	(void) logs;

	for (l = e->attacks; l; l = l->next)
	{
		AttackData* ad = l->data;

		if (ad->cooldown > 0)
			ad->cooldown -= 1;
	}
}

void
end_turn(Entity* e, Queue* logs)
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
		Cell* log;

		e->health -= health_lost;

		log = malloc(sizeof(Cell) * 81);
		ccnprintf(log, 81, MAGENTA, 0, "  <<< %+iHP (statuses)",
			-health_lost);
		queue_add(logs, log);
	}
}

