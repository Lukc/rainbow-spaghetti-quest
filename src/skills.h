
#ifndef SKILLS_H
#define SKILLS_H

/*
 * @fixme: Add more skills. Yeah, srsly.
 */
enum SKILLS {
	SKILL_WOODCUTTING,
	SKILL_STONECUTTING,
	SKILL_MINING,
	SKILL_GATHERING,

	SKILL_COOKING,
	SKILL_ALCHEMY,
	SKILL_WOODWORKING,
	SKILL_STONEWORKING,
	SKILL_METALWORKING,
	SKILL_LEATHERWORKING,

	SKILL_MAX
};

#include "game.h"
#include "entities.h"

char* skill_to_string(int);

void lower_skills_cooldown(struct Entity*);
void skills(Game*);

#endif

