#include <stdlib.h>

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
		default:
			return "unknown";
	}
}

