#include <stdlib.h>

#include "attack.h"

void
free_attack(Attack* attack)
{
	if (attack->name)
		free(attack->name);

	free(attack);
}

