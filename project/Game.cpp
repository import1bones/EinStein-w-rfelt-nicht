#include "Game.h"



Game::Game()
{
	//randomDiceList = new RandomList(1, 7);
	int tem_dice;
	InitalNeuralNetwork();
	N_Inital();
	if (draw.GetMode() == 'A')
	{
		AutoNumberChessman();
	}
	else
	{
		ManualNumberChessman();
	}
	note.Record_ChessBoard(now.cb);
	bool lt_is_first;
	now.islt = true;//must be ture and top/false and bottom,because in neuralnetwork,lt is winner,the other is losser.
	//if islt is true,in this while,top is first-hand,bottom is last-hand
	if (draw.IsLtTrue())
	{
		now.islt = true;
		lt_is_first = true;
	}
	else
	{
		now.islt = false;
		lt_is_first = false;
	}
	//need a chosen to choose how to number chessman
	the_line_of_time.push_back(now);
	do
	{
		if (now.islt)
		{
			draw.PaintChessboard();
			draw.PaintChessman(now.cb);
			Record_now = now;
			Dice = draw.GetDice();
			tem_dice = Dice;
			Expansion();
			Simulation();
			Way = tree.N_FindMax(limit_l, limit_r);
			N_MoveChessman(Way);//ensure islt being not
			tree.N_Move(Way);
			draw.PaintChessboard();
			draw.PaintChessman(now.cb);
			Record_Dataset(Dice, Way, Record_now);
			note.Record_Action(tem_dice, now, Way);//
			the_line_of_time.push_back(now);
		}
		else
		{
			draw.PaintChessboard();
			draw.PaintChessman(now.cb);
			Record_now = now;
			Dice = draw.GetDice();
			MovingChessnan = draw.GetMovingChessman();
			Way = draw.GetWay();
			Way = (MovingChessnan - 1) * 3 + Way;
			if (N_MoveChessman(Way))//ensure islt being not
			{
				tree.N_SetImitation(Way);
				tree.N_Move(Way);
			}
			draw.PaintChessboard();
			draw.PaintChessman(now.cb);
			Record_Dataset(Dice, Way, Record_now);
			note.Record_Action(Dice, now, Way);
			the_line_of_time.push_back(now);
		}
		//some else,maybe,i want back some moves; 
		if (draw.IsBackMode())
		{
			for (;draw.IsContinueBack();)
			{
				tree.N_BackStep();
				the_line_of_time.pop_back();
				now = the_line_of_time[the_line_of_time.size() - 1];
				draw.PaintChessboard();
				draw.PaintChessman(now.cb);
				note.Record_DeleteEnd();
				Record_Dataset_Del();
			}
		}
	} while (Judge(now) == 0);
	if (Judge(now) == LT_SIGN)
	{
		if (lt_is_first)
		{
			note.Record_print(true);
			Record_PrintDataset();
		}
		else
		{
			note.Record_print(false);
			Record_PrintDataset();
		}
	}
	else
	{
		if (lt_is_first)
		{
			note.Record_print(false);
		}
		else
		{
			note.Record_print(true);
		}
	}

}


Game::~Game()
{
}

