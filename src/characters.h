
#ifndef CHARACTERS_H
#define CHARACTERS_H

#include "game.h"

enum EVENT_TYPE {
	EVENT_MESSAGE,
	EVENT_CHOICE,
	EVENT_CONDITION,
	EVENT_GIVE_ITEM,
	EVENT_REMOVE_ITEM,
	EVENT_SET_VARIABLE,
	EVENT_FIRE
};

/* All Event structures must contain at least the following fields, in the
 * following order. You could see this as some sort of manually-managed
 * inheritance. */
typedef struct {
	int type;
	char* name;
} Event;

typedef struct {
	int type;
	char* name;

	char* from;
	char* text;
} MessageEvent;

typedef struct {
	char* text;
	List* events; /* List* of Event* */
} ChoiceEventOption;

typedef struct {
	int type;
	char* name;

	List* options; /* List* of ChoiceEventOption* */
} ChoiceEvent;

typedef struct {
	char* name;
	int value;
} Variable;

typedef struct {
	enum {
		VARIABLE_EXISTS,
		VARIABLE_NOT_EQUALS
	} condition;
	char* variable;
	int value;
} VariableCondition;

typedef struct {
	int type;
	char* name;

	List* items; /* List* of ItemStack* */
	List* variables; /* List* of VariableCondition* */

	List* then; /* List* of Event* */
	List* _else; /* List* of Event* */
} ConditionEvent;

typedef struct {
	int type;
	char* name;

	Item* item;
	int quantity;
} GiveItemEvent;

typedef GiveItemEvent RemoveItemEvent;

typedef struct {
	int type;
	char* name;

	char* variable;
	int value;
} SetVariableEvent;

typedef struct {
	int type;
	char* name;

	char* event;
} FireEvent;

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

