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
		case TYPE_ARCANE:
			return "arcane";
		case TYPE_FIRE:
			return "fire";
		case TYPE_COLD:
			return "cold";
		default:
			return "unknown";
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

