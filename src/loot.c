#include <stdlib.h>
#include <stdio.h>

#include "loot.h"

#include "items.h"
#include "term.h"
#include "colors.h"

/**
 * @param list: List* of Item*
 */
void
loot_screen(List* list)
{
	int i;

	if (list)
	{
		system("clear");
		printcf(WHITE, -1, "\nYou gained the following items:\n");

		for (; list; list = list->next)
		{
			int c = GRAY;
			Item* item = list->data;

			if (item->slot >= 0)
				c = WHITE;
			else if (item->on_use && item->on_use->strikes > 0)
				c = YELLOW;
			else if (is_item_usable(item))
				c = GREEN;

			printcf(-1, -1, "  - ");
			printcf(c, -1, "%s\n", item->name);
		}

		back_to_top();
		for (i = 0; i < 21; i++)
			printf("\n");

		printcf(WHITE, -1, "\nPress any key to continue...\n\n");

		getch();
	}
}


