#pragma once
#include "../h2bParser.h"
#include "../FSLogo.h"
#include "../../Gateware/Gateware.h"
using namespace std;
using namespace H2B;

vector<char*> h2bVec;

#define MAX_SUBMESH_PER_DRAW 1024
struct SHADER_MODEL_DATA
{
	GW::MATH::GVECTORF sunDirection, sunColor;
	GW::MATH::GMATRIXF view, projection;

	GW::MATH::GMATRIXF matricies[MAX_SUBMESH_PER_DRAW];
	OBJ_ATTRIBUTES materials[MAX_SUBMESH_PER_DRAW];
};

class Model
{
public:
	int meshID;
	int modelID;
	Parser parse;
	SHADER_MODEL_DATA smd;
};

string FileEndFix(string strings)
{
	string temp;

	// Cylinder End Delete
	if (strings.at(strings.size() - 1) == 'r' && strings.at(strings.size() - 2) == 'e')
	{
		for (size_t i = 0; i < 9; i++)
		{
			strings.pop_back();
		}

		temp = strings;
		return temp;
	}
	// Plane End Delete
	else if (strings.at(strings.size() - 1) == 'e' && strings.at(strings.size() - 2) == 'n')
	{
		for (size_t i = 0; i < 6; i++)
		{
			strings.pop_back();
		}

		temp = strings;
		return temp;
	}
	// Cube End Check
	else if (strings.at(strings.size() - 1) == 'e' && strings.at(strings.size() - 2) == 'b')
	{
		for (size_t i = 0; i < 5; i++)
		{
			strings.pop_back();
		}

		temp = strings;
		return temp;
	}
}

vector<string> Text2Model(const char* filename, int _amountOfModels)
{
	vector<string> lines;

	ifstream file;

	file.open(filename, ios::in);

	if (file.is_open())
	{
		string tempo;
		char temp[255];
		file.getline(temp, 255);
		tempo = temp;

		for (size_t i = 0; i < _amountOfModels; i++)
		{
			file.getline(temp, 255, '\n');
			tempo = temp;

			file.getline(temp, 255, '.');
			tempo = temp;
			tempo = FileEndFix(tempo);
			lines.push_back(tempo);

			for (size_t j = 0; j < 4; j++)
			{
				file.getline(temp, 255, '(');
				tempo = temp;

				file.getline(temp, 255, ')');
				tempo = temp;
				lines.push_back(tempo);
			}

			file.getline(temp, 255, '\n');
			tempo = temp;
		}
	}

	return lines;
}