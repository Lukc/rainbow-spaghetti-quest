
#ifndef FOCUS_H
#define FOCUS_H

#include "../entities.h"
#include "../queue.h"
#include "../attack.h"

void focus(Entity*, Queue*);
Queue* command_focus(Game*);

#endif

