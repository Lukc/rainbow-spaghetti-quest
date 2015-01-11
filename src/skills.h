
#ifndef SKILLS_H
#define SKILLS_H

#include "parser.h"
#include "game.h"
#include "places.h"

typedef struct Skill {
	/* Static data. */
	char* name;

	/* Dynamic data. */
	int cooldown;
	int experience;
} Skill;

/* For exclusive use in Place*. */
typedef struct {
	Skill* skill;
	List* drops; /* List* of Drop* */
} SkillDrops;

#include "game.h"
#include "entities.h"

void load_skill(Game*, List*);

Skill* get_skill_by_name(List*, char*);
List* get_skill_drops(Skill*, struct Place*);

int get_skill_level(int);
void lower_skills_cooldown(Game*);
void skills(Game*);

#endif

