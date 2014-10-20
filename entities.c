#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "types.h"
#include "colors.h"
#include "entities.h"

int
get_max_mana(Entity *e)
{
	return 10;
}

int
get_max_health(Entity *e)
{
	return 30;
}

int
get_attack(Entity *e)
{
	return 10;
}

int
get_defense(Entity *e)
{
	return 5;
}

int
init_entity(Entity *e)
{
	e->id = 0;

	e->name = "(no name)";

	e->mana = get_max_mana(e);
	e->health = get_max_health(e);

	e->kills = 0;
	e->caps = 0;

	e->class = 0;

	return 42;
}

int
init_entity_from_class(Entity *e, Class *c)
{
	e->id = c->id;
	e->name = strdup(c->name);

	e->mana = get_max_mana(e);
	e->health = get_max_health(e);

	e->kills = 0;
	e->caps = 0;

	e->class = 0;

	return 42;
}

void
print_entity(Entity *e)
{
	printf(BRIGHT BLUE ">> %s\n" NOCOLOR, e->name);

	/* FIXME: Make colors change depending on HP/mana levels */
	printf(
		"  Health:   " GREEN "%i/%i\n" NOCOLOR
		"  Mana:     " BLUE  "%i/%i\n" NOCOLOR
		"  Attack:   " RED   "%i\n" NOCOLOR
		"  Defense:  " BLUE  "%i\n" NOCOLOR
		,
		e->health, get_max_health(e),
		e->mana, get_max_mana(e),
		get_attack(e),
		get_defense(e)
	);

	printf("  Kills:    %i\n", e->kills);
}

/* vim: set ts=4 sw=4 cc=80 : */
