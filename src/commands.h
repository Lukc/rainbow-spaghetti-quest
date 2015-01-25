
#ifndef COMMANDS_H
#define COMMANDS_H

#include "queue.h"

typedef struct {
	List* head;
	List* tail;
} Logs;

Logs* logs_new();
void logs_add(Logs*, char*);
void logs_print(Logs*);
int  logs_empty(Logs*);
void logs_free(Logs*);

#endif

