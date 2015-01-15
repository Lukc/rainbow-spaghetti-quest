#include "mana_cost.h"

int
get_mana_cost(Entity* e, Attack* attack)
{
	List* l;
	int mana = attack->mana;

	if (mana == 0)
		return 0;
	else if (mana > 0)
	{
		int max = get_max_mana(e);

		return e->mana + mana > max ?
			max - mana : mana;
	}
	else
	{
		for (l = e->statuses; l; l = l->next)
		{
			StatusData* status = l->data;

			if (status->status->increases_mana_costs)
				mana *= 2;
		}

		return mana;
	}
}

