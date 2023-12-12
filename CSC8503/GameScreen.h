#pragma once
#include "PushdownState.h"
#include "Window.h"

using namespace NCL;
using namespace CSC8503;

class GameScreen : public PushdownState
{
	PushdownResult OnUpdate(float dt, PushdownState** newState) override
	{
		if (!gameRef->connected)
		{
			gameRef->EraseWorld();
			gameRef->DisconnectAsClient();
			passedTag = "Failed Connection";
			return PushdownResult::Pop;
		}
		if (exitFromPause)
		{
			if (isServer)
			{
				gameRef->DisconnectAsServer();
			}
			else
			{
				gameRef->DisconnectAsClient();
			}
			return PushdownResult::Pop;
		}
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::P))
		{
			*newState = new PauseScreen(gameRef);
			return PushdownResult::Push;
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
		if (passedTag == "Exit")
		{
			exitFromPause = true;
		}
		gameRef->connected = true;
		passedTag = "Gameplay";
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
	bool exitFromPause = false;
};