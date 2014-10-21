
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
} Class;

#endif

