#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

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
get_type_resistance(Battle* data, Entity* e, int type)
{
	Item* equipment;
	int i;
	int bonus = 0;

	for (i = 0; i < EQ_MAX; i++)
	{
		if (e->equipment[i])
		{
			equipment = get_item_from_id(data, e->equipment[i]);

			bonus += equipment->defense[type];
		}
	}


	return bonus;
}

int
get_attack_type(Battle* data, Entity* e)
{
	if (e->equipment[EQ_WEAPON])
			return get_item_from_id(data, e->equipment[EQ_WEAPON])->attack_type;
	else
		/* Bare hands... that’s impact, right? */
		return TYPE_IMPACT;
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
	int current, int max, int text_size, int size)
{
	int i;
	int printed;

	printed = printf("%s%s%i/%i  ", statstring, color, current, max);

	for (i = printed; i < text_size; i++)
		printf(" ");

	size = size - text_size;

	printf(BRIGHT "[");
	for (i = 0; i < size; i++)
	{
		if (current != 0 && ((float) i) / size <= ((float) current) / max)
			printf("|");
		else
			printf(" ");
	}
	printf("]\n" NOCOLOR);
}

static void
print_resistance(Battle* data, Entity* e, int type)
{
	char* string;
	int printed, i;
	int resistance;

	string = strdup(type_string(type));
	string[0] = toupper(string[0]);

	printed = printf(" %s: ", string);

	for (i = printed; i < 14; i++)
		printf(" ");
	
	resistance = 100 + get_type_resistance(data, e, type);

	printed = 14 + printf(BRIGHT "%i", resistance);

	for (i = printed; i < 24; i++)
		printf(" ");

	printf(YELLOW "[");
	for (i = 0; i < 46; i++)
	{
		if (resistance != 0 && ((float) i) / 46 <= ((float) resistance) / 200)
		{
			if (((float) i / 46) <= .15)
				printf(RED);
			else if (((float) i / 46) > 0.5)
				printf(GREEN);
			else
				printf(YELLOW);

			printf("|");
		}
		else
			printf(" ");
	}
	printf(YELLOW "]\n" NOCOLOR);
}

void
print_entity(Battle* data, Entity *e)
{
	printf(BRIGHT BLUE ">> %s\n" NOCOLOR, e->name);

	print_bar(
		BRIGHT WHITE "  Health:   ",
		health_color(e),
		e->health, get_max_health(e), 40, 80
	);

	print_bar(
		BRIGHT WHITE "  Mana:     ",
		BLUE,
		e->mana, get_max_mana(e), 40, 80
	);

	/* FIXME: Make colors change depending on HP/mana levels */
	printf(
		BRIGHT WHITE "  Attack:   " RED   "%i" NOCOLOR BRIGHT " - %s\n" NOCOLOR
		BRIGHT WHITE "  Defense:  " BLUE  "%i\n" NOCOLOR
		WHITE "  Kills:    " "%i" "\n" NOCOLOR
		,
		get_attack(data, e), type_string(get_attack_type(data, e)),
		get_defense(data, e),
		e->kills
	);

	printf("\n");

	/* FIXME: Print typed defenses here */
	for (int i = 0; i < TYPE_MAX; i++)
	{
		print_resistance(data, e, i);
	}
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
