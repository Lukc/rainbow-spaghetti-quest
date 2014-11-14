#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "travel.h"
#include "term.h"
#include "colors.h"

static void
first_visit(Battle* data)
{
	List* list;
	int i;

	for (list = data->location->on_first_visit; list; list = list->next)
	{
		char** image = list->data;

		system("clear");

		for (i = 0; image[i]; i++)
		{
			/* Ugly hack to print stuff on the last line. */
			printf("%s", image[i]);

			if (i != 0)
			{
				back(1);
				printf("\n");
			}

			if (image[i+1])
				printf("\n");
		}

		getch();
	}

	system("clear");
}

/**
 * Fires the first_visit events if needed.
 */
void
check_first_visit(Battle* data)
{
	if (!has_visited(data, data->location) &&
		data->location->on_first_visit)
	{
		first_visit(data);

		list_add(&data->visited, data->location);
	}
}

void
travel(Battle* data)
{
	List* list;
	char* info = NULL;
	char input = -42;
	int i;

	system("clear");

	while (!isexit(input))
	{
		if (input == -42)
			;
		else if (isdigit(input))
		{
			Place* place;

			input = input - '0';

			if ((place = list_nth(data->location->destinations, input)))
			{
				/* If not, we’ll be damn screwed... */
				if (place)
				{
					data->location = place;

					check_first_visit(data);
				}

				system("clear");
				return;
			}
			else
			{
				info = "Hey! Invalid input!";
			}
		}
		else
		{
			info = "Hey! Invalid input!";
		}

		printf(BRIGHT BLUE "\n >> Places you can go to:\n\n" NOCOLOR);

		i = 0;
		for (list = data->location->destinations; list; list = list->next)
		{
			printf(WHITE "  (%i)  %s\n" NOCOLOR,
				i, ((Place*) list->data)->name);

			i++;
		}

		for (; i < 17; i++)
			printf("\n");

		printf("\nPlease select a destination.\n");

		menu_separator();

		if (info)
		{
			printf("%s", info);
			back(1);
			printf("\n");
		}

		input = getch();
		system("clear");
	}

	system("clear");

	return;
}

