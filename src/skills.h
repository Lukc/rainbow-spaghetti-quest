
#ifndef SKILLS_H
#define SKILLS_H

/*
 * @fixme: ’k, so... how do I differenciate collect skills from crafting ones?
 * @fixme: Need about 10 skills (because it fits a (24-4)/2 lines term)
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

char* skill_string(int);

void skills(Game*);

#endif

