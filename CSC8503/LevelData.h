#pragma once
#include "Transform.h"
namespace NCL
{
	namespace CSC8503
	{
		struct LevelGridUnit
		{
			LevelGridUnit* parent;

			LevelGridUnit* connected[4];

			Vector3		position;

			int type;

			LevelGridUnit() {
				for (int i = 0; i < 4; ++i) {
					connected[i] = nullptr;
				}
				type = 0;
				parent = nullptr;
			}
			~LevelGridUnit() {	}
		};
		class LevelData
		{
		public:
			LevelData();
			LevelData(const std::string& filename);
			LevelGridUnit* GetAllGridUnits() { return allGridUnits; }
			int GetGridSize() { return gridWidth * gridHeight; }
			int GetNodeSize() { return nodeSize; }
			Vector2 GetGridDimentions() { return Vector2(gridWidth, gridHeight); }
			std::string GetNavigationFile() { return navigationFile; }
			std::vector<LevelGridUnit> GetWalkableSpots() { return walkableSpots; }
		protected:
			int nodeSize;
			int gridWidth;
			int gridHeight;
			std::string navigationFile;
			LevelGridUnit* allGridUnits;
			std::vector<LevelGridUnit> walkableSpots;
		};
	}
}