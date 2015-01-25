
#ifndef BATTLE_ATTACK_H
#define BATTLE_ATTACK_H

#include "../entities.h"
#include "../queue.h"
#include "../attack.h"

/**
 * Possible return values for can_use_attack().
 */
#define E_NO_MANA    -1
#define E_NO_HEALTH  -2
#define E_COOLDOWN   -3

int can_use_attack(Entity*, AttackData*);
void reset_charges(Entity*);

void attack(Entity*, Attack*, Entity*, Queue*);
Queue* command_attack(Game*, AttackData*);

#endif

