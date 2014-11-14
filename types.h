
#ifndef _TYPES_H
#define _TYPES_H

enum TYPES {
	/* Physical types */
	TYPE_SLASHING,
	TYPE_IMPACT,
	TYPE_PIERCING,

	TYPE_ARCANE,
	TYPE_FIRE,
	TYPE_COLD,

	TYPE_MAX
};

char* type_string(int);
char* type_to_damage_string(int);
int type_id(char*);

#endif

