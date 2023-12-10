#pragma once
#include "PushdownState.h"
#include "Window.h"

using namespace NCL;
using namespace CSC8503;

class GameScreen : public PushdownState
{
	PushdownResult OnUpdate(float dt, PushdownState** newState) override
	{
		
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::P))
		{
			*newState = new PauseScreen(gameRef);
			return PushdownResult::Push;
		}
		if (Window::GetKeyboard()->KeyDown(KeyCodes::F1))
		{
			std::cout << "Returning to main menu!\n";
			return PushdownResult::Pop;
		}
		if (isServer)
		{
			gameRef->UpdateAsServer(dt);
		}
		else
		{ 
			gameRef->UpdateAsClient(dt);
		}
		return PushdownResult::NoChange;
	};
	void OnAwake() override
	{
		
		std::cout << "Preparing to mine bitcoin!\n";
	}
public:
	GameScreen(CourseworkGame* g, bool b)
	{
		gameRef = g;
		isServer = b;
	}

protected:
	int pauseBuffer;
	float pauseReminder = 1.0f;
	CourseworkGame* gameRef;
	bool isServer;
};