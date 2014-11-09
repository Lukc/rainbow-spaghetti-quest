
#ifndef ENTITIES_H
#define ENTITIES_H

#include "classes.h"

/* Why? Because! */
#define INVENTORY_SIZE 24

enum EQUIPMENT_SLOTS {
	EQ_WEAPON,
	EQ_SHIELD,
	EQ_ARMOR,

	EQ_MAX
};

typedef struct {
	char *name;
	int damage;
	char *type;
	int mana; /* mana requirement to use */
} Attack;

typedef struct {
	int id; /* @fixme: should be class_id or removed */
	char *name;
	int health;
	int mana;
	unsigned int kills;
	int caps; /* Bottle Caps */

	Class *class;

	int equipment[EQ_MAX];
	int inventory[INVENTORY_SIZE];
} Entity;

int get_max_mana(Entity*);
int get_max_health(Entity*);
int get_attack(Entity*);
int get_defense(Entity*);

char* equipment_string(int);

int init_entity(Entity*);
int init_entity_from_class(Entity*, Class*);
void remove_entity(Entity*);

void print_entity(Entity*);

#endif
