#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "commands.h"
#include "string.h"
#include "colors.h"

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
		printf("%s\n", (char*) link->data);

		link = link->next;
	}
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
