#pragma once
#include "PushdownState.h"
#include "Window.h"

using namespace NCL;
using namespace CSC8503;

class IntroScreen : public PushdownState
{
	
	PushdownResult OnUpdate(float dt, PushdownState** newState) override
	{
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::NUM1))
		{	
			gameRef->InitialiseGameAsServer();
			*newState = new GameScreen(gameRef, true);
			return PushdownResult::Push;
		}
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::NUM2))
		{
			gameRef->InitialiseGameAsClient();
			*newState = new GameScreen(gameRef, false);
			return PushdownResult::Push;
		}
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::ESCAPE)) 
		{
			return PushdownResult::Pop;
		}
		Debug::Print("MAIN MENU", Vector2(5, 85), Vector4(1,1,1,1));

		gameRef->UpdateOuter(dt);
		return PushdownResult::NoChange;
		
	}
	void OnAwake() override
	{
		gameRef->EraseWorld();
		std::cout << "Welcome to game\n";
	}
	void OnSleep() override
	{
	}

public:
	IntroScreen(CourseworkGame* g)
	{
		gameRef = g;
	}

protected:
	CourseworkGame* gameRef;
};