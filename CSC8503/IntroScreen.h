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
			Debug::Print((selected && menuSelect == 0 ? "> " : "") + options[i], Vector2(5, 15 + 5 * i), (selected && menuSelect == 0) ? colors[i] : Vector4(0.05, 0.05, 0.05, 1));
		}

		for (int i = 0; i < optionsLevel.size(); i++)
		{
			bool selected = selectedLevel == i;
			Debug::Print((selected ? "> " : "") + optionsLevel[i], Vector2(69, 15 + 5 * i),
				(selected) ? 
				(selectedOption == 0 ?
					Vector4(menuSelect == 1 ? 0.25 : 0.1, menuSelect == 1 ? 0.25 : 0.1, menuSelect == 1 ? 1 : 0.8, 0) : Vector4(0.05, 0.05, 0.8, 0)) :
				(selectedOption == 0 ? 
					Vector4(0.25, 0.25, 0.25, 1) : Vector4(0.05, 0.05, 0.05,1)));
		}
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::UP))
		{
			switch(menuSelect)
			{
				case(0):
					selectedOption--;
					if (selectedOption < 0) selectedOption = options.size() - 1;
					break;

				case(1):
					selectedLevel--;
					if (selectedLevel < 0) selectedLevel = optionsLevel.size() - 1;
					break;
			}
		}
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::DOWN))
		{
			switch (menuSelect)
			{
				case(0):
					selectedOption = (selectedOption + 1) % options.size();
					break;
					
				case(1):
					selectedLevel = (selectedLevel + 1) % optionsLevel.size();
					break;
			}
		}

		if (Window::GetKeyboard()->KeyPressed(KeyCodes::LEFT) && selectedOption == 0)
		{
			menuSelect--;
			if (menuSelect < 0) menuSelect = 1;
		}
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::RIGHT) && selectedOption == 0)
		{
			menuSelect = (menuSelect + 1) % 2;
		}
		
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::SPACE) && (menuSelect == 0 || selectedOption == 0))
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
			colors[1] = Vector4(1, 0.25, 0.25, 1);
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
	int menuSelect = 0;
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
		Vector4(0.25,1,0.25,1),
		Vector4(0.25,1,0.25,1),
		Vector4(0.25,1,0.25,1),
		Vector4(0.25,1,0.25,1)
	};
	std::vector<std::string> optionsLevel =
	{
		"Test Level 1",
		"Test Level 2",
		"Level 1"
	};
};