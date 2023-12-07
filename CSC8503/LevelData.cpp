#include "LevelData.h"
#include <fstream>
#include "Assets.h"


using namespace NCL;
using namespace CSC8503;

LevelData::LevelData()
{
	nodeSize = 0;
	gridWidth = 0;
	gridHeight = 0;
	allGridUnits = nullptr;
}
LevelData::LevelData(const std::string& filename) : LevelData()
{
	std::ifstream infile(Assets::DATADIR + filename);
	navigationFile = filename;
	infile >> nodeSize;
	infile >> gridWidth;
	infile >> gridHeight;

	allGridUnits = new LevelGridUnit[gridWidth * gridHeight];

	for (int y = 0; y < gridHeight; ++y) {
		for (int x = 0; x < gridWidth; ++x) {
			LevelGridUnit& n = allGridUnits[(gridWidth * y) + x];
			char type = 0;
			infile >> type;
			n.type = type;
			n.position = Vector3((float)(x * nodeSize), 0, (float)(y * nodeSize));
		}
	}

	//now to build the connectivity between the nodes
	for (int y = 0; y < gridHeight; ++y) {
		for (int x = 0; x < gridWidth; ++x) {
			LevelGridUnit& n = allGridUnits[(gridWidth * y) + x];

			if (y > 0) { //get the above node
				n.connected[0] = &allGridUnits[(gridWidth * (y - 1)) + x];
			}
			if (y < gridHeight - 1) { //get the below node
				n.connected[1] = &allGridUnits[(gridWidth * (y + 1)) + x];
			}
			if (x > 0) { //get left node
				n.connected[2] = &allGridUnits[(gridWidth * (y)) + (x - 1)];
			}
			if (x < gridWidth - 1) { //get right node
				n.connected[3] = &allGridUnits[(gridWidth * (y)) + (x + 1)];
			}
			for (int i = 0; i < 4; ++i) {
				if (n.connected[i]) {
					if (n.connected[i]->type != '.') {
						n.connected[i] = nullptr;
					}
				}
			}
		}
	}
}
