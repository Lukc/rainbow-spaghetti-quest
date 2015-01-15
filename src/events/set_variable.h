
#ifndef SET_VARIABLE_EVENT_H
#define SET_VARIABLE_EVENT_H

#include "../events.h"
#include "../parser.h"

typedef struct {
	int type;
	char* name;

	char* variable;
	int value;
} SetVariableEvent;

Event* load_set_variable_event(ParserElement*);
void fire_set_variable_event(Game*, Event*);

#endif

