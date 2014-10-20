
#ifndef ENTITIES_H
#define ENTITIES_H

#include "classes.h"

typedef struct {
	int id;
	char *name;
	int health;
	int mana;
	unsigned int kills;
	int caps; /* Bottle Caps */
	int class;
} Entity;

int get_max_mana(Entity*);
int get_max_health(Entity*);
int get_attack(Entity*);
int get_defense(Entity*);

int init_entity(Entity*);
int init_entity_from_class(Entity*, Class*);

void print_entity(Entity*);

#endif
