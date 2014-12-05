
#ifndef GAME_H
#define GAME_H

#include "list.h"

/**
 * Mega-container for everything.
 */
typedef struct Game {
	struct Entity* player;
	struct Entity* enemy;
	List* classes;
	List* items;
	struct Place* location;
	List* images; /* List* of char* */
	List* statuses; /* List* of StatusData* */

	List* visited; /* List* of Place* */

	int flee;
} Game;

#include "places.h"
#include "entities.h"

#endif

