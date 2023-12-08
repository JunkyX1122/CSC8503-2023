#include "StateGameObject.h"
#include "StateTransition.h"
#include "StateMachine.h"
#include "State.h"
#include "PhysicsObject.h"
#include "Debug.h"
#include "Ray.h"
using namespace NCL;
using namespace CSC8503;

StateGameObject::StateGameObject()
{
	counter = 0.0f;
	stateMachine = new StateMachine();

}
StateGameObject::~StateGameObject() {
	delete stateMachine;
}

void StateGameObject::Update(float dt) 
{
	stateMachine->Update(dt);
}

void StateGameObject::MoveLeft(float dt) 
{
	//std::cout << "GO LEFT\n";
	GetPhysicsObject()->AddForce({ -100, 0, 0 });
	counter += dt;
}

void StateGameObject::MoveRight(float dt) 
{
	GetPhysicsObject()->AddForce({ 100, 0, 0 });
	counter -= dt;
}





EnemyObject::EnemyObject(LevelData* l, GameWorld* g, const std::string& n)
{
	levelData = l;
	gameWorld = g;
	navigationGridFile = l->GetNavigationFile();
	name = n;
	State* Wandering = new State([&](float dt)->void
		{
			if (levelData)
			{
				int size = levelData->GetGridSize();
				int maxX = levelData->GetGridDimentions().x;
				int maxZ = levelData->GetGridDimentions().y;
				bool foundPosition = false;
				Vector3 target;
				if ((this->GetTransform().GetPosition() - this->GetTargetDestination()).Length() < levelData->GetNodeSize())
				{
					isSearchingForSpot = true;
				}
				while (!foundPosition)
				{
					if (isSearchingForSpot)
					{
						Vector3 target(levelData->GetWalkableSpots()[RandomValue(0,levelData->GetWalkableSpots().size()-1)].position);
						target.y = 0;
						this->SetTargetDestination(target);
					}
					if (this->FindPath(this->GetTargetDestination()))
					{
						this->SetMoveSpeed(6.0f);
						this->DrawNavigationPath();
						this->MoveAlongPath(dt);
						foundPosition = true;
						isSearchingForSpot = false;
					}
				}
			}
		});
	State* ChasePlayer = new State([&](float dt)->void
		{
			isSearchingForSpot = true;
			if (playerObject)
			{
				Vector3 target = playerObject->GetTransform().GetPosition();
				target.y = 0;
				this->SetMoveSpeed(6.0f * 2);
				this->SetTargetDestination(target);

				this->FindPath(this->GetTargetDestination());
				this->DrawNavigationPath();
				this->MoveAlongPath(dt);
			}
		});

	StateTransition* wanderToChase = new StateTransition(Wandering, ChasePlayer, [&](void)->bool { return  CanSeePlayer(); });
	StateTransition* chaseToWander = new StateTransition(ChasePlayer, Wandering, [&](void)->bool { return !CanSeePlayer(); });

	this->GetStateMachine()->AddState(Wandering);
	this->GetStateMachine()->AddState(ChasePlayer);

	this->GetStateMachine()->AddTransition(wanderToChase);
	this->GetStateMachine()->AddTransition(chaseToWander);
}

bool EnemyObject::FindPath(Vector3 target)
{
	pathFindingNodes = vector<Vector3>{};
	NavigationGrid grid(navigationGridFile);

	NavigationPath outPath;

	Vector3 startPos = GetTransform().GetPosition();
	startPos.y = 0;
	Vector3 endPos = target;

	bool found = grid.FindPath(startPos, endPos, outPath);
	if (!found) return false;

	Vector3 pos;
	while (outPath.PopWaypoint(pos))
	{
		pathFindingNodes.push_back(pos);
	}
	return true;
}

void EnemyObject::DrawNavigationPath()
{
	for (int i = 1; i < pathFindingNodes.size(); i++)
	{
		Vector3 a = pathFindingNodes[i - 1];
		Vector3 b = pathFindingNodes[i];
		if (i - 1 == 0)
		{
			Debug::DrawSphereLines(a, Quaternion::EulerAnglesToQuaternion(0, 0, 0), 2.5f, Vector4(1, 0, 0, 0.25f));
		}
		if (i == pathFindingNodes.size() - 1)
		{
			Debug::DrawSphereLines(b, Quaternion::EulerAnglesToQuaternion(0, 0, 0), 2.5f, Vector4(0, 1, 0, 0.25f));
		}
		Debug::DrawLine(a, b, Vector4(0, 0, 1, 0.25f));
	}
}

void EnemyObject::MoveAlongPath(float dt)
{
	Vector3 direction = (this->GetNextPathNode() - this->GetTransform().GetPosition()).Normalised();
	this->GetPhysicsObject()->AddForce(direction * this->GetMoveSpeed() * dt);
}

bool EnemyObject::CanSeePlayer()
{
	if (playerObject)
	{
		Ray ray(this->GetTransform().GetPosition(), (playerObject->GetTransform().GetPosition() - this->GetTransform().GetPosition()).Normalised());

		RayCollision closestCollision;
		std::vector<int> ignoreList = { LAYER_ENEMY };
		if (gameWorld->Raycast(ray, closestCollision, true, nullptr, ignoreList))
		{
			GameObject* selectionObject = (GameObject*)closestCollision.node;
			//Debug::DrawLine(this->GetTransform().GetPosition(), playerObject->GetTransform().GetPosition(), Vector4(1, 0, 0, 0.1f));
			Debug::DrawLine(this->GetTransform().GetPosition(), closestCollision.collidedAt, Vector4(0, 1, 0, 0.1f));
			if (selectionObject->GetBoundingVolume()->collisionLayer == LAYER_PLAYER)
			{
				std::cout << "can\n";
				return true;
			}
		}
	}
	std::cout << "can't\n";
	return false;
}