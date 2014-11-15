
#ifndef PLACES_H
#define PLACES_H

#include "list.h"
#include "game.h"

typedef struct Place {
	char* name;
	List* shop_items;      /* List* of Item* */
	List* random_enemies;  /* List* of Class* */
	List* destinations;    /* List* of Destination* */
	char** image;

	List* on_first_visit;  /* List* of char** (images) */
} Place;

List* load_places(Game*, char*);
Place* load_place(Game*, char*);

Place* get_place_by_name(List*, char*);

int has_visited(Game*, Place*);

#endif

