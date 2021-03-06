
#ifndef PARSER_H
#define PARSER_H

#include "commands.h"
#include "attack.h"

enum PARSER_TYPES {
	PARSER_INTEGER,
	PARSER_STRING,
	PARSER_LIST
};

typedef struct ParserElement {
	int type;
	char* name;
	void* value;
	int lineno;
	char* filename;
} ParserElement;

#include "list.h"
#include "drop.h"
#include "game.h"

List* parse_file(char*);
void parser_free(ParserElement*);

int parser_get_integer(ParserElement*, Logs*);
char* parser_get_string(ParserElement*, Logs*);

void load_game(Game*, char*);
void unload_game(Game*);

#endif

