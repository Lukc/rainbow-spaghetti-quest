
#ifndef PARSER_BUFF_H
#define PARSER_BUFF_H

#include "../game.h"
#include "../buff.h"

void parser_load_buff(Game*, Buff*);

Buff* parser_get_buff(ParserElement*);

#endif

