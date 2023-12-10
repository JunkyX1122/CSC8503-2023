#include "Window.h"

#include "Debug.h"

#include "StateMachine.h"
#include "StateTransition.h"
#include "State.h"

#include "GameServer.h"
#include "GameClient.h"

#include "NavigationGrid.h"
#include "NavigationMesh.h"

#include "CourseworkGame.h"
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
#include <string>

using std::string;

class TestPacketReceiver : public PacketReceiver
{
public:
	TestPacketReceiver(string name)
	{
		this->name = name;
	}

	void ReceivePacket(int type, GamePacket* payload, int source)
	{
		if (type == String_Message)
		{
			StringPacket* realPacket = (StringPacket*)payload;

			string msg = realPacket->GetStringFromData();

			std::cout << name << " recieved message: " << msg << std::endl;
		}
	}
protected:
	string name;
};

void TestNetworking()
{
	///*
	NetworkBase::Initialise();

	TestPacketReceiver serverReceiver("Server");
	TestPacketReceiver clientReceiver("Client");

	int port = NetworkBase::GetDefaultPort();

	GameServer* server = new GameServer(port, 1);
	GameClient* client = new GameClient();

	server->RegisterPacketHandler(String_Message, &serverReceiver);
	client->RegisterPacketHandler(String_Message, &clientReceiver);

	bool canConnect = client->Connect(127, 0, 0, 1, port);

	for (int i = 0; i < 100; i++)
	{
		StringPacket p("Server says hello! " + std::to_string(i));
		server->SendGlobalPacket(p);

		p = StringPacket("Client says hello! " + std::to_string(i));
		client->SendPacket(p);

		server->UpdateServer();
		client->UpdateClient();

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	NetworkBase::Destroy();
	//*/
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
	

	TestNetworking();
	//TestBehaviourTree();
	//TestPushdownAutomata(w);
	//TestStateMachine();
	w->ShowOSPointer(false);
	w->LockMouseToWindow(true);

	CourseworkGame* g = new CourseworkGame();
	//TestPathfinding();
	PushdownMachine machine(new IntroScreen(g));

	bool game = false;
	w->GetTimer().GetTimeDeltaSeconds(); //Clear the timer so we don't get a larget first dt!
	bool endGame = false;

	while (w->UpdateWindow() && !Window::GetKeyboard()->KeyDown(KeyCodes::ESCAPE) && !endGame)
	 {
		/*
		float dt = w->GetTimer().GetTimeDeltaSeconds();
		if (game)
		{
			if (dt > 0.1f) {
				std::cout << "Skipping large time delta" << std::endl;
			}
			else
			{
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
				//g->UpdateGame(dt);
				if (!machine.Update(dt))
				{
					endGame = true;
				}
			}
		}
		*/
		
		//DisplayPathfinding();
		
		
	}
	Window::DestroyGameWindow();
}