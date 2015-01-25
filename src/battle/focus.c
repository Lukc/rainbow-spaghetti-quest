#include <stdlib.h>
#include <stdio.h>

#include "focus.h"

#include "../game.h"
#include "../enemies.h"
#include "../entities.h"
#include "../term.h"
#include "../colors.h"

#include "ai.h"
#include "turns.h"

#define BUFFER_SIZE 64

/**
 * Gives back mana (and possibly health or other stats).
 *
 * @fixme: Be clearer about recovery being prevented, when it is.
 */
void
focus(Entity* entity, Queue* logs)
{
	int mana_gained, health_gained;
	int can_recover = 1;
	int i, offset;
	Cell health_string[64];
	Cell mana_string[64];
	Cell cure_string[64];
	int cured = 0;
	Cell* log;
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
		ccnprintf(cure_string, 64, CYAN, 0, " -Cured!-");
	else
		cure_string[0].ch = '\0';

	log = malloc(sizeof(Cell) * 81);
	ccnprintf(log, 81, WHITE, 0, "%s focuses", entity->name);
	queue_add(logs, log);

	if (can_recover)
	{
		health_gained = entity->class->health_regen_on_focus;

		for (i = 0; i < EQ_MAX; i++)
			if (entity->equipment[i])
				health_gained += entity->equipment[i]->health_on_focus;

		if (entity->health + health_gained > get_max_health(entity))
			health_gained = get_max_health(entity) - entity->health;

		ccnprintf(
			health_string, 64,
			GREEN, 0, 
			" +%iHP",
			health_gained
		);

		entity->health += health_gained;
	}
	else
	{
		health_gained = 0;

		ccnprintf(health_string, 64, MAGENTA, 0, "<recovery prevented>");
	}

	mana_gained = entity->class->mana_regen_on_focus;

	for (i = 0; i < EQ_MAX; i++)
		if (entity->equipment[i])
			mana_gained += entity->equipment[i]->mana_on_focus;

	if (entity->mana + mana_gained > get_max_mana(entity))
		mana_gained = get_max_mana(entity) - entity->mana;

	entity->mana += mana_gained;

	if (mana_gained)
		ccnprintf(mana_string, 64, BLUE, 0, " +%iMP", mana_gained);
	else
		mana_string[0].ch = '\0';

	/* Finally concatenating everything. */

	log = malloc(sizeof(Cell) * 81);

	offset = ccnprintf(log, 81, BLUE, 0,  "   >>>");
	offset += copy_cells(log + offset, health_string, 81 - offset);
	offset += copy_cells(log + offset, mana_string, 81 - offset);
	offset += copy_cells(log + offset, cure_string, 81 - offset);

	queue_add(logs, log);
}

Queue*
command_focus(Game* game)
{
	Queue* logs;

	logs = queue_new();

	begin_turn(game->player, logs);

	focus(game->player, logs);

	end_turn(game->player, logs);

	ai_action(game, logs);

	return logs;
}

/* vim: set ts=4 sw=4 cc=80 : */
