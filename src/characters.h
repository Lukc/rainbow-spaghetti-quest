
#ifndef CHARACTERS_H
#define CHARACTERS_H

#include "parser.h"
#include "game.h"

enum EVENT_TYPE {
	EVENT_MESSAGE,
	EVENT_CHOICE
};

typedef struct {
	int type;
} Event;

typedef struct {
	int type;
	char* from;
	char* text;
} MessageEvent;

typedef struct {
	char* text;
	List* events; /* List* of Event* */
} ChoiceEventOption;

typedef struct {
	int type;
	List* options; /* List* of ChoiceEventOption* */
} ChoiceEvent;

typedef struct Character {
	char* name;
	char* description;
	List* events;

	/* Used to display “tags” and help the player recognise people. */
	short quester;
	short trader;
} Character;

Character* load_character(ParserElement*);
void quests(Game*);

#endif

