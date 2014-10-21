
#ifndef CLASSES_H
#define CLASSES_H

typedef struct {
	int id;
	char *name;

	int base_health;
	int base_mana;

	int caps_on_kill;

	int base_attack;
	int base_defense;

	int mana_regen_on_focus;
} Class;

Class load_class(char*);
Class* load_classes(char*);
Class* get_class_by_name(Class*, char*);

#endif

