#include <stdlib.h>
#include <string.h>

#include "types.h"

char*
type_string(int type)
{
	switch (type)
	{
		case TYPE_SLASHING:
			return "slashing";
		case TYPE_IMPACT:
			return "impact";
		case TYPE_PIERCING:
			return "piercing";
		case TYPE_POISON:
			return "poison";
		case TYPE_ARCANE:
			return "arcane";
		case TYPE_FIRE:
			return "fire";
		case TYPE_COLD:
			return "cold";
		case TYPE_ELECTRIC:
			return "electric";
		default:
			return "unknown";
	}
}

char*
type_to_damage_string(int type)
{
	switch(type)
	{
		case TYPE_SLASHING:
			return "slashed";
		case TYPE_PIERCING:
			return "impaled";
		case TYPE_IMPACT:
			return "pounded";
		case TYPE_POISON:
			return "poisoned";
		case TYPE_FIRE:
			return "burned";
		case TYPE_COLD:
			return "froze";
		case TYPE_ARCANE:
			return "exorcized";
		case TYPE_ELECTRIC:
			return "electrified";
		default:
			return "hurt";
	}
}

int
type_id(char* string)
{
	int i;

	for (i = 0; i < TYPE_MAX; i++)
	{
		if (!strcmp(type_string(i), string))
			return i;
	}

	return -1;
}

