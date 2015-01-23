
#ifndef COLORS_H
#define COLORS_H

/*#define BRIGHT   "\033[01m"*/
#define RED      1
#define GREEN    2
#define YELLOW   3
#define BLUE     color(1, 3, 5)
#define MAGENTA  5
#define CYAN     6
#define WHITE    7
#define GRAY     color(3, 3, 3)
#define BLACK    color(1, 1, 1)
/*#define NOCOLOR  "\033[00m"*/

int color(int, int, int);

void fg(int);
void bg(int);
void nocolor();

#endif

