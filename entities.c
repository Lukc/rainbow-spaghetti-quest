#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "types.h"
#include "colors.h"
#include "entities.h"

int
get_max_mana(Entity *e)
{
	return e->class->base_mana;
}

int
get_max_health(Entity *e)
{
	return e->class->base_health;
}

int
get_attack(Entity *e)
{
	return e->class->base_attack;
}

int
get_defense(Entity *e)
{
	return e->class->base_defense;
}

int
init_entity_from_class(Entity *e, Class *c)
{
	unsigned int i;

	e->class = c;

	e->id = c->id;
	e->name = strdup(c->name);

	e->mana = get_max_mana(e);
	e->health = get_max_health(e);

	e->kills = 0;
	e->caps = 0;

	for (i = 0; i < sizeof(enum EQUIPMENT_SLOTS); i++)
		e->equipment[i] = 0;

	e->inventory = (int*) malloc(sizeof(int) * 42);
	e->inventory[0] = 0;

	return 42;
}

void
remove_entity(Entity *e)
{
	free(e->inventory);
}

static const char*
health_color(Entity *e)
{
	int cur, max, ratio;

	cur = e->health;
	max = get_max_health(e);
	ratio = 100 * cur / max;

	if (ratio > 66)
		return GREEN;
	else if (ratio > 33)
		return YELLOW;
	else
		return RED;
}

void
print_entity(Entity *e)
{
	printf(BRIGHT BLUE ">> %s\n" NOCOLOR, e->name);

	/* FIXME: Make colors change depending on HP/mana levels */
	printf(
		WHITE "  Health:   " "%s"  "%i/%i\n" NOCOLOR
		WHITE "  Mana:     " BLUE  "%i/%i\n" NOCOLOR
		WHITE "  Attack:   " RED   "%i\n" NOCOLOR
		WHITE "  Defense:  " BLUE  "%i\n" NOCOLOR
		WHITE "  Kills:    " "%i" "\n" NOCOLOR
		,
		health_color(e), e->health, get_max_health(e),
		e->mana, get_max_mana(e),
		get_attack(e),
		get_defense(e),
		e->kills
	);
}

/* vim: set ts=4 sw=4 cc=80 : */
