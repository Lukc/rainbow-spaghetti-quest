#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "commands.h"
#include "colors.h"

Logs*
execute_commands(char* line, Command *commands, void *opts)
{
	Logs* logs;
	char* log;
	int i;

	if (line[0] == '\0')
		return NULL;

	for (i = 0; commands[i].name; i++)
	{
		if (!strcmp(line, commands[i].name) || !strcmp(line, commands[i].shortcut))
		{
			if (commands[i].callback)
			{
				return commands[i].callback(opts);
			}
			else
			{
				logs = logs_new();
				logs_add(logs, strdup(
					BRIGHT YELLOW "Command not implemented. :(((" NOCOLOR));

				return logs;
			}
		}
	}

	logs = logs_new();
	log = (char*) malloc(sizeof(char) * 80);
	snprintf(log, 80,
		BRIGHT YELLOW "Command not recognized: %s." NOCOLOR, line);
	logs_add(logs, log);

	return logs;
}

void
print_commands(Command *commands)
{
	int i, j;

	printf("Options:\n");

	for (i = 0; commands[i].name; i++)
	{
		if (!commands[i].callback)
			printf(YELLOW);
		else
			printf(WHITE);

		if (commands[i].shortcut)
			printf("  - (%s) %s: ", commands[i].shortcut, commands[i].name);
		else
			printf("  -      %s: ", commands[i].name);

		for (j = 0; j < (int) (10 - strlen(commands[i].name)); j++)
			printf(" ");

		printf("%s\n" NOCOLOR, commands[i].description);
	}

	printf("\n");
}

Logs*
logs_new()
{
	Logs* l = (Logs*) malloc(sizeof(Logs));

	l->head = NULL;
	l->tail = NULL;

	return l;
}

void
logs_add(Logs* l, char* string)
{
	List* link;

	link = (List*) malloc(sizeof(List));

	link->data = (void*) string;
	link->next = NULL;

	if (l->tail)
		l->tail->next = link;
	else
		l->head = link;

	l->tail = link;
}

void
logs_print(Logs* l)
{
	List* link;

	link = l->head;

	while (link)
	{
		printf("%s\n", link->data);

		link = link->next;
	}

	printf("\n");
}

int
logs_empty(Logs* l)
{
	return l->head == NULL;
}

void
logs_free(Logs* l)
{
	List* link, * temp;

	link = l->head;

	while (link)
	{
		temp = link;

		free(temp->data);

		link = link->next;

		free(temp);
	}

	free(l);
}

/* vim: set ts=4 sw=4 cc=80 : */
