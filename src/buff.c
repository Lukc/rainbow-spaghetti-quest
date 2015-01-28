
#include "buff.h"

int
buff_is_empty(Buff* buff)
{
	return buff->attack == 0 && buff->defense == 0;
}

