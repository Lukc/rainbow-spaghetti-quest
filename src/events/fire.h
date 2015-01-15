
#ifndef FIRE_EVENT_H
#define FIRE_EVENT_H

#include "../events.h"
#include "../parser.h"

typedef struct {
	int type;
	char* name;

	char* event;
} FireEvent;

Event* load_fire_event(ParserElement*);

#endif

