
#ifndef PLACES_H
#define PLACES_H

#include "list.h"
#include "battle.h"

typedef struct Place {
	char* name;
	List* shop_items;      /* List* of Item* */
	List* random_enemies;  /* List* of Class* */
	List* destinations;    /* List* of Place* */
} Place;

List* load_places(Battle*, char*);
Place* load_place(Battle*, char*);

Place* get_place_by_name(List*, char*);

#endif

