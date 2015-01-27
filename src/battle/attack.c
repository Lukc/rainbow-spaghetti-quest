#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "attack.h"
#include "turns.h"
#include "mana_cost.h"
#include "ai.h"

#include "../string.h"
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
 * Puts all attacks’ charge counter to 0.
 */
void
reset_charges(Entity* e)
{
	List* l;

	for (l = e->attacks; l; l = l->next)
		((AttackData*) l->data)->charge = 0;
}

/**
 * Takes care of A attacking B with a specified attack.
 * Adds a corresponding log entry in “logs”.
 */
void
attack(Entity* attacker, Attack* attack, Entity* defender, Queue* logs)
{
	float random_modifier;
	int offset;
	int strikes;
	int damage_inflicted = 0;
	int mana_cost;
	int type_modifier;
	Cell* log;

	log = (Cell*) malloc(sizeof(Cell) * 81);
	offset = ccnprintf(log, 4, 0, 0, "  ");

	mana_cost = get_mana_cost(attacker, attack);

	if (attack->self_inflicts_status)
	{
		inflict_status(attacker, attack->self_inflicts_status);

		offset += ccnprintf(log + offset, 81 - offset,
				RED, 0, " <<< %s", attack->self_inflicts_status->name);
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

		random_modifier = (float) (rand() % 100) / 100;

		damage_inflicted =
			(int) (
				(get_attack_bonus(attacker) - get_defense_bonus(defender)
				 + attack->damage.min + random_modifier *
					(attack->damage.max - attack->damage.min)) *
				(100 - type_modifier) / 100
			) * strikes;

		defender->health -= damage_inflicted;

		offset += ccnprintf(log + offset, 81 - offset, RED, 0, " >>>");
		offset += ccnprintf(log + offset, 81 - offset, WHITE, 0,
			" %s -%iHP ", defender->name, damage_inflicted
		);
		offset += ccnprintf(log + offset, 81 - offset, GRAY, 0,
			"<(%i-%i)x%i %s>",
			get_attack_bonus(attacker) + attack->damage.min,
			get_attack_bonus(attacker) + attack->damage.max,
			strikes, type_to_string(attack->type)
		);
	}

	if (attack->inflicts_status)
	{
		inflict_status(defender, attack->inflicts_status);

		offset += ccnprintf(
			log + offset, 81 - offset, MAGENTA, 0,
			" >>> %s",
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

		offset += ccnprintf(log + offset, 81 - offset, CYAN, 0, " -Cured!-");
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

				offset += ccnprintf(
					log + offset, 81 - offset, GREEN, 0,
					" <<< +%iHP",
					attack->health
				);
			}
			else
				offset += ccnprintf(
					log + offset, 81 - offset, MAGENTA, 0,
					" <<< -recovery prevented-"
				);
		}
		else
		{
			attacker->health += attack->health;

			offset += ccnprintf(
				log + offset, 81 - offset, RED, 0,
				" <<< %+iHP ", attack->health
			);
		}
	}

	if (mana_cost)
		offset += ccnprintf(log + offset, 81 - offset, BLUE, 0, " <<< %+iMP", mana_cost);

	attacker->mana += mana_cost;

	reset_charges(attacker);

	queue_add(logs, log);
}

Queue*
command_attack(Game* game, AttackData* player_attack)
{
	Entity* player;
	Entity* enemy;
	Queue* logs;
	Cell* log;

	logs = queue_new();

	player = game->player;
	enemy = game->enemy;

	begin_turn(game->player, logs);

	log = malloc(sizeof(Cell) * 81);
	ccnprintf(log, 81, WHITE, 0,
		"%s uses “%s”",
		player->name,
		player_attack->attack->name
	);
	queue_add(logs, log);

	if (player_attack->charge >= player_attack->attack->charge)
		attack(player, player_attack->attack, enemy, logs);
	else
	{
		player_attack->charge += 1;
		queue_add(logs, strtocells("   >>> The attack is charging!", WHITE, 0));
	}

	player_attack->cooldown = player_attack->attack->cooldown;

	if (enemy->health <= 0)
	{
		queue_add(logs, strtocells("Your enemy is dead.", WHITE, 0));
	}
	else
	{
		end_turn(game->player, logs);

		ai_action(game, logs);
	}

	return logs;
}

/* vim: set ts=4 sw=4 cc=80 : */
