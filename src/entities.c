#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "string.h"
#include "types.h"
#include "statuses.h"
#include "colors.h"
#include "term.h"
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
	List* list;
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

	/* Some statuses can cut an entity’s attack in half. */
	for (list = e->statuses; list; list = list->next)
	{
		StatusData* data = list->data;
		Status* status = data->status;

		if (status->divides_attack)
		{
			bonus = bonus / 2;

			break;
		}
	}

	return e->class->attack_bonus + bonus;
}

int
get_defense_bonus(Entity* e)
{
	List* list;
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

	for (list = e->statuses; list; list = list->next)
	{
		StatusData* data = list->data;
		Status* status = data->status;

		if (status->divides_defense)
		{
			bonus = bonus / 2;

			break;
		}
	}

	return e->class->defense_bonus + bonus;
}

int
get_type_resistance(Entity* e, int type)
{
	Item* equipment;
	int i;
	int bonus;

	bonus = e->class->type_resistance[type];

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
give_health(Entity* e, int amount)
{
	int max = get_max_health(e);

	if (e->health + amount > max)
	{
		int r = max - e->health;

		e->health = max;

		return r;
	}
	else
	{
		e->health += amount;

		return amount;
	}
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
	e->gold = 0;

	for (i = 0; i < INVENTORY_SIZE; i++)
	{
		e->inventory[i].quantity = 0;
		e->inventory[i].item = NULL;
	}

	e->statuses = NULL;

	return 42;
}

static int
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
	const char* statstring, int colorset,
	int current, int max, int text_size, int size)
{
	int i;
	int printed;

	fg(WHITE);
	printed = printf("%s", statstring);

	fg(colorset);

	printed += printf("%i/%i  ", current, max);

	for (i = printed; i < text_size; i++)
		printf(" ");

	size = size - text_size;

	fg(WHITE);
	printf("<");
	fg(colorset);
	for (i = 0; i < size; i++)
	{
		if (current != 0 && ((float) i) / size <= ((float) current) / max)
			printf("=");
		else
			printf(" ");
	}

	fg(WHITE);
	printf(">\n");
	nocolor();
}

static void
print_resistance(Entity* e, int type)
{
	char* string;
	int printed, i;
	int resistance;

	string = strdup(type_to_string(type));
	string[0] = toupper(string[0]);

	fg(WHITE);
	printed = printf("  %s: ", string);
	
	resistance = get_type_resistance(e, type);

	for (i = printed; i < 16; i++)
		printf(" ");

	if (resistance < 0)
		fg(YELLOW);
	else if (resistance > 0)
		fg(GREEN);

	printf("%i\n", resistance);
}

static void
print_attacks(Entity* e)
{
	List* list;
	Attack* attack;
	List* attacks;

	attacks = get_all_attacks(e);

	move(40);
	fg(WHITE);
	printf("  Attacks:\n");
	for (list = attacks; list; list = list->next)
	{
		attack = list->data;

		move(40);
		if (attack->strikes)
			printf("    (%i-%i)x%i  %s\n",
				attack->damage.min + get_attack_bonus(e),
				attack->damage.max + get_attack_bonus(e),
				attack->strikes,
				type_to_string(attack->type));
		else
			printf("    support attack\n");
	}
}

void
print_entity_basestats(Entity* e)
{
	List* l;

	fg(BLUE);
	printf(" >> ");
	fg(WHITE);
	printf("%s", e->name);

	if (e->statuses)
	{
		for (l = e->statuses; l; l = l->next)
		{
			printf(" <%s>",
				((StatusData*) l->data)->status->affliction_name);
		}
	}

	printf("\n");

	print_bar(
		"  Health:   ",
		health_color(e),
		e->health, get_max_health(e), 34, 80
	);

	print_bar(
		"  Mana:     ",
		BLUE,
		e->mana, get_max_mana(e), 34, 80
	);
}

static void
tag(const char* name, int value, int tag_color)
{
	fg(WHITE);
	printf("[%s ", name);
	fg(tag_color);
	printf("%i", value);
	fg(WHITE);
	printf("]");
}

void
print_entity(Entity *e)
{
	int i;

	print_entity_basestats(e);

	/* FIXME: That’s ugly. Not just the code, the output as well! */
	tag("Def.", get_defense_bonus(e), BLUE);
	tag("Att.", get_attack_bonus(e), BLUE);

	printf("\n");

	printf("Resistances:\n");
	for (i = 0; i < TYPE_MAX; i++)
	{
		print_resistance(e, i);
	}

	back(TYPE_MAX + 1);
	print_attacks(e);

	back_to_top();
	for (i = 0; i < 16; i++)
		printf("\n");
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

	for (i = EQ_MAX - 1; i >= 0; i--)
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

void
remove_statuses(Entity* e)
{
	List* list;

	/* For now, no content to free. */
	for (list = e->statuses; list; list = list->next)
		;

	/* FIXME: callback to free the StatusData*s */
	list_free(e->statuses, NULL);

	e->statuses = (List*) NULL;
}

/* vim: set ts=4 sw=4 cc=80 : */
