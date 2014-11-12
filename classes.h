
#ifndef CLASSES_H
#define CLASSES_H

#include "list.h"
#include "types.h"
#include "attack.h"

typedef struct Class {
	char *name;

	int base_health;
	int base_mana;

	int caps_on_kill;

	int attack_bonus;
	int defense_bonus;

	int type_resistance[TYPE_MAX];

	/* Attack to use if no equipment to provide one */
	List* attacks;

	int mana_regen_on_focus;
} Class;

Class* load_class(char*);
List* load_classes(char*);
Class* get_class_by_name(List*, char*);

#endif

