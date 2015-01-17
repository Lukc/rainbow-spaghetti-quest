#include <stdlib.h>
#include <stdio.h>

#include "ai.h"
#include "attack.h"
#include "focus.h"
#include "turns.h"

#include "../colors.h"

void
ai_action(Game* game, Logs* logs)
{
	Entity* player;
	Entity* enemy;
	List* available_attacks;
	AttackData* selected_attack;
	char* log;

	player = game->player;
	enemy = game->enemy;

	if (player->health > 0)
	{
		available_attacks = get_all_attacks(enemy);
		selected_attack = list_nth(
			enemy->attacks,
			rand() % list_size(enemy->attacks));

		if (can_use_attack(enemy, selected_attack))
		{
			log = (char*) malloc(sizeof(char) * 128);
			snprintf(log, 128,
				BRIGHT WHITE "%s used “%s”",
				enemy->name,
				selected_attack->attack->name
			);
			logs_add(logs, log);

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

