
#ifndef BATTLE_ATTACK_H
#define BATTLE_ATTACK_H

#include "../entities.h"
#include "../commands.h"
#include "../attack.h"

int can_use_attack(Entity*, Attack*);

void attack(Entity*, Attack*, Entity*, Logs*);
Logs* command_attack(Game*, Attack*);

#endif

