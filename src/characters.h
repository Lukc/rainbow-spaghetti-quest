
#ifndef CHARACTERS_H
#define CHARACTERS_H

#include "game.h"

#include "events.h"
#include "conditions.h"

typedef struct Character {
	char* name;
	char* description;
	List* events;

	/* Used to display “tags” and help the player recognise people. */
	short quester;
	short trader;
} Character;

Character* load_character(Game*, ParserElement*);
void quests(Game*);

#endif

