#include "MCST.h"



MCST::MCST()
{
	//randomMaxList = new RandomList(10000, 11000);
	root = new node();
	now = root;
}


MCST::~MCST()
{
}

void MCST::Calc_Val(node * tem)
{
	//maybe we can create a confidence interval for neural network;
	tem->value = tem->win / tem->pass + (sqrtf(log(root->pass) / tem->pass)*CONFIDENCE_INTERVAL);
}

void MCST::N_SetImitation(int W)
{
	bool bool_tem = true;
	for (int i = 0; i < now->next.size(); i++)
	{
		if (now->next[i]->type == transform_W_S(W))
		{
			bool_tem = false;
		}
	}
	if (bool_tem)
	{
		node *tem = new node();
		//生成随机数
		//tem->value = randomMaxList->GetRandom();
		tem->value = rand() % 10 + 10000;
		tem->ahead = now;
		tem->type = transform_W_S(W);
		now->next.push_back(tem);
	}
}

void MCST::N_Move(int W)
{
	for (int i = 0; i < now->next.size(); i++)
	{
		if (now->next[i]->type == transform_W_S(W))
		{
			now = now->next[i];
			break;
		}
	}
}

int MCST::N_FindMax(int L, int R)
{
	float max;
	int sign;
	max = -1;
	for (int i = 0; i < now->next.size(); i++)
	{
		if (now->next[i]->pass > 0)
		{
			Calc_Val(now->next[i]);
		}
		if (now->next[i]->type >= transform_W_S(L) && now->next[i]->type < transform_W_S(R) && now->next[i]->value >= max)
		{
			sign = now->next[i]->type - 'A';
			max = now->next[i]->value;
		}	
	}
	return sign;
} 

void MCST::I_SetImitation(int W)
{
	bool bool_tem = true;
	for (int i = 0; i < imitation->next.size(); i++)
	{
		if (imitation->next[i]->type == transform_W_S(W))
		{
			bool_tem = false;
			break;
		}
	}
	if (bool_tem)
	{
		node *tem = new node();
		//生成随机数
		//tem->value = randomMaxList->GetRandom();
		tem->value = rand() % 10 + 10000;
		tem->ahead = imitation;
		tem->type = transform_W_S(W);
		imitation->next.push_back(tem);
	}
}

void MCST::I_Move(int W)
{
	bool moved = false;
	for (int i = 0; i < imitation->next.size(); i++)
	{
		if (imitation->next[i]->type == transform_W_S(W))
		{
			imitation = imitation->next[i];
			break;
		}
	}
}

void MCST::I_BackPropagation(bool win)
{
	for (; imitation != NULL; imitation = imitation->ahead)
	{
		imitation->pass += 1;
		if (win)
		{
			imitation->win += 1;
		}
	}
}

int MCST::I_FindMax(int L, int R)
{
	float max;
	int sign;
	max = -1;
	for (int i = 0; i < imitation->next.size(); i++)
	{
		if (imitation->next[i]->pass > 0)
		{
			Calc_Val(imitation->next[i]);
		}
		if (imitation->next[i]->type >= transform_W_S(L) && imitation->next[i]->type < transform_W_S(R) && imitation->next[i]->value>max)
		{
			sign = imitation->next[i]->type - 'A';
			max = imitation->next[i]->value;
		}
	}
	return sign;
}

void MCST::I_Recover()
{
	imitation = now;
}

bool MCST::I_IsUsed(int Dice)
{
	if (imitation->Dice[Dice - 1])
	{
		return true;
	}
	else
	{
		return false;
	}
}

void MCST::I_SetUse(int Dice)
{
	imitation->Dice[Dice - 1] = true;
}

void MCST::N_BackStep()
{
	now = now->ahead;
	I_Recover();
}

char MCST::transform_W_S(int W)
{
	return char('A' + W);
}
