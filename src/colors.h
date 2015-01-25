
#ifndef COLORS_H
#define COLORS_H

#define RED      color(4, 0, 0)
#define GREEN    color(0, 4, 0)
#define YELLOW   color(4, 4, 1)
#define BLUE     color(1, 2, 5)
#define MAGENTA  color(2, 1, 4)
#define CYAN     color(0, 4, 4)
#define WHITE    color(4, 4, 4)
#define GRAY     color(3, 3, 3)
#define BLACK    color(1, 1, 1)

int color(int, int, int);

void fg(int);
void bg(int);
void nocolor();

#endif

