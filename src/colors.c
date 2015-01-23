#include <stdio.h>

#include "colors.h"

/**
 * Converts three RGB values between 0 and 6 into a 256-color number.
 */
int
color(int r, int g, int b)
{
	r = r > 5 ? 5 : r;
	g = g > 5 ? 5 : g;
	b = b > 5 ? 5 : b;

	return 16 + r * 6 * 6 + g * 6 + b;
}

/**
 * Depends on a 256colors terminal.
 *
 * @fixme: Support for 88colors and 16colors terminals might be cool.
 *         Of course, I don’t really care about it, because 256 > 88 > 16,
 *         but some people might want to use those anyway.
 *
 *         88c terminals should basically s/6/4/g;s/5/3/g. Not sure about 16c.
 *
 * Also note that grayscales are pretty much wasted with this thing, being
 * limited to 6 shades instead of the 24 available.
 */
void
fg(int c)
{
	printf("\033[38;5;%im", c);
}

void
bg(int c)
{
	printf("\033[48;5;%im", c);
}

void
nocolor()
{
	printf("\033[0m");
}

