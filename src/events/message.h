
#ifndef EVENT_MESSAGE_H
#define EVENT_MESSAGE_H

#include "../parser.h"
#include "../events.h"

typedef struct {
	int type;
	char* name;

	char* from;
	char* text;
} MessageEvent;

Event* load_message_event(ParserElement* element);
void fire_message_event(Event*);

#endif

