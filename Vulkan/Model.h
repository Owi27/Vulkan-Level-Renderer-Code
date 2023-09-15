#pragma once
#include "h2bParser.h"
#include "FSLogo.h"
#include "../Gateware/Gateware.h"
#include <vulkan/vulkan.h>
#include <cstring>
//#include <vulkan/vulkan_xlib.h>
using namespace std;
using namespace H2B;

#define MAX_SUBMESH_PER_DRAW 20
struct SHADER_MODEL_DATA
{
	GW::MATH::GVECTORF sunDirection, sunColor;
	GW::MATH::GMATRIXF view, projection;

	GW::MATH::GMATRIXF matricies[MAX_SUBMESH_PER_DRAW];
	ATTRIBUTES materials[MAX_SUBMESH_PER_DRAW];
};

//shadermodeld

vector<VkBuffer> bufferVector;
vector<VkDeviceMemory> memoryVector;
class Model
{
public:
	int meshID;
	int modelID;
	Parser parse;
	SHADER_MODEL_DATA smd;
	VkBuffer buffer = nullptr;
	VkBuffer indicieBuffer = nullptr;
	VkDeviceMemory deviceMemory = nullptr;
	VkDeviceMemory indicieMemory = nullptr;
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

vector<Model> Text2Model(const char* filename)
{
	vector<Model> models;
	vector<float> rowData;

	ifstream file;

	file.open(filename, ios::in);

	string tempo;
	char temp[255];

	file.getline(temp, 255);
	tempo = temp;

	int idx = 0;
	while (!file.eof())
	{
		Model add = {};

		file.getline(temp, 255, '\n');
		tempo = temp;

		file.getline(temp, 255, '.');
		tempo = temp;
		if (tempo.size() != 0)
		{
			tempo = FileEndFix(tempo);
		}
		string filepath = "../DEV4Git/";
		filepath.append(tempo);
		filepath.append(".h2b");
		add.parse.Parse(filepath.c_str()); // Returning True!!!
		//lines.push_back(tempo);

		for (size_t j = 0; j < 4; j++)
		{
			file.getline(temp, 255, '(');
			tempo = temp;

			file.getline(temp, 255, ')');
			tempo = temp;
			char* token;
			token = strtok(temp, ",");

			while (token != NULL)
			{
				rowData.push_back(atof(token));
				token = strtok(NULL, ",");
			}
			//lines.push_back(tempo);
		}
		for (size_t i = 0; i < 16; i++)
		{

			add.smd.matricies[0].data[i] = rowData[i];
		}
		rowData.clear();
		//add.smd.matricies[i].
		file.getline(temp, 255, '\n');
		tempo = temp;

		add.meshID = idx;
		add.modelID = idx;

		for (size_t i = 0; i < add.parse.materialCount; i++)
		{
			add.smd.materials[i] = add.parse.materials[i].attrib;
		}

		models.push_back(add);

		idx++;
	}

	file.close();
	return models;
}