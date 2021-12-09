#pragma once
#include "../h2bParser.h"
using namespace std;
using namespace H2B;

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


