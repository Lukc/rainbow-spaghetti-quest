#include <stdlib.h>
#include <stdio.h>

#include "term.h"
#include "colors.h"

int
isexit(int c)
{
	return
		c == 0 || c == -1 || c == 27 ||
		c == 3 || c == 4 /* ^C and ^D */;
}

int
getch()
{
	int c;

	system("stty raw -echo");

	c = getchar();

	/* Here comez the hackz */
	if (c == 27)
	{
		int c2 = getchar();

		if (c2 == 91)
	 		c = - getchar();
		else
			/* Pretty sure this is wrong. */
			c = c2;
	}
	else if (c == 12) /* ^L */
		system("clear");

	system("stty sane");

	return c;
}

void
menu_separator()
{
	int i;

	nocolor();
	for (i = 0; i < 80; i++)
		printf("─");
	printf("\n");
}

/**
 * Puts the cursor back to the first character of the first line of
 * the screen without clearing it.
 */
void
back_to_top()
{
	printf("\033[0H");
}

/**
 * Moves the cursor X characters to the right.
 */
void
move(int where)
{
	printf("\033[%iG", where);
}

/**
 * Moves a cursor X lines higher.
 */
void
back(int lines)
{
	printf("\033[%iA", lines);
}

