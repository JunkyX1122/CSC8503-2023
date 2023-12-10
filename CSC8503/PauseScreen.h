#pragma once
#include "PushdownState.h"
#include "Window.h"

using namespace NCL;
using namespace CSC8503;

class PauseScreen : public PushdownState
{
	PushdownResult OnUpdate(float dt, PushdownState** newState) override
	{
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::P))
		{
			return PushdownResult::Pop;
		}
		Debug::Print("PAUSE MENU", Vector2(5, 85), Vector4(1, 1, 1, 1));

		//gameRef->UpdateOuter(dt);
		return PushdownResult::NoChange;
	}
	void OnAwake() override
	{
		std::cout << "Press U to unpause game!\n";
	}
public:
	PauseScreen(CourseworkGame* g)
	{
		gameRef = g;
	}

protected:
	CourseworkGame* gameRef;
};