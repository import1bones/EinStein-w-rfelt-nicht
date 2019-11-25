#pragma once
#include"MCST.h"
#include"NeuralNetwork.h"
//in P V E mode,LT is ai's side.
#define LT_SIGN -1
#define RB_SIGN 1
#define NNUCT_NUM 1000
const string HOST_AI = "假面棋士";
const string PLAYER = "";
const string TIME_PLACE = "2019/10/11";
const string GAME_NAME = "test";
struct state
{
	chessboard cb;
	bool islt;
};
struct c_set
{
	char side;
	char dice;
	char chessman;
	char x;
	char y;
};

class Record
{
public:
	Record();
	~Record();
	void Record_ChessBoard(chessboard temp);
	void Record_Action(int tem_dice, state temp, int tem_way);
	void Record_DeleteEnd();
	void Record_print(bool is_first_win);
private:
	char inital_r[6][2];
	char inital_b[6][2];
	string host;
	string player;
	string time_place;
	string game_name;
};

class Game
{
public:
	Game();
	~Game();
private:
	//RandomList *randomDiceList;
	MCST tree;
	Record note;
	chessboard zero;//be used to fill zero in stack; 
	Paint draw;
	state now;
	state Record_now;
	vector<state> the_line_of_time;
	vector<statestack*> dataset;//
	state imitation, tem_chessboard;
	statestack NN_stack;//just be used to store temp data for neural-network in simuation(); 
	int Way;//be used N 传递数据
	int Dice;
	int MovingChessnan;
	int limit_l, limit_r;
	long long int The_sum_of_gv = 0;
	long long int the_sum_of_non = 0;

	void Record_Dataset(int tem_dice, int tem_way, state tem);
	void Record_Dataset_Del();
	void Record_PrintDataset();

	void N_Inital();
	void stack_Inital();
	void AutoNumberChessman();
	void ManualNumberChessman();
	void I_Recover();
	int transform_W_D(int W);
	bool N_CanMove(int W);  
	bool I_CanMove(int W);
	bool N_MoveChessman(int W);
	bool I_MoveChessman(int W);
	void Expansion();
	//if four thread ,need four Simulation;
	void Simulation();
	int Judge(state s);
	bool Is_Chessboard_Zero(chessboard tem);
};


