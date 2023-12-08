#include "../NCLCoreClasses/KeyboardMouseController.h"

#pragma once
#include "GameTechRenderer.h"
#ifdef USEVULKAN
#include "GameTechVulkanRenderer.h"
#endif
#include "PhysicsSystem.h"

#include "StateGameObject.h"
#include "StateMachine.h"
#include "StateTransition.h"
#include "State.h"

#include <string>
namespace NCL {
	namespace CSC8503 {
		class CourseworkGame {
		public:
			CourseworkGame();
			~CourseworkGame();

			virtual void UpdateGame(float dt);

		protected:
			void InitialiseAssets();

			void InitCamera();
			void UpdateKeys();

			void InitWorld();

			/*
			These are some of the world/object creation functions I created when testing the functionality
			in the module. Feel free to mess around with them to see different objects being created in different
			test scenarios (constraints, collision types, and so on). 
			*/
			void InitGameExamples();

			void InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius);
			void InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing);
			void InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims);
			void BridgeConstraintTest();

			void InitDefaultFloor();

			bool SelectObject();
			void MoveSelectedObject();
			void DebugObjectMovement();
			void LockedObjectMovement();
			void AttachCameraPlayer();
			void MovePlayerObject(float dt);
			void GenerateLevel();
			void UpdatePathFindings(float dt);

			GameObject* AddFloorToWorld(const Vector3& position, Vector3 dimensions);
			GameObject* AddSphereToWorld(const Vector3& position, float radius, float inverseMass = 10.0f, int collisionLayer = LAYER_DEFAULT, bool isCollidable = true, bool rendered = true);
			GameObject* AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f, int collisionLayer = LAYER_DEFAULT, bool isCollidable = true, bool rendered = true);
			GameObject* AddCapsuleToWorld(const Vector3& position, float halfHeight, float radius, float inverseMass = 10.0f, int collisionLayer = LAYER_DEFAULT, bool isCollidable = true, bool rendered = true);

			PlayerObject* AddPlayerToWorld(const Vector3& position);
			EnemyObject* AddEnemyToWorld(const Vector3& position);
			GameObject* AddBonusToWorld(const Vector3& position);

#ifdef USEVULKAN
			GameTechVulkanRenderer*	renderer;
#else
			GameTechRenderer* renderer;
#endif
			PhysicsSystem*		physics;
			GameWorld*			world;

			KeyboardMouseController controller;

			bool useGravity;
			bool inSelectionMode;
			bool inPlayerMode = false;
			float		forceMagnitude;

			GameObject* selectionObject = nullptr;

			Mesh*	capsuleMesh = nullptr;
			Mesh*	cubeMesh	= nullptr;
			Mesh*	sphereMesh	= nullptr;

			Texture*	basicTex	= nullptr;
			Shader*		basicShader = nullptr;

			//Coursework Meshes
			Mesh*	charMesh	= nullptr;
			Mesh*	enemyMesh	= nullptr;
			Mesh*	bonusMesh	= nullptr;

			//Coursework Additional functionality	
			GameObject* lockedObject	= nullptr;
			Vector3 lockedOffset		= Vector3(0, 0, 0);
			void LockCameraToObject(GameObject* o) {
				lockedObject = o;
			}

			GameObject* objClosest = nullptr;

			StateGameObject* AddStateObjectToWorld(const Vector3& position);
			StateGameObject* testStateObject;

			PlayerObject* playerObject = nullptr;
			GameObject* playerGroundedCollider = nullptr;
			Controller* playerController = nullptr;
			Quaternion* playerCameraRotation;
			LevelData* levelData = nullptr;

			std::vector<EnemyObject*> enemyObjects = std::vector<EnemyObject*>{};
		};
	
	
		
	}
}

