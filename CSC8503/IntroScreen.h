#pragma once
#include "PushdownState.h"
#include "Window.h"

using namespace NCL;
using namespace CSC8503;

class IntroScreen : public PushdownState
{
	PushdownResult OnUpdate(float dt, PushdownState** newState) override
	{
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::SPACE))
		{
			*newState = new GameScreen();
			return PushdownResult::Push;
		}
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::ESCAPE)) 
		{
			return PushdownResult::Pop;
		}
		return PushdownResult::NoChange;
	}
	void OnAwake() override
	{
		std::cout << "Welcome to game\n";
		std::cout << "Press Space!!!!!!!!!!!!!!!!!!\n";
	}
};