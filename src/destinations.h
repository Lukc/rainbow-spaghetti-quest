
#ifndef DESTINATIONS_H
#define DESTINATIONS_H

typedef struct Destination {
	char* name;
	List* needed_items; /* List* of Item* */
	Place* place;
} Destination;

#endif

