
#ifndef TERM_H
#define TERM_H

int isexit(int);
int getch();
void menu_separator();
void back_to_top();
void move(int);
void back(int);

#define KEY_UP    (-65)
#define KEY_DOWN  (-66)
#define KEY_RIGHT (-67)
#define KEY_LEFT  (-68)

#endif

