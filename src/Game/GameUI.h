#pragma once
#include <utility>

#include "Cloporte.h"
#include "abstraction/abstraction_core.h"
#include "GameLogic.h"
#include "Renderer.h"
#include "../../Platform/IO/GameInputs.h"
#include "../../Platform/IO/Sound.h"


class GameUI
{
private:
	static bool m_isOption;
	static bool m_isOptionsControls;
	static bool m_isOptionsSound;

	static int newKey;

	//////////////////////////////////////////////////////////////////////////
	//Main menu
	static void buttonMainMenu();

	static void textMainMenu();

	//////////////////////////////////////////////////////////////////////////
	//In game
	static void buttonPauseInGame(Camera* currentCamera, GameLogic& game_logic, Terrain& terrain);

	static void textInGame(Cloporte& player, const float& elapsedTime);

	static void textInGamePause();

	static void drawResult(GameLogic& game_logic);

	static void drawPassedCheckpoint();

	static void drawMissedCheckpoint();

	static void drawFailedFinish();

	//////////////////////////////////////////////////////////////////////////
	//Options

	static void buttonOptions();

	static void buttonOptionsControls();

	static void buttonOptionsSound();

	static void textOptionsPause();

	static void textOptionsControls();

	static void textOptionsSound();

	//////////////////////////////////////////////////////////////////////////

	static bool isOptionsControls() { return m_isOption && m_isOptionsControls; }
	static bool isOptionsSound() { return m_isOption && m_isOptionsSound; }
	//////////////////////////////////////////////////////////////////////////


public:
	static void mainMenu();

	static void inGame(GameLogic& game_logic, Camera* currentCamera, Terrain& terrain);

	static bool isOption() { return m_isOption; }
};
