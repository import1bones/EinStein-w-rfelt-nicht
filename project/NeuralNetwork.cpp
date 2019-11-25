#include"NeuralNetwork.h"
PyObject *PyModule;
PyObject *PyFunction;
PyObject *PyInput;
PyObject *PyResult;
void InitalNeuralNetwork()
{
	Py_Initialize();
	//多线程支持
	//PyEval_InitThreads();
	PyRun_SimpleString("import sys");
	PyRun_SimpleString("sys.path.append('./')");
	if (!Py_IsInitialized())
	{
		system("pause");
	}
	PyModule = PyImport_ImportModule("Machine_Learning");
	if (PyModule == NULL)
	{
		system("pause");
	}
	PyFunction = PyObject_GetAttrString(PyModule, "GetValue");
}
int NeuralNetWork(statestack input)
{
	PyInput = PyTuple_New(150);
	for (int s = 0; s < 6; s++)
	{
		for (int y = 0; y < 5; y++)
		{
			for (int x = 0; x < 5; x++)
			{
				PyTuple_SetItem(PyInput, s * 25 + y * 5 + x, Py_BuildValue("i", input.cb[s].set[x][y]));
			//  be used to test python to c;
			//	PyTuple_SetItem(PyInput, s * 25 + y * 5 + x, Py_BuildValue("i", 0));
			}
		}
	}
	PyResult = PyObject_CallFunctionObjArgs(PyFunction, PyInput, NULL);
	//PyErr_PrintEx(1);
	if (PyResult == NULL)
	{
		system("pause");
	}
	int result = PyLong_AsLong(PyResult);
	Py_DECREF(PyInput);
	return result;
}

void Py_Rename(string tem)
{
	PyRun_SimpleString("import os");
}
void DestroyNeuralNetWork()
{
	Py_Finalize();
}
