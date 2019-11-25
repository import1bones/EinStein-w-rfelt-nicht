#include "RandomList.h"



RandomList::RandomList(int l, int r)
{
	srand(unsigned(time(NULL)));
	limit_l = l;
	limit_r = r;
	InitalList();
}


RandomList::~RandomList()
{
}

int RandomList::GetRandom()
{
	count_m++;
	if (count_m == LENGTH_OF_LIST - 1)
	{
		count_m = rand() % 1000;
	}
	return list[count_m];
}

void RandomList::InitalList()
{
	int tem = limit_r - limit_l;
	for (int i = 0; i < LENGTH_OF_LIST; i++)
	{
		list[i] = rand() % tem + limit_l;
	}
	count_m = list[1] % 900;
}
