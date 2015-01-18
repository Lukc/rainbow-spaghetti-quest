#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "attack.h"
#include "turns.h"
#include "mana_cost.h"
#include "ai.h"

#include "../colors.h"
#include "../term.h"

/**
 * Checks whether someone has enough mana to use an attack or not.
 *
 * @return 1 if the attack can be used.
 * @return < 0 if the attack cannot be used.
 *   For the exact return values, see battle/attack.h
 */
int
can_use_attack(Entity* attacker, AttackData* ad)
{
	Attack* attack = ad->attack;

	if (attacker->mana + get_mana_cost(attacker, attack) < 0)
		return E_NO_MANA;

	if (attacker->health + attack->health < 0)
		return E_NO_HEALTH;

	if (ad->cooldown > 0)
		return E_COOLDOWN;

	return 1;
}

/**
 * Takes care of A attacking B with a specified attack.
 * Adds a corresponding log entry in “logs”.
 */
void
attack(Entity* attacker, Attack* attack, Entity* defender, Logs* logs)
{
	int strikes;
	int damage_inflicted = 0;
	int mana_cost;
	int type_modifier;
	char* log;
	char attack_string[64];
	char status_string[64];
	char self_status_string[64];
	char healing_string[64];
	char injury_string[64];
	char mana_string[64];
	char* cure_string = "";

	attack_string[0] = '\0';
	self_status_string[0] = '\0';
	status_string[0] = '\0';
	healing_string[0] = '\0';
	injury_string[0] = '\0';
	mana_string[0] = '\0';

	mana_cost = get_mana_cost(attacker, attack);

	if (attack->self_inflicts_status)
	{
		inflict_status(attacker, attack->self_inflicts_status);

		snprintf(self_status_string, 64, MAGENTA " <<< %s", attack->self_inflicts_status->name);
	}

	if (attack->strikes)
	{
		List* l;

		strikes = attack->strikes;

		for (l = attacker->statuses; l; l = l->next)
		{
			StatusData* data = l->data;
			Status* status = data->status;

			if (mana_cost != 0)
			{
				if (status->reduces_magical_strikes)
					strikes /= 2;
			}
			else
				if (status->reduces_physical_strikes)
					strikes /= 2;
		}

		type_modifier = get_type_resistance(defender, attack->type);

		/* Calculating for a single strike */
		damage_inflicted =
			(int) (((get_attack_bonus(attacker) + attack->damage) *
					(100. - type_modifier)) / 100.) -
			get_defense_bonus(defender);

		/* Taking care of negative damages... */
		damage_inflicted = damage_inflicted < 0 ? 0 : damage_inflicted;

		/* Taking the number of strikes into account now */
		damage_inflicted = damage_inflicted * strikes;

		defender->health -= damage_inflicted;

		snprintf(attack_string, 64,
			RED " >>>" WHITE " %s " RED "-%iHP " WHITE "<%i-%i %s>",
			defender->name,
			damage_inflicted,
			get_attack_bonus(attacker) + attack->damage,
			strikes, type_to_string(attack->type)
		);
	}

	if (attack->inflicts_status)
	{
		inflict_status(defender, attack->inflicts_status);

		snprintf(
			status_string, 64,
			MAGENTA " >>> %s",
			attack->inflicts_status->name
		);
	}

	if (attack->cures_statuses)
	{
		List* l;

		for (l = attack->cures_statuses; l; l = l->next)
		{
			Status* status = l->data;

			cure_status(attacker, status);
		}

		cure_string = BRIGHT WHITE " -" CYAN "Cured!" WHITE "-";
	}

	if (attack->health)
	{
		List* l;
		int can_recover = 1;

		if (attack->health > 0)
		{
			for (l = attacker->statuses; l && can_recover; l = l->next)
			{
				StatusData* data = l->data;
				Status* status = data->status;

				if (status->prevents_recovery)
					can_recover = 0;
			}

			if (can_recover)
			{
				give_health(attacker, attack->health);

				snprintf(
					healing_string, 64,
					GREEN " <<< +%iHP" WHITE,
					attack->health
				);
			}
			else
				snprintf(
					healing_string, 64,
					MAGENTA " <<< -recovery prevented-"
				);
		}
		else
		{
			attacker->health += attack->health;

			snprintf(
				injury_string, 64,
				RED " <<< %+iHP", attack->health
			);
		}
	}

	snprintf(mana_string, 64, BLUE " <<< %s%+iMP",
		mana_cost < 0 ? GRAY : "", mana_cost);

	attacker->mana += mana_cost;

	log = (char*) malloc(sizeof(char) * 128);
	snprintf(log, 128,
		BRIGHT WHITE "  %s%s%s%s%s%s",
		injury_string,
		healing_string,
		attack_string,
		status_string,
		cure_string,
		mana_string
	);
	logs_add(logs, log);
}

Logs*
command_attack(Game* game, AttackData* player_attack)
{
	Entity* player;
	Entity* enemy;
	Logs* logs;
	char* log;

	logs = logs_new();

	player = game->player;
	enemy = game->enemy;

	begin_turn(game->player, logs);

	log = (char*) malloc(sizeof(char) * 128);
	snprintf(log, 128,
		BRIGHT WHITE "%s uses “%s”",
		player->name,
		player_attack->attack->name
	);
	logs_add(logs, log);

	attack(player, player_attack->attack, enemy, logs);

	player_attack->cooldown = player_attack->attack->cooldown;

	if (enemy->health <= 0)
	{
		logs_add(logs, strdup("Your enemy is dead."));
	}
	else
	{
		end_turn(game->player, logs);

		ai_action(game, logs);
	}

	return logs;
}

/* vim: set ts=4 sw=4 cc=80 : */
