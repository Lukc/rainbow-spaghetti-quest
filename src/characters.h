
#ifndef CHARACTERS_H
#define CHARACTERS_H

#include "parser.h"
#include "game.h"

enum EVENT_TYPE {
	EVENT_MESSAGE
};

typedef struct {
	int type;
} Event;

typedef struct {
	int type;
	char* from;
	char* text;
} MessageEvent;

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

