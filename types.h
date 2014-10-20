
#ifndef _TYPES_H
#define _TYPES_H

typedef struct {
	int current;
	int max;
} Bar;

typedef struct List {
	struct List *next;
	void *data;
} List;

#endif

