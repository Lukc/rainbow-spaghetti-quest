
#ifndef PARSER_DROP_H
#define PARSER_DROP_H

#include "../game.h"
#include "../drop.h"

void parser_load_drop(Game*, Drop*);

Drop* parser_get_drop(ParserElement*);

#endif

