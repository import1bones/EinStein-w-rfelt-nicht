#include "Paint.h"



Paint::Paint()
{
	initgraph(600, 600);
}


Paint::~Paint()
{
	closegraph();
}

void Paint::PaintChessboard()
{
	setfillcolor(WHITE);
	setlinecolor(BLACK);
	solidrectangle(0, 0, 600, 600);
	for (int i = 0; i < 6; i++)
	{
		line(50, 100 * i + 50, 550, 100 * i + 50);
		line(100 * i + 50, 50, 100 * i + 50, 550);
	}
}

void Paint::PaintChessman(chessboard cb)
{
	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < 5; j++)
		{
			if (cb.set[i][j] != 0)
			{
				if (cb.set[i][j] > 0)
				{
					setfillcolor(BLUE);
					solidrectangle(100 * i + 51, 100 * j + 51, 100 * i + 149, 100 * j + 149);
				}
				else
				{
					setfillcolor(RED);
					solidrectangle(100 * i + 51, 100 * j + 51, 100 * i + 149, 100 * j + 149);
				}
				outtextxy(100 * i + 51, 100 * j + 51, char(abs(cb.set[i][j]))+'0');
			}
		}
	}
}

bool Paint::IsLtTrue()
{
	char tem[2];
	InputBox(tem, 2, "is AI first?(y/n)");
	if (tem[0] == 'y' || tem[0] == 'Y')
	{
		return true;
	}
	else
	{
		return false;
	}
}

int Paint::GetDice()
{
	char tem[2];
	InputBox(tem, 2, "input the value of dice");
	return (tem[0] - '0');
}

int Paint::GetMovingChessman()
{
	char tem[2];
	InputBox(tem, 2, "input the number of moving-chessman");
	return (tem[0] - '0');
}

int Paint::GetWay()
{
	char tem[2];
	InputBox(tem, 2, "input the value of way\n|*|0|    |1|2|\n|2|1|    |0|*|");
	return (tem[0] - '0');
}

char Paint::GetMode()
{
	char tem[2];
	InputBox(tem, 2, "input mode of number chessman(auto-A,manual-M)");
	return tem[0];
}

bool Paint::IsBackMode()
{
	char tem[2];
	InputBox(tem, 2, "do you want to use back-mode(y/n)");
	if (tem[0] == 'y' || tem[0] == 'Y')
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool Paint::IsContinueBack()
{
	char tem[2];
	InputBox(tem, 2, "do you want to back a move(y/n)");
	if (tem[0] == 'y' || tem[0] == 'Y')
	{
		return true;
	}
	else
	{
		return false;
	}
}

int Paint::NumberChessman(int x, int y)
{

	char tem[2];
	setfillcolor(GREEN);
	solidrectangle(100 * x + 51, 100 * y + 51, 100 * x + 149, 100 * y + 149);
	InputBox(tem, 2, "number for light");
	return tem[0] - '0';
}

