#include <stdlib.h>
#include <stdio.h>

#include "flee.h"

Logs*
command_flee(Game* game)
{
	Logs* logs;
	char* log;

	logs = logs_new();

	log = (char*) malloc(sizeof(char) * 128);
	snprintf(
		log, 128,
		"You fled from battle!");
	logs_add(logs, log);

	game->flee = 1;

	return NULL;
}

