#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "buff.h"

Buff*
parser_get_buff(ParserElement* element)
{
	Buff* buff;
	List* l;

	if (element->type != PARSER_LIST)
	{
		fprintf(stderr, "<%s:%i> Buff is not a list.\n",
			element->filename, element->lineno);

		return NULL;
	}

	buff = malloc(sizeof(Buff));
	memset(buff, 0, sizeof(Buff));

	for (l = element->value; l; l = l->next)
	{
		element = l->data;

		if (!strcmp(element->name, "name"))
			buff->name = parser_get_string(element, NULL);
		else if (!strcmp(element->name, "attack"))
			buff->attack = parser_get_integer(element, NULL);
		else if (!strcmp(element->name, "defense"))
			buff->defense = parser_get_integer(element, NULL);
		else
			fprintf(stderr, "<%s:%i> Unknown property ignored: %s.\n",
				element->filename, element->lineno, element->name);
	}

	return buff;
}

