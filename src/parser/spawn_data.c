#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "spawn_data.h"

SpawnData*
parser_get_spawn_data(ParserElement* element)
{
	SpawnData* spawn;
	char* field;

	if (element->type == PARSER_STRING)
	{
		char* string = parser_get_string(element, NULL);

		if (string)
		{
			spawn = malloc(sizeof(*spawn));

			/* Will be converted to Class* later. */
			spawn->class = (Class*) string;
			spawn->frequency = 1;

			return spawn;
		}
	}
	else if (element->type == PARSER_LIST)
	{
		char* class = NULL;
		int frequency = 0;
		List* sl = element->value;

		for (; sl; sl = sl->next)
		{
			element = sl->data;
			field = element->name;

			if (!strcmp(field, "class"))
				class = parser_get_string(element, NULL);
			else if (!strcmp(field, "frequency"))
				frequency = parser_get_integer(element, NULL);
			else
				fprintf(stderr, "<%s:%i> Unknown field: “%s”.\n",
					element->filename, element->lineno, field);
		}

		if (class)
		{
			spawn = malloc(sizeof(*spawn));

			spawn->class = (Class*) class;
			spawn->frequency = frequency > 0 ? frequency : 1;

			return spawn;
		}
	}
	else
		fprintf(stderr, "<%s:%i> "
			"Field “Random enemy” is not a string or a list.\n",
			element->filename, element->lineno);

	return NULL;
}

