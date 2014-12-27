#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

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
	int i;

	move(x);
	printf(BRIGHT WHITE " %c%c " NOCOLOR, c, c);
	if (usable)
	{
		if (cooldown)
			printf(BRIGHT YELLOW);
		else
			printf(BRIGHT GREEN);
	}
	else
		fg(1, 1, 1);

	printf(" %s", skill_to_string(skill));
	printf(NOCOLOR "\n");

	move(x);
	if (usable)
	{
		if (cooldown)
			printf(YELLOW);
		else
			printf(BLUE);
	}
	else
		fg(1, 1, 1);

	printf("      <");
	for (i = 0; i < 28; i++)
		printf("%c", i * 100 / 28 < (6 - cooldown) * 100 / 6 ? '=' : ' ');
	printf(">");

	printf(NOCOLOR "\n");
}

void
skills(Game* game)
{
	int input = -42;
	int default_cooldown = 6;
	int selection = 0;
	int i;
	char* log;
	Entity* player = game->player;

	system("clear");

	while (input != 'l')
	{
		log = NULL;

		switch (input)
		{
			case -42:
			case KEY_CLEAR:
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
			case 'u':
				if (player->skills_cooldown[selection] == 0 &&
				    game->location->skill_drop[selection])
				{
					give_drop(player, game->location->skill_drop[selection]);
					player->skills_cooldown[selection] = default_cooldown;

					log = strdup(
						BRIGHT GREEN " >> " WHITE "Items collected." NOCOLOR);

					/* FIXME: Print the obtained items! */
				}
				break;
			default:
				log = strdup(
					BRIGHT RED " >> " WHITE "Unrecognized key!" NOCOLOR);
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

		printf(WHITE);
		printf("Cooldown:            ");
		if (player->skills_cooldown[selection] > 4)
			printf(BRIGHT RED);
		else if (player->skills_cooldown[selection] > 0)
			printf(BRIGHT YELLOW);
		else
			printf(BRIGHT);
		printf("%-5i\n", player->skills_cooldown[selection]);
		printf(NOCOLOR);

		printf("Possible drop here:  (none)\n");
		back(1);
		move(40);
		printf(WHITE " (u)  Use skill\n" NOCOLOR);

		printf("Needed resources:    (none)\n");
		back(1);
		move(40);
		printf(WHITE " (l)  Leave\n" NOCOLOR);

		menu_separator();

		printf("%-80s", log ? log : "");
		back(1);
		printf("\n");

		if (log)
		{
			free(log);
		}

		input = getch();
	}

	system("clear");
}

/* vim: set ts=4 sw=4 : */
