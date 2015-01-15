#include <stdlib.h>
#include <stdio.h>

#include "focus.h"

#include "../game.h"
#include "../enemies.h"
#include "../entities.h"
#include "../colors.h"

#include "ai.h"
#include "turns.h"

/**
 * Gives back mana (and possibly health or other stats).
 *
 * @fixme: Be clearer about recovery being prevented, when it is.
 */
void
focus(Entity* entity, Logs* logs)
{
	int mana_gained, health_gained;
	int can_recover = 1;
	int i;
	char health_string[64];
	char mana_string[64];
	char cure_string[64];
	int cured = 0;
	char* log;
	List* l;
	List* next;

	for (l = entity->statuses; l && can_recover; l = l->next)
	{
		StatusData* data = l->data;
		Status* status = data->status;

		if (status->prevents_recovery)
			can_recover = 0;
	}

	for (l = entity->statuses; l; l = next)
	{
		StatusData* data = l->data;
		Status* status = data->status;

		/* l might be freed due to cure_status. Getting next element now. */
		next = l->next;

		if (status->removed_on_focus)
			if (has_status(entity, status))
			{
				cured = 1;
				cure_status(entity, status);
			}
	}

	if (cured)
		snprintf(cure_string, 64, BRIGHT WHITE " -" CYAN "Cured!" WHITE "-");
	else
		cure_string[0] = '\0';

	log = (char*) malloc(sizeof(char) * 128);
	snprintf(
		log, 128,
		BRIGHT WHITE "%s focuses"
		NOCOLOR,
		entity->name);
	logs_add(logs, log);

	if (can_recover)
	{
		health_gained = entity->class->health_regen_on_focus;

		for (i = 0; i < EQ_MAX; i++)
			if (entity->equipment[i])
				health_gained += entity->equipment[i]->health_on_focus;

		if (entity->health + health_gained > get_max_health(entity))
			health_gained = get_max_health(entity) - entity->health;

		snprintf(
			health_string, 64,
			GREEN " +%iHP" WHITE,
			health_gained
		);

		entity->health += health_gained;
	}
	else
	{
		health_gained = 0;

		snprintf(
			health_string, 64,
			MAGENTA BRIGHT "<recovery prevented>" NOCOLOR WHITE
		);
	}

	mana_gained = entity->class->mana_regen_on_focus;

	for (i = 0; i < EQ_MAX; i++)
		if (entity->equipment[i])
			mana_gained += entity->equipment[i]->mana_on_focus;

	if (entity->mana + mana_gained > get_max_mana(entity))
		mana_gained = get_max_mana(entity) - entity->mana;

	entity->mana += mana_gained;

	if (mana_gained)
		snprintf(
			mana_string, 64,
			BRIGHT BLUE " +%iMP" WHITE,
			mana_gained
		);
	else
		mana_string[0] = '\0';

	log = (char*) malloc(sizeof(char) * 128);
	snprintf(
		log, 128,
		BRIGHT WHITE "   " BLUE "<<< %s%s%s"
		NOCOLOR,
		health_string, mana_string, cure_string);
	logs_add(logs, log);
}

Logs*
command_focus(Game* game)
{
	Logs* logs;

	logs = logs_new();

	focus(game->player, logs);

	end_turn(game->player, logs);

	ai_action(game, logs);

	return logs;
}

/* vim: set ts=4 sw=4 cc=80 : */
