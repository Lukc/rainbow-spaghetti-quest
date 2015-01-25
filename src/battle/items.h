
#ifndef BATTLE_ITEMS_H
#define BATTLE_ITEMS_H

#include "../entities.h"
#include "../items.h"
#include "../queue.h"

void use_item(Entity*, Item*, Entity*, Queue*);
Queue* command_use_item(Game*, Entity*, Item*);

#endif