void Game::Record_Dataset(int tem_dice, int tem_way, state tem)
{
	statestack *tem_state = new statestack;
	int start, end;
	bool finded_l, finded_r;
	finded_l = false;
	finded_r = false;
	for (int t = 0; !(finded_l && finded_r); t += 1)
	{
		if (!finded_l)
		{
			start = ((tem_dice - 1 - t) * 3);
		}
		if (!finded_r)
		{
			end = ((tem_dice + t) * 3);
		}
		if (start < 0)
		{
			start = 0;
			finded_l = true;
		}
		if (end > 18)
		{
			end = 18;
			finded_r = true;
		}
		//需要将中间的已I_CanMove的剔出掉
		for (int i = 0; i < 6; i++)
		{
			tem_state->cb[i] = zero;
		}
		for (int j = start; j < end; j++) 
		{
			imitation = tem;
			if (I_CanMove(j))
			{
				if (I_MoveChessman(j))
				{
					//can move,will create new seat to storage;
					if (t == 0)
					{
						tem_state->cb[j % 3] = imitation.cb;
						finded_l = true;
						finded_r = true;
					}
					else
					{
						if (j < start + 3)
						{
							//0-3
							tem_state->cb[j % 3] = imitation.cb;
							finded_l = true;
						}
						if (j >= end - 3)
						{
							//3-6
							tem_state->cb[(j % 3) + 3] = imitation.cb;
							finded_r = true;
						}
					}
				}
				else
				{
					InputBox(NULL, NULL, "debug in 406");
				}
			}
		}
	}
	if ((tem_way - (tem_way % 3)) / 3 == tem_dice - 1)
	{
		tem_state->choose = tem_way % 3;
	}
	else
	{
		if ((tem_way - (tem_way % 3)) / 3 > tem_dice - 1)
		{
			tem_state->choose = tem_way % 3 + 3;
		}
		else
		{
			tem_state->choose = tem_way % 3;
		}
	}
	dataset.push_back(tem_state);
}

void Game::Record_Dataset_Del()
{
	dataset.pop_back();
}

void Game::Record_PrintDataset()
{
	for (int i = 0; i < dataset.size(); i++)
	{
		std::ofstream data(char(i + 'A') + "data.txt");
		data << dataset[i]->choose << std::endl;
		data.flush();
		for (int j = 0; j < 6; j++)
		{
			for (int y = 0; y < 5; y++)
			{
				for (int x = 0; x < 5; x++)
				{
					data<<dataset[i]->cb[j].set[x][y]<<" ";
					data.flush();
				}
				data << std::endl;
			}
		}
	}
}

void Game::N_Inital()
{
	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < 5; j++)
		{
			now.cb.set[i][j] = 0;
			zero.set[i][j] = 0;
		}
	}
}

void Game::stack_Inital()
{
	for (int i = 0; i < 6; i++)
	{
		NN_stack.cb[i] = zero;
	}
}

void Game::AutoNumberChessman()
{
	now.cb.set[0][0] = -6;
	now.cb.set[0][1] = -1;
	now.cb.set[0][2] = -3;
	now.cb.set[1][0] = -2;
	now.cb.set[1][1] = -5;
	now.cb.set[2][0] = -4;
	now.cb.set[2][4] = 4;
	now.cb.set[3][3] = 5;
	now.cb.set[3][4] = 2;
	now.cb.set[4][2] = 3;
	now.cb.set[4][3] = 1;
	now.cb.set[4][4] = 6;
}

void Game::ManualNumberChessman()
{
	now.cb.set[0][0] = -6;
	now.cb.set[0][1] = -1;
	now.cb.set[0][2] = -3;
	now.cb.set[1][0] = -2;
	now.cb.set[1][1] = -5;
	now.cb.set[2][0] = -4;
	draw.PaintChessboard();
	for (int i = 2; i < 5; i++)
	{
		for (int j = 2; j < 5; j++)
		{
			now.cb.set[i][j] = 0;
			if (i >= 2 && j >= 2 && i + j >= 6)
			{

				now.cb.set[i][j] = draw.NumberChessman(i, j);
				draw.PaintChessman(now.cb);
			}
		}
	}
}

void Game::I_Recover()
{
	imitation = now;
}

int Game::transform_W_D(int W)
{
	return ((W - (W % 3)) / 3 + 1);
}

bool Game::N_CanMove(int W)
{
	int move;
	if (now.islt)
	{
		move = LT_SIGN;
	}
	else
	{
		move = RB_SIGN;
	}
	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < 5; j++)
		{
			if (now.cb.set[i][j] == transform_W_D(W)*move)
			{
				switch (W % 3)
				{
				case 0:
					if (i - move <= 4 && i - move >= 0)
					{
						return true;
					}
					else
					{
						return false;
					}
					break;
				case 1:
					if (i - move <= 4 && i - move >= 0 && j - move <= 4 && j - move >= 0)
					{
						return true;
					}
					else
					{
						return false;
					}
					break;
				case 2:
					if (j - move <= 4 && j - move >= 0)
					{
						return true;
					}
					else
					{
						return false;
					}
					break;
				}
			}
		}
	}
	return false;
}

