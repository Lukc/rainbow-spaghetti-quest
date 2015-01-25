#include <stdlib.h>
#include <stdio.h>

#include "flee.h"

#include "../colors.h"
#include "../term.h"

Queue*
command_flee(Game* game)
{
	Queue* logs;
	Cell* log;

	logs = queue_new();

	log = malloc(sizeof(Cell) * 81);
	ccnprintf(log, 81, WHITE, 0, "You fled from battle!");
	queue_add(logs, log);

	game->flee = 1;

	return NULL;
}

