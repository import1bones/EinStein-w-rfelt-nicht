#pragma once
#include<graphics.h>
#include<string>
#include<fstream>
#include<iostream>
using std::string;
struct chessboard
{
	int set[5][5];
};

class Paint
{
public:
	Paint();
	~Paint();
	void PaintChessboard();
	void PaintChessman(chessboard cb);
	bool IsLtTrue();
	int GetDice();
	int GetMovingChessman();
	int GetWay();
	char GetMode();
	bool IsBackMode();
	bool IsContinueBack();
	int NumberChessman(int x, int y);
};

