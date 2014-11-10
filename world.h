
#ifndef WORLD_H
#define WORLD_H

#define MAX_PLACES 256

typedef struct Place {
	char* name;
} Place;

typedef struct World {
	Place* places[MAX_PLACES];
} World;

#endif

