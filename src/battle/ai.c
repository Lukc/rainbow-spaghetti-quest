#include <stdlib.h>
#include <stdio.h>

#include "ai.h"
#include "attack.h"
#include "focus.h"
#include "turns.h"

#include "../colors.h"
#include "../term.h"

void
ai_action(Game* game, Queue* logs)
{
	Entity* player;
	Entity* enemy;
	AttackData* selected_attack;
	Cell* log;

	player = game->player;
	enemy = game->enemy;

	if (player->health > 0)
	{
		selected_attack = list_nth(
			enemy->attacks,
			rand() % list_size(enemy->attacks));

		if (can_use_attack(enemy, selected_attack) > 0)
		{
			begin_turn(enemy, logs);

			log = malloc(sizeof(Cell) * 81);
			ccnprintf(log, 81, WHITE, 0,
				"%s used “%s”",
				enemy->name,
				selected_attack->attack->name
			);
			queue_add(logs, log);

			attack(
				enemy,
				selected_attack->attack,
				player,
				logs
			);
		}
		else
			focus(enemy, logs);
	}

	end_turn(enemy, logs);
}

