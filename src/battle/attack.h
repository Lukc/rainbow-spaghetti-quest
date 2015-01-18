
#ifndef BATTLE_ATTACK_H
#define BATTLE_ATTACK_H

#include "../entities.h"
#include "../commands.h"
#include "../attack.h"

/**
 * Possible return values for can_use_attack().
 */
#define E_NO_MANA    -1
#define E_NO_HEALTH  -2
#define E_COOLDOWN   -3

int can_use_attack(Entity*, AttackData*);

void attack(Entity*, Attack*, Entity*, Logs*);
Logs* command_attack(Game*, AttackData*);

#endif

