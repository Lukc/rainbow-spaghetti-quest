#include <string.h>

char*
strcut(char* string, int n)
{
	static char cut[256];

	strncpy(cut, string, n - 1);

	cut[n] = '\0';

	return cut;
}

