
#include "item.h"
#include "attack.h"

void
parser_load_item(Game* game, Item* item)
{
	List* l;

	for (l = item->attacks; l; l = l->next)
		parser_load_attack(game, (Attack*) l->data);
}

