
#ifndef CONDITIONS_H
#define CONDITIONS_H

#include "game.h"
#include "list.h"

typedef struct {
	char* name;
	int value;
} Variable;

typedef struct {
	enum {
		VARIABLE_EXISTS,
		VARIABLE_EQUALS,
		VARIABLE_NOT_EQUALS,
		VARIABLE_LOWER,
		VARIABLE_GREATER,
		VARIABLE_LOWER_OR_EQUAL,
		VARIABLE_GREATER_OR_EQUAL
	} condition;
	char* variable;
	int value;
} VariableCondition;

typedef struct {
	List* items; /* List* of ItemStack* */
	List* variables; /* List* of VariableCondition* */
	List* has_statuses; /* List* of Status* */
} Condition;

int condition_check(Game*, Condition*);

void load_condition(Condition*, List*);

#endif

