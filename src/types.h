
#ifndef _TYPES_H
#define _TYPES_H

enum TYPES {
	/* Physical types */
	TYPE_SLASHING,
	TYPE_IMPACT,
	TYPE_PIERCING,
	TYPE_POISON,

	TYPE_ARCANE,
	TYPE_FIRE,
	TYPE_COLD,
	TYPE_ELECTRIC,

	TYPE_MAX
};

char* type_to_string(int);
char* type_to_damage_string(int);
char* type_to_attack_name(int);
int type_id(char*);

#endif

