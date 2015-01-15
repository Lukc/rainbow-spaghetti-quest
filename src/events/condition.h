
#ifndef CONDITION_EVENT_H
#define CONDITION_EVENT_H

#include "../list.h"
#include "../events.h"
#include "../parser.h"

typedef struct {
	int type;
	char* name;

	Condition condition;

	List* then; /* List* of Event* */
	List* _else; /* List* of Event* */
} ConditionEvent;

Event* load_condition_event(Game*, ParserElement*);
void fire_condition_event(Game*, Event*);

#endif

