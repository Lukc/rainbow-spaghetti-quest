
#ifndef CLASSES_H
#define CLASSES_H

#include "list.h"
#include "types.h"

typedef struct Class {
	char *name;

	int base_health;
	int base_mana;

	int gold_on_kill;

	int attack_bonus;
	int defense_bonus;

	int type_resistance[TYPE_MAX];

	/* Attack to use if no equipment to provide one */
	List* attacks;

	/* Random loot for random mobs. */
	List* drop;

	int health_regen_on_focus;
	int mana_regen_on_focus;
} Class;

#include "game.h"
#include "attack.h"
#include "drop.h"

Class* load_class(Game*, char*);
List* load_classes(Game*, char*);
Class* get_class_by_name(List*, char*);

#endif

