
#ifndef CLASSES_H
#define CLASSES_H

#include "list.h"
#include "types.h"

typedef struct Class {
	int id;
	char *name;

	int base_health;
	int base_mana;

	int caps_on_kill;

	int base_attack;
	int base_defense;

	int type_resistance[TYPE_MAX];
	int attack_type; /* In case no weapon is equipped */

	int mana_regen_on_focus;
} Class;

Class* load_class(char*);
List* load_classes(char*);
Class* get_class_by_name(List*, char*);

#endif

