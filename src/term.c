#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

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

/**
 * Similar to snprintf, except that it writes in an array of Cells instead of
 * an array of char.
 *
 * @fixme: unicode :’(
 */
int
ccnprintf(Cell* dest, size_t n, int fg, int bg, const char* format, ...)
{
	char buffer[1024];
	va_list l;
	unsigned int i;

	va_start(l, format);
	vsnprintf(buffer, sizeof(buffer), format, l);
	va_end(l);

	for (i = 0; buffer[i] && i < n - 1; i++)
	{
		dest[i].ch = buffer[i];
		dest[i].fg = fg;
		dest[i].bg = bg;
	}

	dest[i].ch = '\0';

	return i;
}

/**
 * Prints an array of cells to stdout.
 * Takes care of color changes and everything.
 */
void
printc(Cell* cells)
{
	int i;
	/* Let’s always assume default values… */
	int lfg = 0;
	int lbg = 0;

	for (i = 0; cells[i].ch; i++)
	{
		if (cells[i].fg != lfg)
		{
			lfg = cells[i].fg;
			fg(lfg);
		}
		if (cells[i].bg != lbg)
		{
			lbg = cells[i].bg;
			bg(lbg);
		}

		printf("%c", cells[i].ch);
	}
}

Cell*
strtocells(const char* string, int fg, int bg)
{
	size_t len;
	Cell* out;
	unsigned int i;

	len = strlen(string);
	out = malloc(sizeof(Cell) * (len + 1));

	for (i = 0; i < len; i++)
	{
		out[i].ch = string[i];
		out[i].fg = fg;
		out[i].bg = bg;
	}

	out[len].ch = '\0';

	return out;
}

int
copy_cells(Cell* dest, Cell* origin, size_t maxlen)
{
	unsigned int i;

	for (i = 0; i < maxlen && origin[i].ch != '\0'; i++)
		dest[i] = origin[i];

	dest[i].ch = '\0';

	return i;
}

/* vim: set ts=4 sw=4 cc=80 : */

