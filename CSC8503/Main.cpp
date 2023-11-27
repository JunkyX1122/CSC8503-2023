#include "Window.h"

#include "Debug.h"

#include "StateMachine.h"
#include "StateTransition.h"
#include "State.h"

#include "GameServer.h"
#include "GameClient.h"

#include "NavigationGrid.h"
#include "NavigationMesh.h"

#include "TutorialGame.h"
#include "NetworkedGame.h"

#include "PushdownMachine.h"

#include "PushdownState.h"

#include "PauseScreen.h"
#include "GameScreen.h"
#include "IntroScreen.h"

#include "BehaviourNode.h"
#include "BehaviourSelector.h"
#include "BehaviourSequence.h"
#include "BehaviourAction.h"

using namespace NCL;
using namespace CSC8503;

#include <chrono>
#include <thread>
#include <sstream>

void TestStateMachine()
{
	StateMachine* testMachine = new StateMachine();
	int data = 0;
	State* A = new State([&](float dt)->void
		{
			std::cout << "STATE LETTER A!!!!!!!!!!!!!!!!!!!!!!\n";
		});
	State* B = new State([&](float dt)->void
		{
			std::cout << "STATE LETTER B!!!!!!!!!!!!!!!!!!!!!!\n";
		});

	StateTransition* stateAB = new StateTransition(A, B, [&](void)->bool { return data > 10; });
	StateTransition* stateBA = new StateTransition(B, A, [&](void)->bool { return data < 0; });

	testMachine->AddState(A);
	testMachine->AddState(B);

	testMachine->AddTransition(stateAB);
	testMachine->AddTransition(stateBA);
	for (int i = 0; i < 100; i++)
	{
		testMachine->Update(1.0f);
	}
}
vector<Vector3> testNodes;

void TestPathfinding() 
{
	NavigationGrid grid("TestGrid1.txt");

	NavigationPath outPath;

	Vector3 startPos(80, 0, 10);
	Vector3 endPos(80, 0, 80);

	bool found = grid.FindPath(startPos, endPos, outPath);

	Vector3 pos;
	while (outPath.PopWaypoint(pos))
	{
		testNodes.push_back(pos);
	}
}

void DisplayPathfinding() 
{
	for (int i = 1; i < testNodes.size(); i++)
	{
		Vector3 a = testNodes[i - 1];
		Vector3 b = testNodes[i];
		Debug::DrawLine(a, b, Vector4(0, 1, 0, 1));
	}
}

