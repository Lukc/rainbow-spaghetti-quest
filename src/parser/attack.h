
#ifndef PARSER_ATTACK_H
#define PARSER_ATTACK_H

#include "../game.h"
#include "../attack.h"

void parser_load_attack(Game*, Attack*);

Attack* parser_get_attack(ParserElement*);

#endif

