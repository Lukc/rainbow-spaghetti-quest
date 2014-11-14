
#ifndef PARSER_H
#define PARSER_H

#include "list.h"
#include "commands.h"

enum PARSER_TYPES {
	PARSER_INTEGER,
	PARSER_STRING,
	PARSER_LIST
};

typedef struct ParserElement {
	int type;
	char* name;
	void* value;
	struct ParserElement* parent;
} ParserElement;

List* load_file(char*);
void parser_free(ParserElement*);

int parser_get_integer(ParserElement*, Logs*);
char* parser_get_string(ParserElement*, Logs*);

#endif

