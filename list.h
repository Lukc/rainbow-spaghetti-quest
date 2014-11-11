
#ifndef LIST_H
#define LIST_H

typedef struct List {
	void* data;
	struct List* next;
} List;

void list_add(List**, void*);
void* list_nth(List*, int);
int list_size(List*);

#endif

