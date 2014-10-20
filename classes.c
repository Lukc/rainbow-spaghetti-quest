#include<stdio.h>
#include<stdlib.h>

void load_classes (char* filename)
{
	FILE *f = fopen (filename, " r");
	char *str = NULL;

	while (getline(&str, 0, f))
	{
		free(str);
		str = NULL;
	}

	fclose(f);
}

/* vim: set ts=4 sw=4 cc=80 : */
