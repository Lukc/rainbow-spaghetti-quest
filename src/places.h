
#ifndef PLACES_H
#define PLACES_H

#include "list.h"
#include "skills.h"
#include "game.h"
#include "characters.h"

typedef struct Place {
	char* name;
	List* shop_items;      /* List* of Item* */
	List* random_enemies;  /* List* of RandomEnemy* */
	List* destinations;    /* List* of Destination* */
	char** image;

	List* skill_drops; /* List*s of SkillDrops* */

	List* on_first_visit;  /* List* of char** (images) */
	List* characters; /* List* of Character* */

	/* Stuff required at load-time */
	List* random_enemy_names; /* List* of char* */
	List* shop_item_names; /* List* of char* */
} Place;

void load_place(Game*, List*);

Place* get_place_by_name(List*, char*);

int has_visited(Game*, Place*);

void free_place(void*);

#endif

