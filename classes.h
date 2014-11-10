
#ifndef CLASSES_H
#define CLASSES_H

#include "types.h"

typedef struct Class {
	int id;
	char *name;

	int base_health;
	int base_mana;

	int caps_on_kill;

	int base_attack;
	int base_defense;

	int type_defense[TYPE_MAX];

	int mana_regen_on_focus;
} Class;

void load_class(Class*, char*);
Class* load_classes(char*);
Class* get_class_by_name(Class*, char*);

#endif

