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
	unsigned int i, j;

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

		for (j = 0; j < 8 - strlen(commands[i].name); j++)
			printf(" ");

		printf("%s\n" NOCOLOR, commands[i].description);
	}

	printf("\n");
}

void
print_logs(char **logs)
{
	unsigned int i;

	if (logs)
	{
		for (i = 0; logs[i] && i < 16; i++)
		{
			printf("%s\n", logs[i]);
			free(logs[i]);
		}

		free(logs);

		printf("\n");
	}
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
	struct logs_list* link;

	link = (struct logs_list*) malloc(sizeof(struct logs_list));

	link->string = string;
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
	struct logs_list* link;

	link = l->head;

	while (link)
	{
		printf("%s\n", link->string);

		link = link->next;
	}

	printf("\n");
}

void
logs_free(Logs* l)
{
	struct logs_list* link, * temp;

	link = l->head;

	while (link)
	{
		temp = link;

		free(temp->string);

		link = link->next;

		free(temp);
	}

	free(l);
}

/* vim: set ts=4 sw=4 cc=80 : */