bool Game::I_CanMove(int W)
{
	int move;
	if (imitation.islt)
	{
		move = LT_SIGN;
	}
	else
	{
		move = RB_SIGN;
	}
	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < 5; j++)
		{
			if (imitation.cb.set[i][j] == transform_W_D(W)*move)
			{
				switch (W % 3)
				{
				case 0:
					if (i - move <= 4 && i - move >= 0)
					{
						return true;
					}
					else
					{
						return false;
					}
					break;
				case 1:
					if (i - move <= 4 && i - move >= 0 && j - move <= 4 && j - move >= 0)
					{
						return true;
					}
					else
					{
						return false;
					}
					break;
				case 2:
					if (j - move <= 4 && j - move >= 0)
					{
						return true;
					}
					else
					{
						return false;
					}
					break;
				}
			}
		}
	}
	return false;
}

bool Game::N_MoveChessman(int W)
{
	int move;
	if (now.islt)
	{
		move = LT_SIGN;
	}
	else
	{
		move = RB_SIGN;
	}
	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < 5; j++)
		{
			if (now.cb.set[i][j] == transform_W_D(W)*move)
			{
				switch (W % 3)
				{
				case 0:
					now.cb.set[i - move][j] = now.cb.set[i][j];
					break;
				case 1:
					now.cb.set[i - move][j - move] = now.cb.set[i][j];
					break;
				case 2:
					now.cb.set[i][j - move] = now.cb.set[i][j];
					break;
				}
				now.cb.set[i][j] = 0;
				now.islt = !now.islt;
				return true;
			}
		}
	}
	return false;
}

bool Game::I_MoveChessman(int W)
{
	int move;
	if (imitation.islt)
	{
		move = LT_SIGN;
	}
	else
	{
		move = RB_SIGN;
	}
	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < 5; j++)
		{
			if (imitation.cb.set[i][j] == transform_W_D(W)*move)
			{
				switch (W % 3)
				{
				case 0:
					imitation.cb.set[i - move][j] = imitation.cb.set[i][j];
					break;
				case 1:
					imitation.cb.set[i - move][j - move] = imitation.cb.set[i][j];
					break;
				case 2:
					imitation.cb.set[i][j - move] = imitation.cb.set[i][j];
					break;
				}
				imitation.cb.set[i][j] = 0;
				imitation.islt = !imitation.islt; 
				return true;
			}
		}
	}
	return false;
}

void Game::Expansion()
{
	int start, end;
	bool finded_l, finded_r;
	finded_l = false;
	finded_r = false;
	for (int t = 0; !(finded_l && finded_r); t++)
	{
		if (!finded_l)
		{
			start = ((Dice - 1 - t) * 3);
		}
		if (!finded_r)
		{
			end = ((Dice + t) * 3);
		}
		if (start < 0)
		{
			start = 0;
			finded_l = true;
		}
		if (end > 18)
		{
			end = 18;
			finded_r = true;
		}
		for (int j = start; j < end; j++)
		{
			I_Recover();
			if (I_CanMove(j))
			{
				if (I_MoveChessman(j))
				{
					if (t == 0)
					{
						finded_l = true;
						finded_r = true;

					}
					else
					{
						if (j < start + 3)
						{
							finded_l = true;
						}
						if (j >= end - 3)
						{
							finded_r = true;
						}
					}
					tree.N_SetImitation(j);
				}
				else
				{
					InputBox(NULL, NULL, "debug in 320");
				}
			}
		}
	}
	limit_l = start;
	limit_r = end;
}

