#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "types.h"
#include "colors.h"
#include "entities.h"

int
get_max_mana(Entity* e)
{
	return e->class->base_mana;
}

int
get_max_health(Entity* e)
{
	return e->class->base_health;
}

int
get_attack(Battle* data, Entity* e)
{
	Item* equipment;
	int i;
	int bonus = 0;

	for (i = 0; i < EQ_MAX; i++)
	{
		if (e->equipment[i])
		{
			equipment = get_item_from_id(data, e->equipment[i]);

			bonus += equipment->attack_bonus;
		}
	}

	return e->class->base_attack + bonus;
}

int
get_defense(Battle* data, Entity* e)
{
	Item* equipment;
	int i;
	int bonus = 0;

	for (i = 0; i < EQ_MAX; i++)
	{
		if (e->equipment[i])
		{
			equipment = get_item_from_id(data, e->equipment[i]);

			bonus += equipment->defense_bonus;
		}
	}


	return e->class->base_defense + bonus;
}

int
init_entity_from_class(Entity* e, Class* c)
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

	for (i = 0; i < INVENTORY_SIZE; i++)
		e->inventory[i] = -1;

	return 42;
}

static const char*
health_color(Entity* e)
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

static void
print_bar(
	const char* statstring, const char* color,
	int current, int max, int size)
{
	int i;
	int printed;

	printed = printf("%s%s%i/%i  ", statstring, color, current, max);

	for (i = printed; i < 40; i++)
		printf(" ");

	size = size - 40;

	printf("[");
	for (i = 0; i < size; i++)
	{
		if (current != 0 && ((float) i) / size <= ((float) current) / max)
			printf("|");
		else
			printf(" ");
	}
	printf("]\n");
}

void
print_entity(Battle* data, Entity *e)
{
	printf(BRIGHT BLUE ">> %s\n" NOCOLOR, e->name);

	print_bar(
		WHITE "  Health:   " NOCOLOR,
		health_color(e),
		e->health, get_max_health(e), 80
	);

	print_bar(
		WHITE "  Mana:     " NOCOLOR,
		BLUE,
		e->mana, get_max_mana(e), 80
	);

	/* FIXME: Make colors change depending on HP/mana levels */
	printf(
		WHITE "  Attack:   " RED   "%i\n" NOCOLOR
		WHITE "  Defense:  " BLUE  "%i\n" NOCOLOR
		WHITE "  Kills:    " "%i" "\n" NOCOLOR
		,
		get_attack(data, e),
		get_defense(data, e),
		e->kills
	);
}

char*
equipment_string(int id)
{
	if (id == 0)
		return "Weapon";
	else if (id == 1)
		return "Shield";
	else if (id == 2)
		return "Armor";
	else
		return "Unknown";
}

/* vim: set ts=4 sw=4 cc=80 : */
