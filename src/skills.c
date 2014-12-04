#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "term.h"
#include "colors.h"

#include "skills.h"

char*
skill_to_string(int skill)
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
lower_skills_cooldown(Entity* player)
{
	int i;

	for (i = 0; i < SKILL_MAX; i++)
		if (player->skills_cooldown[i] > 0)
			player->skills_cooldown[i]--;
}

void
print_skill(int skill, int x, int selected, int cooldown, int usable)
{
	int c = selected ? '>' : ' ';

	move(x);
	if (usable)
	{
		if (cooldown)
			printf(RED);
		else
			printf(BRIGHT BLUE);
	}
	else
		printf(BLACK);
	printf("  %c %s", c, skill_to_string(skill));
	printf(NOCOLOR "\n");

	move(x);
	if (usable)
	{
		if (cooldown)
			printf(RED);
		else
			printf(BRIGHT BLUE);
	}
	else
		printf(BLACK);
	printf("  %c  cooldown: %i", c, cooldown);
	printf(NOCOLOR "\n");
}

void
skills(Game* game)
{
	int input = -42;
	int default_cooldown = 6;
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
			case 13: /* \n ??? */
				if (player->skills_cooldown[selection] == 0 &&
				    game->location->skill_drop[selection])
				{
					give_drop(player, game->location->skill_drop[selection]);
					player->skills_cooldown[selection] = default_cooldown;

					/* FIXME: Print the obtained items! */
				}
				break;
			default:
				error = "Unrecognized key!";
		}

		back_to_top();

		for (i = 0; i < 9; i++)
		{
			if (i < SKILL_MAX)
				print_skill(
					i, 0, i == selection, player->skills_cooldown[i],
					game->location->skill_drop[i] != NULL);
			else
				printf("\n\n");

			if (i + 9 < SKILL_MAX)
			{
				back(2);
				print_skill(i + 9, 40, i + 9 == selection,
					player->skills_cooldown[i + 9],
					game->location->skill_drop[i + 9] != NULL);
			}
		}

		menu_separator();

		printf("Cooldown after use:  %i\n", default_cooldown);
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
