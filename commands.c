#include <stdio.h>
#include <string.h>

#include "commands.h"

char**
execute_commands(char* line, Command *commands, Entity *player, Entity *enemy)
{
	char **logs = NULL;
	int i;

	if (line[0] == '\0')
		return NULL;

	for (i = 0; commands[i].name; i++)
	{
		if (!strcmp(line, commands[i].name))
		{
			if (commands[i].callback)
			{
				logs = commands[i].callback(player, enemy);
			}
			else
			{
				fprintf(
					stderr,
					"Command not implemented: %s :(((\n",
					commands[i].name
				);
			}
		}
	}

	return logs;
}

/* vim: set ts=4 sw=4 cc=80 : */
