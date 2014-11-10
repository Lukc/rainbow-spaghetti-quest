
#ifndef ENTITIES_H
#define ENTITIES_H

#include "battle.h"
#include "classes.h"

/* Why? Because! */
#define INVENTORY_SIZE 24

enum EQUIPMENT_SLOTS {
	EQ_WEAPON,
	EQ_WEAPON_RANGED,
	EQ_SHIELD,
	EQ_ARMOR,
	EQ_AMULET,

	EQ_MAX
};

typedef struct Attack {
	char *name;
	int damage;
	char *type;
	int mana; /* mana requirement to use */
} Attack;

typedef struct Entity {
	int id; /* @fixme: should be class_id or removed */
	char *name;
	int health;
	int mana;
	unsigned int kills;
	int caps; /* Bottle Caps */

	struct Class *class;

	int equipment[EQ_MAX];
	int inventory[INVENTORY_SIZE];
} Entity;

int get_max_mana(Entity*);
int get_max_health(Entity*);
int get_attack(Battle*, Entity*);
int get_defense(Battle*, Entity*);

int get_type_resistance(Battle*, Entity*, int);
int get_attack_type(Battle*, Entity*);

char* equipment_string(int);

int init_entity_from_class(Entity*, Class*);

void print_entity_basestats(Battle*, Entity*);
void print_entity(Battle*, Entity*);

#endif
