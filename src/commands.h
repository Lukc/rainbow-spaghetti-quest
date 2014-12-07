
#ifndef COMMANDS_H
#define COMMANDS_H

#include "list.h"

typedef struct Logs* CommandFunction(void*);

typedef struct Command {
	char *name;
	char *shortcut;
	CommandFunction *callback;
	char *description;
} Command;

typedef struct Logs {
	List* head;
	List* tail;
} Logs;

Logs* execute_commands(char*, Command*, void*);
void print_commands(Command*);

Logs* logs_new();
void logs_add(Logs*, char*);
void logs_print(Logs*);
void logs_free(Logs*);

#endif

