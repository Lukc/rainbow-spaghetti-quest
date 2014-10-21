#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "commands.h"
#include "colors.h"

char**
execute_commands(char* line, Command *commands, Entity *player, Entity *enemy)
{
	char **logs = NULL;
	int i;

	if (line[0] == '\0')
		return NULL;

	for (i = 0; commands[i].name; i++)
	{
		if (!strcmp(line, commands[i].name) || !strcmp(line, commands[i].shortcut))
		{
			if (commands[i].callback)
			{
				return commands[i].callback(player, enemy);
			}
			else
			{
				logs = (char**) malloc(sizeof(char*) * 2);
				logs[0] = strdup(
					BRIGHT YELLOW "Command not implemented. :(((" NOCOLOR);
				logs[1] = NULL;

				return logs;
			}
		}
	}

	logs = (char**) malloc(sizeof(char*) * 2);
	logs[0] = (char*) malloc(sizeof(char) * 80);
	snprintf(logs[0], 80,
		BRIGHT YELLOW "Command not recognized: %s." NOCOLOR, line);
	logs[1] = NULL;

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

		printf("  - (%s) %s: ", commands[i].shortcut, commands[i].name);

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

/* vim: set ts=4 sw=4 cc=80 : */
