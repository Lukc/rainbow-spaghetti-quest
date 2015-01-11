
#ifndef ENTITIES_H
#define ENTITIES_H

#include "list.h"
#include "game.h"
#include "classes.h"
#include "attack.h"
#include "items.h"
#include "skills.h"

/* Why? Because! */
#define INVENTORY_SIZE 30

enum EQUIPMENT_SLOTS {
	EQ_WEAPON,
	EQ_WEAPON_RANGED,
	EQ_SHIELD,
	EQ_ARMOR,
	EQ_AMULET,
	EQ_EARS,

	EQ_MAX
};

typedef struct Entity {
	char *name;
	int health;
	int mana;
	unsigned int kills;
	int gold;

	List* statuses; /* List* of StatusData* */

	struct Class *class;

	Item* equipment[EQ_MAX];
	ItemStack inventory[INVENTORY_SIZE];
} Entity;

int get_max_mana(Entity*);
int get_max_health(Entity*);
int get_attack_bonus(Entity*);
int get_defense_bonus(Entity*);

int get_type_resistance(Entity*, int);
int get_attack_type(Game*, Entity*);

int give_health(Entity*, int);

char* equipment_string(int);

int init_entity_from_class(Entity*, Class*);

void print_entity_basestats(Entity*);
void print_entity(Entity*);

void remove_statuses(Entity*);

List* get_all_attacks(Entity*);

#endif
