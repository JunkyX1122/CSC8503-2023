#pragma once
#include "PushdownState.h"
#include "Window.h"

using namespace NCL;
using namespace CSC8503;

class PauseScreen : public PushdownState
{
	PushdownResult OnUpdate(float dt, PushdownState** newState) override
	{
		Debug::Print("PAUSE MENU", Vector2(5, 5), Vector4(1, 1, 1, 1));
		for (int i = 0; i < options.size(); i++)
		{
			bool selected = selectedOption == i;
			Debug::Print((selected ? "> " : "") + options[i], Vector2(5, 15 + 5 * i), selected ? Vector4(0, 1, 0, 1) : Vector4(0.2,0.2,0.2, 1));
		}
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::UP))
		{
			selectedOption--;
			if (selectedOption < 0) selectedOption = options.size() - 1;
		}
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::DOWN))
		{
			selectedOption = (selectedOption + 1) % options.size();
		}
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::SPACE))
		{
			switch (selectedOption)
			{
			case(0):
				passedTag = "Continue";
				return PushdownResult::Pop;
				break;
			case(1):
				passedTag = "Exit";
				return PushdownResult::Pop;
				break;
			}
		}
		

		gameRef->UpdateOuter(dt);
		return PushdownResult::NoChange;
	}
	void OnAwake() override
	{
		passedTag = "Pause";
	}
public:
	PauseScreen(CourseworkGame* g)
	{
		gameRef = g;
	}

protected:
	CourseworkGame* gameRef;
	int selectedOption = 0;

	std::vector<std::string> options =
	{
		"Continue",
		"Exit"
	};
};