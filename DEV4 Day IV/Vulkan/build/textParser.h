#pragma once
#include "Model.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
using namespace std;
vector<Model> models;

class textParser
{
	
};

Model* Text2Model(const char* filename)
{
	ifstream file;

	file.open(filename, ios::in);

	if (file.is_open())
	{
		char* temp;
		file.getline(temp, 255, 'M');
	}
}

