
#ifndef TERM_H
#define TERM_H

/**
 * Meant to represent a terminal’s cell.
 *
 * It’s based on termbox’ struct cell structure. We’re kinda planing on
 * adding termbox support in the future.
 */
typedef struct {
	int ch;
	int fg;
	int bg;
} Cell;

int isexit(int);
int getch();
void menu_separator();
void back_to_top();
void move(int);
void back(int);

void term_init();
int ccnprintf(Cell*, size_t, int, int, const char*, ...);
int printcf(int, int, const char*, ...);
void printc(Cell*);

Cell* strtocells(const char*, int, int);
int copy_cells(Cell*, Cell*, size_t);

#define KEY_UP    (-65)
#define KEY_DOWN  (-66)
#define KEY_RIGHT (-67)
#define KEY_LEFT  (-68)
#define KEY_CLEAR  (12)
#define KEY_ENTER  (13)

#endif

