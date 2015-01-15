#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "class.h"
#include "drop.h"
#include "attack.h"

void
parser_load_class(Game* game, Class* class)
{
	List* l;

	for (l = class->drop; l; l = l->next)
	{
		parser_load_drop(game, (Drop*) l->data);
	}

	for (l = class->attacks; l; l = l->next)
	{
		Attack* attack = l->data;

		parser_load_attack(game, attack);
	}
}


/* vim: set ts=4 sw=4 cc=80 : */

