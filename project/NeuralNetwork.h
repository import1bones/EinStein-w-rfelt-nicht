#pragma once
#include"Paint.h"
#include<Python.h>
//datatsets' pro data struct;
struct statestack
{
	chessboard cb[6];
	short int choose;
};
void InitalNeuralNetwork();
int NeuralNetWork(statestack input);
void DestroyNeuralNetWork();
