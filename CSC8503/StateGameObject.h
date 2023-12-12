#pragma once
#include "GameObject.h"
#include "LevelData.h"
#include "GameWorld.h"
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
			EnemyObject(LevelData* l, GameWorld* g, const std::string& name = "");
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
			void SetObjectTarget(GameObject* o)
			{
				targetObject = o;
			}

			void MoveAlongPath(float dt);

			void SetLevelDataRef(LevelData* data)
			{
				levelData = data;
			}

			bool CanSeePlayer();
			float GetMoveSpeed() const
			{
				return moveSpeed;
			}
			void SetMoveSpeed(float f)
			{
				moveSpeed = f;
			}

			void SetTargetableObjects(vector<GameObject*> v)
			{
				playerObjects = v;
			}
		protected:
			std::string navigationGridFile;
			Vector3 targetDestination;
			vector<Vector3> pathFindingNodes;
			vector<GameObject*> playerObjects;
			GameObject* closestPlayerObject;
			GameObject* targetObject;

			LevelData* levelData = nullptr;
			bool isSearchingForSpot = true;
			GameWorld* gameWorld;
			float moveSpeed = 6.0f;
			float timeTraveling = 0.0f;
		};
	}
}
