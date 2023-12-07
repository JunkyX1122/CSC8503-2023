#pragma once
#include "GameObject.h"
#include "LevelData.h"

namespace NCL {
    namespace CSC8503 {
        class StateMachine;
        class StateGameObject : public GameObject  {
        public:
            StateGameObject();
            ~StateGameObject();

            virtual void Update(float dt);

			
        protected:
            void MoveLeft(float dt);
            void MoveRight(float dt);

            StateMachine* stateMachine;
            float counter;

			
        };
   
		class EnemyObject : public StateGameObject
		{
		public:
			EnemyObject(LevelData* l);
			~EnemyObject();

			bool IsNavigationSet()
			{
				return navigationGridFile != "";
			}
			void SetNavigationFile(const std::string& filename)
			{
				navigationGridFile = filename;
			}

			Vector3 GetTargetDestination() const
			{
				return targetDestination;
			}

			void SetTargetDestination(Vector3 v)
			{
				targetDestination = v;
			}

			void ResetPathFindingNodes()
			{
				pathFindingNodes = vector<Vector3>{};
			}

			bool FindPath(Vector3 target);

			void DrawNavigationPath();

			Vector3 GetNextPathNode()
			{
				if (1 < pathFindingNodes.size()) return pathFindingNodes[1];
				return targetDestination;
			}

			void InitialiseStateMachine()
			{
				stateMachine = new StateMachine;
			}

			StateMachine* GetStateMachine() const
			{
				return stateMachine;
			}
			void SetPlayerObjectTarget(GameObject* o)
			{
				playerObject = o;
			}

			void MoveAlongPath(float dt);

			void SetLevelDataRef(LevelData* data)
			{
				levelData = data;
			}

		protected:
			std::string navigationGridFile;
			Vector3 targetDestination;
			vector<Vector3> pathFindingNodes;
			GameObject* playerObject;
			LevelData* levelData = nullptr;
			bool isSearchingForSpot = true;
		};
	}
}
