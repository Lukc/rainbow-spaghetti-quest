
#ifndef COMMANDS_H
#define COMMANDS_H

#include "entities.h"

typedef char** CommandFunction(void*);

typedef struct {
	char *name;
	char *shortcut;
	CommandFunction *callback;
	char *description;
} Command;

char** execute_commands(char*, Command*, void*);
void print_commands(Command*);
void print_logs(char**);

#endif

