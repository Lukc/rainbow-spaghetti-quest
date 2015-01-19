#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "string.h"

#ifdef NEED_STRDUP
char*
strdup(const char* in)
{
	int i;
	char* out = malloc(strlen(in));

	for (i = 0; in[i]; i++)
		out[i] = in[i];
	out[i] = '\0';

	return out;
}
#endif

#ifdef NEED_STRCASECMP
int
strcasecmp(const char* a, const char* b)
{
	int i;

	for (i = 0; a[i] & b[i]; i++)
	{
		char ca, cb;

		ca = tolower(a[i]);
		cb = tolower(b[i]);

		if (ca < cb)
			return -1;
		else if (ca > cb)
			return 1;
	}

	return 0;
}
#endif

