#pragma once                                                                                                                                                                                                                                                                                                                                       
#include<vector>
#include<thread>
#include<mutex>
//#include"RandomList.h"
#define CONFIDENCE_INTERVAL 0.68
using std::vector;
struct node
{
	node *ahead;
	char type;
	bool choose = false;
	bool Dice[6]; 
	float pass = 0;
	float win = 0;
	float value = 0;
	vector<node *> next;
};
class MCST
{
public:
	MCST();
	~MCST();
	void Calc_Val(node *tem);
	void N_SetImitation(int W);
	void N_Move(int W);
	int N_FindMax(int L, int R);
	void I_SetImitation(int W);
	void I_Move(int W);
	void I_BackPropagation(bool win);
	int I_FindMax(int L, int R);
	void I_Recover();
	bool I_IsUsed(int Dice);
	void I_SetUse(int Dice);
	void N_BackStep();
private:
//	RandomList *randomMaxList;
	char transform_W_S(int W);
	node *root, *now;
	node *imitation;
};