void Game::Simulation()
{
	int start, end;
	bool finded_l, finded_r;
	for (int i = 0; i < NNUCT_NUM; i++)
	{
		I_Recover();
		tree.I_Recover();
		if (I_MoveChessman(tree.N_FindMax(limit_l, limit_r)))
		{
			tree.I_Move(tree.N_FindMax(limit_l, limit_r));
		}
		else
		{
			InputBox(NULL, NULL, "debug in 337");
		}
		do
		{
			tem_chessboard = imitation;
			//Dice = randomDiceList->GetRandom();
			Dice = rand() % 6 + 1;
			finded_l = false;
			finded_r = false;
			// before use stack must inital
			stack_Inital();
			//by the value of Dice,set nood in mcst;
			for (int t = 0; !(finded_l && finded_r); t += 1)
			{
				if (!finded_l)
				{
					start = ((Dice - 1 - t) * 3);
				}
				if (!finded_r)
				{
					end = ((Dice + t) * 3);
				}
				if (start < 0)
				{
					start = 0;
					finded_l = true;
				}
				if (end > 18)
				{
					end = 18;
					finded_r = true;
				}
				//需要将中间的已I_CanMove的剔出掉
				for (int j = start; j < end; j++)
				{
					imitation = tem_chessboard;
					if (I_CanMove(j))
					{
						if (I_MoveChessman(j))
						{
							//can move,will create new seat to storage;
							if (t == 0)
							{
								NN_stack.cb[j % 3] = imitation.cb;
								finded_l = true;
								finded_r = true;
							}
							else
							{
								if (j < start + 3)
								{
									//0-3
									NN_stack.cb[j % 3] = imitation.cb;
									finded_l = true;
								}
								if (j >= end - 3)
								{
									//3-6
									NN_stack.cb[(j % 3) + 3] = imitation.cb;
									finded_r = true;
								}
					 		}
							tree.I_SetImitation(j);
						}
						else
						{
							InputBox(NULL, NULL, "debug in 406");
						}
					}
				}
			}
			imitation = tem_chessboard;
			//I_FindMax() will be instead of NN
			//reason:NN is too slow,in some nood ,the same state will be use the method of NN again,we need a switch,if this Dice was use,don't use NN method
			if (tree.I_IsUsed(Dice))
			{
				if (I_MoveChessman(tree.I_FindMax(start, end)))
				{
					tree.I_Move(tree.I_FindMax(start, end));
				}
				else
				{
					InputBox(NULL, NULL, "debug in 425");
				}
			}
			else
			{
				NN_stack.choose = NeuralNetWork(NN_stack);
				The_sum_of_gv++;
				//must give a method for state which NN_stack.cb[tem]==zero
				if (Is_Chessboard_Zero(NN_stack.cb[NN_stack.choose]))
				{
					the_sum_of_non++;
					//NN method get way is illegal,we use find max method
					if (I_MoveChessman(tree.I_FindMax(start, end)))
					{
						tree.I_Move(tree.I_FindMax(start, end));
					}
					else
					{
						InputBox(NULL, NULL, "debug in 444");
					}
				}
				else
				{
					if (NN_stack.choose < 3)
					{
						if (I_MoveChessman(start + NN_stack.choose))
						{
							tree.I_SetUse(Dice);
							tree.I_Move(start + NN_stack.choose);
						}
						else
						{
							InputBox(NULL, NULL, "debug in 458");
						}
					}
					else
					{
						if (I_MoveChessman(end + NN_stack.choose - 6))
						{
							tree.I_SetUse(Dice);
							tree.I_Move(end + NN_stack.choose - 6);
						}
						else
						{
							InputBox(NULL, NULL, "debug in 470");
						}
					}
				}
			}
		} while (Judge(imitation) == 0);
		if (Judge(imitation) == LT_SIGN)
		{
			tree.I_BackPropagation(true);
		}
		else
		{
			tree.I_BackPropagation(false);
		}
	}
}

int Game::Judge(state s)
{
	int lt = 0;
	int rb = 0;
	if (s.cb.set[0][0] > 0)
	{
		return RB_SIGN;
	}
	if (s.cb.set[4][4] < 0)
	{
		return LT_SIGN;
	}
	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < 5; j++)
		{
			if (s.cb.set[i][j] != 0)
			{
				if (s.cb.set[i][j] < 0)
				{
					lt++;
				}
				else
				{
					rb++;
				}
			 }
		}
	}
	if (lt == 0)
	{
		return RB_SIGN;
	}
	else if (rb == 0)
	{
		return LT_SIGN;
	}
	else
	{
		return 0;
	}
}

