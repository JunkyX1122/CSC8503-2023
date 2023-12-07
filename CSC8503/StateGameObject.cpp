#include "StateGameObject.h"
#include "StateTransition.h"
#include "StateMachine.h"
#include "State.h"
#include "PhysicsObject.h"
#include "Debug.h"

using namespace NCL;
using namespace CSC8503;

StateGameObject::StateGameObject()
{
	counter = 0.0f;
	stateMachine = new StateMachine();

}

EnemyObject::EnemyObject(std::string filename)
{
	navigationGridFile = filename;

	State* Wandering = new State([&](float dt)->void
		{

		});
	State* ChasePlayer = new State([&](float dt)->void
		{
			if (playerObject)
			{
				Vector3 target = playerObject->GetTransform().GetPosition();
				target.y = 0;

				this->SetTargetDestination(target);

				this->FindPath(this->GetTargetDestination());
				this->DrawNavigationPath();

				Vector3 direction = (this->GetNextPathNode() - this->GetTransform().GetPosition()).Normalised();
				this->GetPhysicsObject()->AddForce(direction * 6.0f * dt);
			}
		});

	StateTransition* wanderToChase = new StateTransition(Wandering, ChasePlayer, [&](void)->bool { return true;  });
	StateTransition* chaseToWander = new StateTransition(ChasePlayer, Wandering, [&](void)->bool { return false; });

	this->GetStateMachine()->AddState(Wandering);
	this->GetStateMachine()->AddState(ChasePlayer);

	this->GetStateMachine()->AddTransition(wanderToChase);
	this->GetStateMachine()->AddTransition(chaseToWander);
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