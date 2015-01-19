#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "string.h"
#include "term.h"
#include "colors.h"

#include "skills.h"

List*
get_skill_drops(Skill* skill, Place* place)
{
	List* l;
	SkillDrops* sd;

	for (l = place->skill_drops; l; l = l->next)
	{
		sd = l->data;

		if (sd->skill == skill)
			return sd->drops;
	}

	return NULL;
}

void
lower_skills_cooldown(Game* game)
{
	List* l;
	Skill* skill;

	for (l = game->skills; l; l = l->next)
	{
		skill = l->data;

		if (skill->cooldown > 0)
			skill->cooldown--;
	}
}

int
skill_level_to_experience(int level)
{
	int xp = 0;
	int i;

	for (i = 0; i < level; i++)
		xp = xp + xp + 100;

	return xp;
}

int
get_skill_level(int experience)
{
	int level = 0;

	while (skill_level_to_experience(level) <= experience)
		level++;

	return level - 1;
}

void
print_skill(Skill* skill, Game* game, int selected)
{
	int c = selected ? '>' : ' ';
	int i, max, threshold;
	List* drops;

	drops = get_skill_drops(skill, game->location);

	printf(BRIGHT WHITE " %c%c " NOCOLOR, c, c);
	if (drops)
	{
		if (skill->cooldown)
			printf(BRIGHT YELLOW);
		else
			printf(BRIGHT GREEN);
	}
	else
		fg(1, 1, 1);

	printf(" %s", skill->name);
	printf(NOCOLOR "\n");

	if (drops)
		printf(WHITE);
	else
		fg(1, 1, 1);
	printf("  cd: <");
	if (drops)
	{
		if (skill->cooldown)
			printf(YELLOW);
		else
			printf(BLUE);
	}
	else
		fg(1, 1, 1);
	for (i = 0; i < 68; i++)
		printf("%c", i * 100 / 68 < (12 - skill->cooldown) * 100 / 12 ? '=' : ' ');

	if (drops)
		printf(WHITE);
	else
		fg(1, 1, 1);

	printf(">\n");

	max = skill_level_to_experience(
		get_skill_level(skill->experience) + 1
	) - skill_level_to_experience(
		get_skill_level(skill->experience)
	);
	threshold = skill->experience - skill_level_to_experience(
		get_skill_level(skill->experience));
	if (drops)
		printf(WHITE);
	else
		fg(1, 1, 1);
	printf("  xp: <"BLUE);
	for (i = 0; i < 68; i++)
		printf("%c", i * 100 / 68 < threshold * 100 / max ? '=' : ' ');
	if (drops)
		printf(WHITE);
	else
		fg(1, 1, 1);
	printf(">");

	printf(NOCOLOR "\n");
}

void
skills(Game* game)
{
	int input = -42;
	int selection = 0;
	int i;
	char* log;
	Skill* skill;
	List* drops;
	List* l;
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
				selection = selection < list_size(game->skills) - 1 ?
					selection + 1 : list_size(game->skills) - 1;
				break;
			case KEY_UP:
				selection = selection > 0 ? selection - 1 : 0;
				break;
			case 'u':
				if (skill->cooldown == 0 && drops)
				{
					List* given;

					given = give_drop(player, drops);
					skill->cooldown = list_size(given) * 2;

					/* 10xp/item collected */
					skill->experience += skill->cooldown * 5;

					log = strdup(
						BRIGHT GREEN " >> " WHITE "Items collected." NOCOLOR);

					/* FIXME: Print the obtained items! */
				}
				break;
			default:
				log = strdup(
					BRIGHT RED " >> " WHITE "Unrecognized key!" NOCOLOR);
		}

		skill = list_nth(game->skills, selection);
		drops = get_skill_drops(skill, game->location);

		back_to_top();

		l = game->skills;
		for (i = 0; i < selection - selection % 6; i++)
			l = l->next;

		for (i = 0; i < 6; i++)
		{
			printf("%80s\n%80s\n%80s\n", "", "", "");
			back(3);

			if (l)
			{
				print_skill(l->data, game, i == selection % 6);

				l = l->next;
			}
			else
				printf("\n\n\n");
		}

		menu_separator();

		printf(WHITE);
		printf("%-20s", "Cooldown:");
		if (skill->cooldown > 4)
			printf(BRIGHT RED);
		else if (skill->cooldown > 0)
			printf(BRIGHT YELLOW);
		else
			printf(BRIGHT);
		printf("%-5i\n", skill->cooldown);
		printf(NOCOLOR);

		back(1);
		move(40);
		printf(YELLOW " (d)  See drop\n" NOCOLOR);

		printf(WHITE "%-20s%-5i\n" NOCOLOR,
			"Skill Level:", get_skill_level(skill->experience));
		back(1);
		move(40);
		printf(WHITE " (u)  Use skill\n" NOCOLOR);

		printf("%80s", "");
		move(0);
		printf(WHITE "%-20s%i/%i\n" NOCOLOR,
			"Experience:", skill->experience,
			skill_level_to_experience(get_skill_level(skill->experience) + 1));
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

void
load_skill(Game* game, List* elements)
{
	List* l;
	Skill* s;

	s = malloc(sizeof(*s));
	memset(s, 0, sizeof(*s));

	for (l = elements; l; l = l->next)
	{
		ParserElement* element = l->data;

		if (!strcmp(element->name, "name"))
			s->name = parser_get_string(element, NULL);
		else
			fprintf(stderr, "[Skill:%i] Unrecognized element ignored: %s.\n",
				element->lineno, element->name);
	}

	if (s->name)
		list_add(&game->skills, s);
	else
		fprintf(stderr, "Invalid/Incomplete skill.\n");
}

Skill*
get_skill_by_name(List* list, char* name)
{
	Skill* skill;

	for (; list; list = list->next)
	{
		skill = list->data;

		if (!strcasecmp(skill->name, name))
			return skill;
	}

	return NULL;
}

/* vim: set ts=4 sw=4 : */
