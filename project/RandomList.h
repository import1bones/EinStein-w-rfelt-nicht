#pragma once
#include<random>
#include<time.h>
#define LENGTH_OF_LIST 1000
class RandomList
{
public:
	RandomList(int l, int r);
	~RandomList();
	int GetRandom();
private:
	int count_m;
	int limit_l;
	int limit_r;
	int list[LENGTH_OF_LIST];

	void InitalList();
};

