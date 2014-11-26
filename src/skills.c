#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "term.h"
#include "colors.h"

#include "skills.h"

char*
skill_string(int skill)
{
	switch (skill)
	{
		case SKILL_WOODCUTTING:
			return "woodcutting";
		case SKILL_STONECUTTING:
			return "stonecutting";
		case SKILL_MINING:
			return "mining";
		case SKILL_GATHERING:
			return "gathering";

		case SKILL_COOKING:
			return "cooking";
		case SKILL_ALCHEMY:
			return "alchemy";
		case SKILL_WOODWORKING:
			return "woodworking";
		case SKILL_STONEWORKING:
			return "stoneworking";
		case SKILL_METALWORKING:
			return "metalworking";
		case SKILL_LEATHERWORKING:
			return "leatherworking";

		default:
			return NULL;
	}
}

void
print_skill(int skill, int x, int selected, int cooldown)
{
	int c = selected ? '>' : ' ';

	move(x);
	printf("  %c %s\n", c, skill_string(skill));
	move(x);
	printf("  %c  cooldown: %i\n", c, cooldown);
}

void
skills(Game* game)
{
	int input = -42;
	int selection = 0;
	int i;
	char* error;
	Entity* player = game->player;

	system("clear");

	while (!isexit(input))
	{
		error = NULL;

		switch (input)
		{
			case -42:
				break;
			case KEY_DOWN:
				if (selection % 9 != 8)
					selection = selection < SKILL_MAX - 1 ?
						selection + 1 : SKILL_MAX - 1;
				break;
			case KEY_UP:
				if (selection % 9 != 0)
					selection = selection > 0 ? selection - 1 : 0;
				break;
			case KEY_LEFT:
				selection = selection >= 9 ? selection - 9 : selection;
				break;
			case KEY_RIGHT:
				selection =
					selection + 9 < SKILL_MAX ? selection + 9 : selection;
				break;
			default:
				error = "Unrecognized key!";
		}

		back_to_top();

		for (i = 0; i < 9; i++)
		{
			if (i < SKILL_MAX)
				print_skill(i, 0, i == selection, player->skills_cooldown[i]);
			else
				printf("\n\n");

			if (i + 9 < SKILL_MAX)
			{
				back(2);
				print_skill(i + 9, 40, i + 9 == selection,
					player->skills_cooldown[i + 9]);
			}
		}

		menu_separator();

		printf("Cooldown after use:  4247\n");
		printf("Possible drop here:  (none)\n");
		printf("Needed resources:    (none)\n");

		menu_separator();

		if (error)
		{
			printf("%s", error);
			back(1);
			printf("\n");
		}

		input = getch();
	}

	system("clear");
}

/* vim: set ts=4 sw=4 : */
