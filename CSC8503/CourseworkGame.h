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
#include "NetworkObject.h"
#include "GameServer.h"
#include "GameClient.h"
#include <string>
using std::string;
namespace NCL {
	namespace CSC8503 {



		class CourseworkGame : public PacketReceiver {
		public:
			CourseworkGame();
			~CourseworkGame();
			void ReceivePacket(int type, GamePacket* payload, int source) override;
	
			virtual void InitialiseGame();
			
			virtual void EraseWorld();
			virtual void UpdateGame(float dt);
			virtual void UpdateAsServer(float dt);
			virtual void UpdateAsClient(float dt);
			virtual void UpdateOuter(float dt);
			virtual void InitialiseGameAsServer();
			virtual void InitialiseGameAsClient();
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
			void ClientSendInputs();
			void AttachCameraPlayer(bool asServer, PlayerObject* pO, int playerID);
			void MovePlayerObject(float dt, PlayerObject* pO, int playerID);
			void GenerateLevel();
			void UpdatePathFindings(float dt);


			GameObject* AddFloorToWorld(const Vector3& position, Vector3 dimensions);
			GameObject* AddSphereToWorld(const Vector3& position, float radius, float inverseMass = 10.0f, int collisionLayer = LAYER_DEFAULT, bool isCollidable = true, bool rendered = true);
			GameObject* AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f, int collisionLayer = LAYER_DEFAULT, bool isCollidable = true, bool rendered = true);
			GameObject* AddCapsuleToWorld(const Vector3& position, float halfHeight, float radius, float inverseMass = 10.0f, int collisionLayer = LAYER_DEFAULT, bool isCollidable = true, bool rendered = true);

			
			PlayerObject* AddPlayerToWorld(int playerID, const Vector3& position);
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
			bool inPlayerMode = true;
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

			

			LevelData* levelData = nullptr;
			float outOfBounds[4] = {};
			std::vector<EnemyObject*> enemyObjects = std::vector<EnemyObject*>{};


			std::map<int, PlayerObject*> playerObject = {};
			std::map<int, GameObject*> playerGroundedCollider = {};
			std::map<int, Quaternion*> playerCameraRotation = {};
			std::map<int, char[8]> playerInputs = {};
			std::map<int, Quaternion> playerRotation = {};
			int selfClientID = 0;
			// NETWORKING

			GameServer* gameServer = nullptr;
			GameClient* gameClient = nullptr;
			void BroadcastSnapshot(bool deltaFrame);
			
			void OnPlayerConnect(int peerID);
			void OnPlayerDisconnect(int peerID);
			void InitialisePlayerAsServer(int playerID);
			void InitialisePlayerAsClient();

			
			//std::vector<GameObject*, int> networkVector;
		};
	
	
		
	}
}

