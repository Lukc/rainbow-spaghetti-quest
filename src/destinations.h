
#ifndef DESTINATIONS_H
#define DESTINATIONS_H

typedef struct Destination {
	char* name;
	List* needed_items;
	Place* place;
} Destination;

#endif

