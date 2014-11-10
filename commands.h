
#ifndef COMMANDS_H
#define COMMANDS_H

#include "entities.h"

typedef struct Logs* CommandFunction(void*);

typedef struct Command {
	char *name;
	char *shortcut;
	CommandFunction *callback;
	char *description;
} Command;

struct logs_list {
	struct logs_list* next;
	char* string;
};

typedef struct Logs {
	struct logs_list* head;
	struct logs_list* tail;
} Logs;

Logs* execute_commands(char*, Command*, void*);
void print_commands(Command*);

Logs* logs_new();
void logs_add(Logs*, char*);
void logs_print(Logs*);
void logs_free(Logs*);

#endif

