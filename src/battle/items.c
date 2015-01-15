#include <stdlib.h>
#include <stdio.h>

#include "items.h"
#include "attack.h"
#include "turns.h"
#include "ai.h"

#include "../colors.h"

void
use_item(Entity* entity, Item* item, Entity* enemy, Logs* logs)
{
	char* log;

	log = (char*) malloc(sizeof(char) * 128);
	snprintf(
		log, 128,
		BRIGHT WHITE "%s uses “%s” <" YELLOW "inventory" WHITE ">"
		NOCOLOR,
		entity->name, item->name
	);
	logs_add(logs, log);

	attack(entity, item->on_use, enemy, logs);
}

Logs*
command_use_item(Game* game, Entity* player, Item* item)
{
	Logs* logs;

	logs = logs_new();

	use_item(player, item, game->enemy, logs);

	end_turn(player, logs);

	ai_action(game, logs);

	return logs;
}