void TestBehaviourTree()
{
	float behaviourTimer;
	float distanceToTarget;
	BehaviourAction* findKey = new BehaviourAction("Find Key", [&](float dt, BehaviourState state)->BehaviourState
		{
			if (state == Initialise)
			{
				std::cout << "Looking for a key!\n";
				behaviourTimer = rand() % 100;
				state = Ongoing;
			}
			else if (state == Ongoing)
			{
				behaviourTimer -= dt;
				if (behaviourTimer <= 0.0f)
				{
					std::cout << "Found a key!\n";
					return Success;
				}
			}
			return state;
		}
	);

	BehaviourAction* goToRoom = new BehaviourAction("Go To Room", [&](float dt, BehaviourState state)->BehaviourState
		{
			if (state == Initialise)
			{
				std::cout << "Going to the loot ruum!\n";
				state = Ongoing;
			}
			else if (state == Ongoing)
			{
				behaviourTimer -= dt;
				if (behaviourTimer <= 0.0f)
				{
					std::cout << "Reached Room!\n";
					return Success;
				}
			}
			return state;
		}
	);

	BehaviourAction* openDoor = new BehaviourAction("Open Door", [&](float dt, BehaviourState state)->BehaviourState
		{
			if (state == Initialise)
			{
				std::cout << "Opening door!\n";
				state = Success;
			}
			return state;
		}
	);

	BehaviourAction* lookForTreasure = new BehaviourAction("Look For Treasure", [&](float dt, BehaviourState state)->BehaviourState
		{
			if (state == Initialise)
			{
				std::cout << "Looking for that loot crate!\n";
				state = Ongoing;
			}
			else if (state == Ongoing)
			{
				bool found = rand() % 2;
				if (found)
				{
					std::cout << "LOOT FOUND BABY!\n";
					return Success;
				}
				std::cout << "rip no loot\n";
				return Failure;
			}
			return state;
		}
	);

	BehaviourAction* lookForItems= new BehaviourAction("Look For Items", [&](float dt, BehaviourState state)->BehaviourState
		{
			if (state == Initialise)
			{
				std::cout << "Looking for items!\n";
				state = Ongoing;
			}
			else if (state == Ongoing)
			{
				bool found = rand() % 2;
				if (found)
				{
					std::cout << "items FOUND BABY!\n";
					return Success;
				}
				std::cout << "items = false\n";
				return Failure;
			}
			return state;
		}
	);

	BehaviourSequence* sequence = new BehaviourSequence("Room Sequence");
	sequence->AddChild(findKey);
	sequence->AddChild(goToRoom);
	sequence->AddChild(openDoor);

	BehaviourSelector* selection = new BehaviourSelector("Loot Selection");
	selection->AddChild(lookForTreasure);
	selection->AddChild(lookForItems);

	BehaviourSequence* rootSequence = new BehaviourSequence("Root Sequence");
	rootSequence->AddChild(sequence);
	rootSequence->AddChild(selection);

	for (int i = 0; i < 5; i++)
	{
		rootSequence->Reset();
		behaviourTimer = 0.0f;
		distanceToTarget = rand() % 250;
		BehaviourState state = Ongoing;
		std::cout << "Adventure Time!:tm:\n";
		while (state == Ongoing)
		{
			state = rootSequence->Execute(1.0f);
		}
		if (state == Success)
		{
			std::cout << "pog\n";
		}
		else if (state == Failure)
		{
			std::cout << "cringe\n";
		}
		std::cout << "\n";
	}
	std::cout << "end :)\n";
}

void TestPushdownAutomata(Window* w)
{
	PushdownMachine machine(new IntroScreen());
	while (w->UpdateWindow()) 
	{
		float dt = w->GetTimer().GetTimeDeltaSeconds();
		if (!machine.Update(dt)) 
		{
			return;
		}
	}
}
/*

The main function should look pretty familar to you!
We make a window, and then go into a while loop that repeatedly
runs our 'game' until we press escape. Instead of making a 'renderer'
and updating it, we instead make a whole game, and repeatedly update that,
instead. 

This time, we've added some extra functionality to the window class - we can
hide or show the 

*/
int main() {
	Window*w = Window::CreateGameWindow("CSC8503 Game technology!", 1280, 720);

	if (!w->HasInitialised()) {
		return -1;
	}	
	//TestBehaviourTree();
	TestPushdownAutomata(w);

	w->ShowOSPointer(false);
	w->LockMouseToWindow(true);

	TutorialGame* g = new TutorialGame();
	TestPathfinding();
	


	w->GetTimer().GetTimeDeltaSeconds(); //Clear the timer so we don't get a larget first dt!
	while (w->UpdateWindow() && !Window::GetKeyboard()->KeyDown(KeyCodes::ESCAPE)) {
		float dt = w->GetTimer().GetTimeDeltaSeconds();
		DisplayPathfinding();
		if (dt > 0.1f) {
			std::cout << "Skipping large time delta" << std::endl;
			continue; //must have hit a breakpoint or something to have a 1 second frame time!
		}
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::PRIOR)) {
			w->ShowConsole(true);
		}
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::NEXT)) {
			w->ShowConsole(false);
		}

		if (Window::GetKeyboard()->KeyPressed(KeyCodes::T)) {
			w->SetWindowPosition(0, 0);
		}

		w->SetTitle("Gametech frame time:" + std::to_string(1000.0f * dt));

		g->UpdateGame(dt);
	}
	Window::DestroyGameWindow();
}