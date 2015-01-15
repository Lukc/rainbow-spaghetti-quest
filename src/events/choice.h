
#ifndef CHOICE_EVENT_H
#define CHOICE_EVENT_H

#include "../list.h"
#include "../events.h"
#include "../parser.h"
#include "../game.h"

typedef struct {
	char* text;
	List* events; /* List* of Event* */
} ChoiceEventOption;

typedef struct {
	int type;
	char* name;

	List* options; /* List* of ChoiceEventOption* */
} ChoiceEvent;

Event* load_choice_event(Game*, ParserElement*);
void fire_choice_event(Game*, Event*);

#endif

