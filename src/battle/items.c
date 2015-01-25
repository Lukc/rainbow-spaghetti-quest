#include <stdlib.h>
#include <stdio.h>

#include "items.h"
#include "attack.h"
#include "turns.h"
#include "ai.h"

#include "../colors.h"
#include "../term.h"

void
use_item(Entity* entity, Item* item, Entity* enemy, Queue* logs)
{
	Cell* log;

	log = (Cell*) malloc(sizeof(Cell) * 81);
	ccnprintf(
		log, 81, WHITE, 0,
		"%s uses “%s” <inventory>",
		entity->name, item->name
	);
	queue_add(logs, log);

	attack(entity, item->on_use, enemy, logs);
}

Queue*
command_use_item(Game* game, Entity* player, Item* item)
{
	Queue* logs;

	logs = queue_new();

	begin_turn(game->player, logs);

	use_item(player, item, game->enemy, logs);

	end_turn(player, logs);

	ai_action(game, logs);

	return logs;
}