bool Game::Is_Chessboard_Zero(chessboard tem)
{
	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < 5; j++)
		{
			if (tem.set[i][j] != 0)
			{
				return false;
			}
		}
	}
	return true;
}

vector<c_set *> the_note_of_chess;
Record::Record()
{
	//lt is rad
	host = HOST_AI;
	player = PLAYER;
	time_place = TIME_PLACE;
	game_name = GAME_NAME;

}

Record::~Record()
{

}

void Record::Record_ChessBoard(chessboard temp)
{
	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < 5; j++)
		{
			if (temp.set[i][j] != 0)
			{
				if (temp.set[i][j] < 0)
				{
					inital_r[abs(temp.set[i][j]) - 1][0] = char('A' + i);
					inital_r[abs(temp.set[i][j]) - 1][1] = char('5' - j);
				}
				else
				{
					inital_b[abs(temp.set[i][j]) - 1][0] = char('A' + i);
					inital_b[abs(temp.set[i][j]) - 1][1] = char('5' - j);
				}
			}
		}
	}
}

void Record::Record_Action(int dice, state temp, int tem_way)
{
	c_set *v_tem = new c_set;
	int chessman = (tem_way - (tem_way % 3)) / 3 + 1;
	v_tem->chessman = char(chessman + '0');
	v_tem->dice = char(dice + '0');
	int sign;
	if (!temp.islt)
	{
		sign = LT_SIGN;
		v_tem->side = 'R';
	}
	else
	{
		sign = RB_SIGN;
		v_tem->side = 'B';
	}
	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < 5; j++)
		{
			if (temp.cb.set[i][j] == sign * chessman)
			{
				v_tem->x = char('A' + i);
				v_tem->y = char('5' - j);
			}
		}
	}
	the_note_of_chess.push_back(v_tem);
}

void Record::Record_DeleteEnd()
{
	the_note_of_chess.pop_back();
}

void Record::Record_print(bool is_first_win)
{
	string first;
	string second;
	string state;
	if (the_note_of_chess[0]->side == 'R')
	{
		first = host;
		second = player;
	}
	else
	{
		first = player;
		second = host;
	}
	if (is_first_win)
	{
		state = "先手胜";
	}
	else
	{
		state = "后手胜";
	}
	string tem = "WTN-" + first + "vs" + second + "-" + state + "-" + time_place + "-" + game_name + ".txt";
	std::ofstream file;
	file.open("name.txt");
	file << tem;
	file.close();
	file.clear();
	file.open("abc.txt");
	for (; !file.is_open();)
	{
		file.clear();
		file.open("abc.txt");
	}
	file << "#[WTN][" << _TEXT(first);
	file << "][" << _TEXT(second);
	file << "][" << state;
	file << "][" << _TEXT(time_place);
	file << "][" << _TEXT(game_name);
	file << "]" << std::endl;
	file.flush();
	file << "R:";
	for (int i = 0; i < 6; i++)
	{
		file << inital_r[i][0] << inital_r[i][1];
		file << "-" << i + 1;
		file << ";";
		file.flush();
	}
	file << std::endl;
	file << "B:";
	file.flush();
	for (int i = 0; i < 6; i++)
	{
		file << inital_b[i][0] << inital_b[i][1];
		file << "-" << i + 1;
		file << ";";
		file.flush();
	}
	file << std::endl;
	for (int i = 0; i < the_note_of_chess.size(); i++)
	{
		file << i + 1 << ":" << the_note_of_chess[i]->dice;;
		file << ";(" << the_note_of_chess[i]->side;
		file << the_note_of_chess[i]->chessman << ",";
		file << the_note_of_chess[i]->x << the_note_of_chess[i]->y;
		file << ")" << std::endl;
		file.flush();
	}
	file.flush();
	file.close();
}
