#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "destination.h"

Destination*
parser_get_destination(ParserElement* element)
{
	Destination* destination;
	List* l;
	char* field;

	if (element->type == PARSER_STRING)
	{
		destination = (Destination*) malloc(sizeof(Destination));
		memset(destination, 0, sizeof(Destination));

		destination->name = parser_get_string(element, NULL);

		return destination;
	}
	else if (element->type == PARSER_LIST)
	{
		destination = (Destination*) malloc(sizeof(Destination));
		memset(destination, 0, sizeof(Destination));

		for (l = element->value; l; l = l->next)
		{
			element = l->data;
			field = element->name;

			if (!strcmp(field, "name"))
			{
				destination->name = parser_get_string(element, NULL);
			}
			else if (!strcmp(field, "if"))
			{
				if (element->type == PARSER_LIST)
					load_condition(&destination->condition, element->value);
				else
					fprintf(stderr, "[:%i] “if” field is not a list.\n",
						element->lineno);
			}
			else
			{
				fprintf(stderr, "<%s:%i> Unknown field in destination: “%s”.",
					element->filename, element->lineno, element->name);
			}
		}

		return destination;
	}

	return NULL;
}

