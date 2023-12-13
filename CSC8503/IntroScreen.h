#pragma once
#include "PushdownState.h"
#include "Window.h"

using namespace NCL;
using namespace CSC8503;

class IntroScreen : public PushdownState
{
	
	PushdownResult OnUpdate(float dt, PushdownState** newState) override
	{
 		Debug::Print("SOMETHING ABOUT CSC8503 COURSEWORK", Vector2(5, 5), Vector4(1, 1, 1, 1));
		for (int i = 0; i < options.size(); i++)
		{
			bool selected = selectedOption == i;
			Debug::Print((selected ? "> " : "") + options[i], Vector2(5, 15 + 5 * i), selected ? colors[i] : Vector4(0.2, 0.2, 0.2, 1));
		}

		for (int i = 0; i < optionsLevel.size(); i++)
		{
			bool selected = selectedLevel == i;
			Debug::Print(("") + optionsLevel[i], Vector2(65 + 15 * i, 15), (selected && selectedOption == 0) ? Vector4(0, 0, 1, 0) : Vector4(0.2, 0.2, 0.2, 1));
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

		if (Window::GetKeyboard()->KeyPressed(KeyCodes::LEFT))
		{
			selectedLevel--;
			if (selectedLevel < 0) selectedLevel = optionsLevel.size() - 1;
		}
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::RIGHT))
		{
			selectedLevel = (selectedLevel + 1) % optionsLevel.size();
		}
		
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::SPACE))
		{
			switch (selectedOption)
			{
			case(0):
				passedTag = "Start Server";
				windowRef->SetWindowPosition(-1920, 0);
				gameRef->SetLevelID(selectedLevel);
				gameRef->InitialiseGameAsServer();
				*newState = new GameScreen(gameRef, true);
				return PushdownResult::Push;
				break;
			case(1):

				passedTag = "Start Client";
				windowRef->SetWindowPosition(0, 0);
				gameRef->SetLevelID(-1);
				gameRef->InitialiseGameAsClient();
				*newState = new GameScreen(gameRef, false);
				return PushdownResult::Push;
				break;
			case(3):
				return PushdownResult::Pop;
				break;
			}
		}
	
		

		gameRef->UpdateOuter(dt);
		return PushdownResult::NoChange;
		
	}
	void OnAwake() override
	{
		gameRef->EraseWorld();
		std::cout << "Welcome to game\n";
		if (passedTag == "Failed Connection")
		{
			failedToConnect = true;
			options[1] = "Join Server (Connection Failed)";
			colors[1] = Vector4(1, 0, 0, 1);
		}
		
	}
	void OnSleep() override
	{
	}

public:
	IntroScreen(CourseworkGame* g, Window* w)
	{
		gameRef = g;
		windowRef = w;
	}

protected:
	CourseworkGame* gameRef;
	Window* windowRef;
	int selectedOption = 0;
	int selectedLevel = 0;
	bool failedToConnect = false;
	std::vector<std::string> options = 
	{
		"Start Game As Server",
		"Join Server",
		"Credits or something",
		"LEAVE"
	};
	std::vector<Vector4> colors =
	{
		Vector4(0,1,0,1),
		Vector4(0,1,0,1),
		Vector4(0,1,0,1),
		Vector4(0,1,0,1)
	};
	std::vector<std::string> optionsLevel =
	{
		"Level 1",
		"Test Level"
	};
};