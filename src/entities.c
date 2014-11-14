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
	Item* equipment;
	int i;
	int bonus = 0;

	for (i = 0; i < EQ_MAX; i++)
	{
		if (e->equipment[i])
		{
			equipment = e->equipment[i];

			bonus += equipment->mana_bonus;
		}
	}

	return e->class->base_mana + bonus;
}

int
get_max_health(Entity* e)
{
	Item* equipment;
	int i;
	int bonus = 0;

	for (i = 0; i < EQ_MAX; i++)
	{
		if (e->equipment[i])
		{
			equipment = e->equipment[i];

			bonus += equipment->health_bonus;
		}
	}

	return e->class->base_health + bonus;
}

int
get_attack_bonus(Entity* e)
{
	Item* equipment;
	int i;
	int bonus = 0;

	for (i = 0; i < EQ_MAX; i++)
	{
		if (e->equipment[i])
		{
			equipment = e->equipment[i];

			bonus += equipment->attack_bonus;
		}
	}

	return e->class->attack_bonus + bonus;
}

int
get_defense_bonus(Entity* e)
{
	Item* equipment;
	int i;
	int bonus = 0;

	for (i = 0; i < EQ_MAX; i++)
	{
		if (e->equipment[i])
		{
			equipment = e->equipment[i];

			bonus += equipment->defense_bonus;
		}
	}


	return e->class->defense_bonus + bonus;
}

int
get_type_resistance(Entity* e, int type)
{
	Item* equipment;
	int i;
	int bonus = 0;

	for (i = 0; i < EQ_MAX; i++)
	{
		if (e->equipment[i])
		{
			equipment = e->equipment[i];

			bonus += equipment->type_resistance[type];
		}
	}

	return bonus;
}

int
init_entity_from_class(Entity* e, Class* c)
{
	unsigned int i;

	e->class = c;

	e->name = strdup(c->name);

	for (i = 0; i < EQ_MAX; i++)
		e->equipment[i] = NULL;

	e->mana = get_max_mana(e);
	e->health = get_max_health(e);

	e->kills = 0;
	e->caps = 0;

	for (i = 0; i < INVENTORY_SIZE; i++)
		e->inventory[i] = NULL;

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

	printf(BRIGHT "<");
	for (i = 0; i < size; i++)
	{
		if (current != 0 && ((float) i) / size <= ((float) current) / max)
			printf("=");
		else
			printf(" ");
	}
	printf(">\n" NOCOLOR);
}

static void
print_resistance(Entity* e, int type)
{
	char* string;
	int printed, i;
	int resistance;

	string = strdup(type_string(type));
	string[0] = toupper(string[0]);

	printf(WHITE);
	printed = printf("  %s: ", string);

	for (i = printed; i < 14; i++)
		printf(" ");
	
	resistance = get_type_resistance(e, type);

	printed = 14 + printf(BRIGHT "%i%%", resistance);

	for (i = printed; i < 24; i++)
		printf(" ");

	printf(YELLOW "[");
	for (i = 0; i < 46; i++)
	{
		if (((float) i) / 46 <= ((float) resistance + 50) / 100)
		{
			if (((float) i / 46) <= .1)
				printf(BLACK);
			else if (((float) i / 46) <= .25)
				printf(RED);
			else if (((float) i / 46) > 0.5)
				printf(GREEN);
			else if (((float) i / 46) > 0.75)
				printf(WHITE);
			else
				printf(YELLOW);

			printf("|");
		}
		else
			printf(" ");
	}
	printf(YELLOW "]\n" NOCOLOR);
}

static void
print_attacks(Entity* e)
{
	List* list;
	Attack* attack;
	List* attacks;
	int has_attacks = 0;

	attacks = get_all_attacks(e);

	printf(BRIGHT WHITE "  Attacks:\n" NOCOLOR);
	for (list = attacks; list; list = list->next)
	{
		has_attacks = 1;
		attack = list->data;

		printf(WHITE "    %i - %i  %s\n" NOCOLOR,
			attack->damage + get_attack_bonus(e), attack->strikes,
			type_string(attack->type));
	}
}

void
print_entity_basestats(Entity* e)
{
	printf(BRIGHT BLUE ">> %s\n" NOCOLOR, e->name);

	print_bar(
		BRIGHT WHITE "  Health:   ",
		health_color(e),
		e->health, get_max_health(e), 40, 80 + 5 * 2
	);

	print_bar(
		BRIGHT WHITE "  Mana:     ",
		BLUE,
		e->mana, get_max_mana(e), 40, 80 + 5 * 2
	);
}

void
print_entity(Entity *e)
{
	print_entity_basestats(e);

	/* FIXME: That’s ugly. Not just the code, the output as well! */
	printf(
		BRIGHT WHITE "  [Def. " BLUE  "%i"
		BRIGHT WHITE "] - [Att. "
		BRIGHT RED "%i" BRIGHT WHITE "]" NOCOLOR
		"\n"
		,
		get_defense_bonus(e),
		get_attack_bonus(e)
	);

	print_attacks(e);

	printf("\n");

	printf("Resistances:\n");
	for (int i = 0; i < TYPE_MAX; i++)
	{
		print_resistance(e, i);
	}
}

char*
equipment_string(int id)
{
	if (id == -1)
		/* Not an equipment, but whatever… */
		return "item";
	else if (id == EQ_WEAPON)
		return "weapon";
	else if (id == EQ_SHIELD)
		return "shield";
	else if (id == EQ_ARMOR)
		return "armor";
	else if (id == EQ_WEAPON_RANGED)
		return "ranged weapon";
	else if (id == EQ_AMULET)
		return "amulet";
	else if (id == EQ_EARS)
		return "ears";
	else
		return "unknown";
}

List*
get_all_attacks(Entity* e)
{
	int i;
	Item* item;
	List* result = NULL, * list;

	for (i = 0; i < EQ_MAX; i++)
	{
		if((item = e->equipment[i]))
		{
			for (list = item->attacks; list; list = list->next)
			{
				list_add(&result, list->data);
			}
		}
	}

	if (result == NULL)
	{
		/* Well... we have to begin somewhere, right? */
		for (list = e->class->attacks; list; list = list->next)
			list_add(&result, list->data);
	}

	return result;
}

/* vim: set ts=4 sw=4 cc=80 : */
