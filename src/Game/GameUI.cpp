#include "GameUI.h"

bool GameUI::m_isOption = false;
bool GameUI::m_isOptionsControls = false;
bool GameUI::m_isOptionsSound = false;

int GameUI::newKey = 0;

static std::pair<int, int> m_WindowSize;

//////////////////////////////////////////////////////////////////////////
//Main menu
void GameUI::buttonMainMenu()
{
	if (UIRenderer::Button(static_cast<int>(m_WindowSize.first * 0.15), static_cast<int>(m_WindowSize.second * 0.56),
	                       300, 200))
	{
		GameLogic::setNbNPC(0);
		SceneManager::switchToScene("Game");
	}

	if (UIRenderer::Button(static_cast<int>(m_WindowSize.first * 0.4), static_cast<int>(m_WindowSize.second * 0.56),
	                       300, 200))
	{
		GameLogic::setNbNPC(8);
		SceneManager::switchToScene("Game");
	}

	if (UIRenderer::Button(static_cast<int>(m_WindowSize.first * 0.65), static_cast<int>(m_WindowSize.second * 0.56),
	                       300, 200))
	{
		GameLogic::setNbNPC(49);
		SceneManager::switchToScene("Game");
	}

	if (UIRenderer::Button(0, 0, 300, 200))
	{
		m_isOption = true;
	}
}

void GameUI::textMainMenu()
{
	Renderer::writeTextOnScreen("Options", (-m_WindowSize.first / 2) + 55, (m_WindowSize.second / 2) - 140, 1);

	Renderer::writeTextOnScreen("Time Attack", -420, -185, 1);
	Renderer::writeTextOnScreen("Classic", -60, -185, 1);
	Renderer::writeTextOnScreen("Chaos", 265, -185, 1);
}

//////////////////////////////////////////////////////////////////////////
//In game
void GameUI::buttonPauseInGame(Camera* currentCamera, GameLogic& game_logic, Terrain& terrain)
{
	//Bouton Resume
	if (UIRenderer::Button((m_WindowSize.first / 2) - 100, 0, 200, 150))
	{
		game_logic.setPause(false);
	}

	//Bouton Restart
	if (UIRenderer::Button((m_WindowSize.first / 2) - 100, 125, 200, 150))
	{
		game_logic.getPlayer().setPlayable(true);
		game_logic.restartGame(currentCamera);
		game_logic.MoveAllToStartCheckpoint(terrain);
		game_logic.setPause(false);
	}

	//Bouton Options
	if (UIRenderer::Button((m_WindowSize.first / 2) - 100, m_WindowSize.second - 275, 200, 150))
	{
		m_isOption = true;
	}

	//Bouton Quit
	if (UIRenderer::Button((m_WindowSize.first / 2) - 100, m_WindowSize.second - 150, 200, 150))
	{
		SceneManager::switchToScene("MainMenu");
	}
}

void GameUI::textInGame(Cloporte& player, const float& elapsedTime)
{
	std::stringstream speedText{}, timeText{};
	speedText << "Fasts/h " << static_cast<int>((player.getObject().getLinearVelocityMag() / player.getMaxVelocity()) *
		100);
	timeText << "Time : " << static_cast<int>(elapsedTime / 60) << "m " << static_cast<int>(elapsedTime) % 60 << "s " <<
		static_cast<int>(elapsedTime * 1000) % 1000 << "ms";
	Renderer::writeTextOnScreen(speedText.str(), (m_WindowSize.first / 2) - 300, -(m_WindowSize.second / 2) + 50, 1);
	Renderer::writeTextOnScreen(timeText.str(), -(m_WindowSize.first / 2), -(m_WindowSize.second / 2) + 50, 1);

	drawMissedCheckpoint();
	drawPassedCheckpoint();
	drawFailedFinish();
}

void GameUI::textInGamePause()
{
	Renderer::writeTextOnScreen("Resume", -60, (m_WindowSize.second / 2) - 95, 0.6f);
	Renderer::writeTextOnScreen("Restart", -60, (m_WindowSize.second / 2) - 220, 0.6f);
	Renderer::writeTextOnScreen("Pause", -60, 0, 1);
	Renderer::writeTextOnScreen("Options", -60, -(m_WindowSize.second / 2) + 180, 0.6f);
	Renderer::writeTextOnScreen("Quit", -60, -(m_WindowSize.second / 2) + 55, 0.6f);
}

void GameUI::drawResult(GameLogic& game_logic)
{
	
	size_t nbToDisplay = game_logic.getResult().size();
	size_t nbCol, nbRows; 
	float scale;

	if (nbToDisplay <= 19)
	{
		nbCol = nbToDisplay / 10 + 1;
		nbRows = min(10, nbToDisplay);
		scale = 1.f;
	}
	else
	{
		nbCol = 3;
		nbRows = nbToDisplay / 3 + (nbToDisplay % 3 == 0 ? 0 : 1);
		scale = 7.8f / max(static_cast<float>(nbRows), 10);
	}


	scale *= min(static_cast<float> (m_WindowSize.first) / 1424.f,
	             static_cast<float> (m_WindowSize.second) / 714.f);

	//std::cout << "Result Draw : nbCol : " << nbCol << " nbRows : " << nbRows << " scale : " << scale << std::endl;
	//std::cout << "Result Draw : nbToDisplay : " << nbToDisplay << std::endl;
	float espacement = static_cast<float>(m_WindowSize.second) / static_cast<float>(nbRows + 1);

	for (const auto& [id, time] : game_logic.getResult())
	{
		std::stringstream ss;
		ss << "#" << time.first << " ";
		if (id == 0)
		{
			ss << "Player";
		}
		else
		{
			ss << game_logic.getNPC()[id - 1].getName();
		}
		ss << " : " << static_cast<int>(time.second / 60) << "m " << static_cast<int>(time.second) % 60 << "s " <<
			static_cast<int>(time.second * 1000) % 1000 << "ms";

		float posX{}, posY{};

		switch (nbCol)
		{
		case 1:
			// CENTRER
			posX = -static_cast<int>(ss.str().size()) * scale * 10;
			break;

		case 2:
			posX = -static_cast<float>(m_WindowSize.first) * 0.4f
				+ static_cast<float>(m_WindowSize.first) * 0.45f * ((time.first - 1) / nbRows);
			break;

		case 3:
			posX = -static_cast<float>(m_WindowSize.first) * 0.49f
				+ static_cast<float>(m_WindowSize.first) * 0.32f * ((time.first - 1) / nbRows);
			break;
		}


		posY = (nbRows - ((time.first - 1) % nbRows)) * espacement
			- static_cast<float>(m_WindowSize.second) * 0.525f;
		XMVECTOR color;
		switch (time.first)
		{
		case 1:
			color = XMVECTOR{{1.f, 0.843f, 0.f, 1.f}};
			break;
		case 2:
			color = XMVECTOR{{.7529f, .7529f, .7529f, 1.f}};
			break;
		case 3:
			color = XMVECTOR{{0.545f, 0.271f, .075f, 1.f}};
			break;
		default:
			color = XMVECTOR{{1.f, 1.f, 1.f, 1.f}};
			break;
		}

		Renderer::writeTextOnScreen(ss.str(), posX, posY, scale, color);
	}
}

void GameUI::drawPassedCheckpoint()
{
	if (GameLogic::isCheckpointPassed())
		Renderer::writeTextOnScreen("Checkpoint passed",
		                            -static_cast<int>(strlen("Checkpoint passed")) * 10,
		                            0.35f * static_cast<float>(m_WindowSize.second), 1,
		                            XMVECTOR{{0.f, 1.f, 0.f, 1.f}});
}

void GameUI::drawMissedCheckpoint()
{
	if (GameLogic::isCheckpointMissed())
		Renderer::writeTextOnScreen("Previous checkpoint missed",
		                            -static_cast<int>(strlen("Previous checkpoint missed")) * 10,
		                            0.35f * static_cast<float>(m_WindowSize.second), 1,
		                            XMVECTOR{{1.f, 0.f, 0.f, 1.f}});
}

void GameUI::drawFailedFinish()
{
	if(GameLogic::isFailFinish())
		Renderer::writeTextOnScreen("You need to pass all checkpoint before finish",
			-static_cast<int>(strlen("You need to pass all checkpoint before finish")) * 10,
			0.35f * static_cast<float>(m_WindowSize.second), 1,
			XMVECTOR{ {1.f, 1.f, 1.f, 1.f} });
}


//////////////////////////////////////////////////////////////////////////
//Options

void GameUI::buttonOptions()
{
	if (UIRenderer::Button(static_cast<int>(m_WindowSize.first * 0.4), 0, 300, 200))
	{
		m_isOptionsControls = true;
	}
	if (UIRenderer::Button(static_cast<int>(m_WindowSize.first * 0.4), static_cast<int>(m_WindowSize.second * 0.35),
	                       300, 200))
	{
		m_isOptionsSound = true;
	}
	if (UIRenderer::Button(0, 0, 300, 200))
	{
		m_isOption = false;
	}
}

void GameUI::buttonOptionsControls()
{
	//Quit m_isOptionsControls
	if (UIRenderer::Button(0, 0, 300, 200))
	{
		newKey = 0;
		wKbd->flushChar();
		m_isOptionsControls = false;
	}

	//Reset Key mapping
	if (UIRenderer::Button(static_cast<int>(m_WindowSize.first * 0.625), static_cast<int>(m_WindowSize.second * 0.41),
	                       200, 150))
	{
		GameInputs::resetMapping();
	}

	//Recuperate newKey
	{
		auto keyTemp = toupper(wKbd->readChar());
		if (newKey != keyTemp && keyTemp != 0)
		{
			newKey = keyTemp;
			if (GameInputs::isBound(newKey))
				newKey = 0;
		}
	}


	//Reset newKey
	if (UIRenderer::Button(static_cast<int>(m_WindowSize.first * 0.04), static_cast<int>(m_WindowSize.second * 0.56),
	                       300, 200) && newKey != 0)
	{
		newKey = 0;
		wKbd->flushChar();
	}

	//Up
	if (UIRenderer::Button(static_cast<int>(m_WindowSize.first * 0.43), 0, 250, 200) && newKey != 0)
	{
		if (GameInputs::isBound(newKey))
		{
			newKey = 0;
			wKbd->flushChar();
		}
		else
		{
			GameInputs::EditMapping("Forward", newKey);
			newKey = 0;
			wKbd->flushChar();
		}
	}

	//Down
	if (UIRenderer::Button(static_cast<int>(m_WindowSize.first * 0.78), 0, 250, 200) && newKey != 0)
	{
		if (GameInputs::isBound(newKey))
		{
			newKey = 0;
			wKbd->flushChar();
		}
		else
		{
			GameInputs::EditMapping("Backward", newKey);
			newKey = 0;
			wKbd->flushChar();
		}
	}

	//Left
	if (UIRenderer::Button(static_cast<int>(m_WindowSize.first * 0.43), static_cast<int>(m_WindowSize.second * 0.25),
	                       250, 200) && newKey != 0)
	{
		if (GameInputs::isBound(newKey))
		{
			newKey = 0;
			wKbd->flushChar();
		}
		else
		{
			GameInputs::EditMapping("Left", newKey);
			newKey = 0;
			wKbd->flushChar();
		}
	}

	//Right
	if (UIRenderer::Button(static_cast<int>(m_WindowSize.first * 0.78), static_cast<int>(m_WindowSize.second * 0.25),
	                       250, 200) && newKey != 0)
	{
		if (GameInputs::isBound(newKey))
		{
			newKey = 0;
			wKbd->flushChar();
		}
		else
		{
			GameInputs::EditMapping("Right", newKey);
			newKey = 0;
			wKbd->flushChar();
		}
	}

	//POV
	if (UIRenderer::Button(static_cast<int>(m_WindowSize.first * 0.43), static_cast<int>(m_WindowSize.second * 0.49),
	                       250, 200) && newKey != 0)
	{
		if (GameInputs::isBound(newKey))
		{
			newKey = 0;
			wKbd->flushChar();
		}
		else
		{
			GameInputs::EditMapping("POV", newKey);
			newKey = 0;
			wKbd->flushChar();
		}
	}

	//Pause
	if (UIRenderer::Button(static_cast<int>(m_WindowSize.first * 0.78), static_cast<int>(m_WindowSize.second * 0.49),
	                       250, 200) && newKey != 0)
	{
		if (GameInputs::isBound(newKey))
		{
			newKey = 0;
			wKbd->flushChar();
		}
		else
		{
			GameInputs::EditMapping("Pause", newKey);
			newKey = 0;
			wKbd->flushChar();
		}
	}

	//Restart
	if (UIRenderer::Button(static_cast<int>(m_WindowSize.first * 0.43), static_cast<int>(m_WindowSize.second * 0.74),
	                       250, 200) && newKey != 0)
	{
		if (GameInputs::isBound(newKey))
		{
			newKey = 0;
			wKbd->flushChar();
		}
		else
		{
			GameInputs::EditMapping("Reset", newKey);
			newKey = 0;
			wKbd->flushChar();
		}
	}

	//Respawn
	if (UIRenderer::Button(static_cast<int>(m_WindowSize.first * 0.78), static_cast<int>(m_WindowSize.second * 0.74),
	                       250, 200) && newKey != 0)
	{
		if (GameInputs::isBound(newKey))
		{
			newKey = 0;
			wKbd->flushChar();
		}
		else
		{
			GameInputs::EditMapping("Respawn", newKey);
			newKey = 0;
			wKbd->flushChar();
		}
	}
}

void GameUI::buttonOptionsSound()
{
	if (UIRenderer::Button(0, 0, 300, 200))
	{
		m_isOptionsSound = false;
	}

	if (UIRenderer::Button(static_cast<int>(m_WindowSize.first * 0.4), static_cast<int>(m_WindowSize.second * 0.17),
	                       100, 150))
	{
		Sound::setSoundVolumeBGM(Sound::getGain(SoundType::Background) - 0.1f);
	}

	if (UIRenderer::Button(static_cast<int>(m_WindowSize.first * 0.61), static_cast<int>(m_WindowSize.second * 0.17),
	                       100, 150))
	{
		Sound::setSoundVolumeBGM(Sound::getGain(SoundType::Background) + 0.1f);
	}

	if (UIRenderer::Button(static_cast<int>(m_WindowSize.first * 0.4), static_cast<int>(m_WindowSize.second * 0.53),
	                       100, 150))
	{
		Sound::setSoundVolumeEffect(Sound::getGain(SoundType::Effect) - 0.1f);
	}

	if (UIRenderer::Button(static_cast<int>(m_WindowSize.first * 0.61), static_cast<int>(m_WindowSize.second * 0.53),
	                       100, 150))
	{
		Sound::setSoundVolumeEffect(Sound::getGain(SoundType::Effect) + 0.1f);
	}
}

void GameUI::textOptionsPause()
{
	Renderer::writeTextOnScreen("Return", static_cast<int>(-m_WindowSize.first / 2.f) + 55,
	                            (m_WindowSize.second / 2.f) - 140, 1);
	Renderer::writeTextOnScreen("Controls", -100, (m_WindowSize.second / 2.f) - 140, 1);
	Renderer::writeTextOnScreen("Sounds", -100, (m_WindowSize.second / 2.f) - 380, 1);
}

void GameUI::textOptionsControls()
{
	Renderer::writeTextOnScreen("Return", (-m_WindowSize.first / 2.f) + 55, (m_WindowSize.second / 2.f) - 140, 1);

	Renderer::writeTextOnScreen("Temporary Key", (-m_WindowSize.first / 2.f) + 55, (m_WindowSize.second / 2.f) - 350,
	                            1);

	if (newKey != 0)
		Renderer::writeTextOnScreen(std::string{static_cast<char>(newKey)}, (-m_WindowSize.first / 2.f) + 150,
		                            (m_WindowSize.second / 2.f) - 425, 1);
	else
		Renderer::writeTextOnScreen("None", (-m_WindowSize.first / 2.f) + 150, (m_WindowSize.second / 2.f) - 425, 1.f);

	Renderer::writeTextOnScreen("Reset Temporary Key", (-m_WindowSize.first / 2.f) + 67,
	                            (m_WindowSize.second / 2.f) - 505, 0.6f);

	Renderer::writeTextOnScreen("Reset Keys", 175, (m_WindowSize.second / 2.f) - 385, 0.7f);

	Renderer::writeTextOnScreen(GameInputs::displayKey("Forward"), -75, (m_WindowSize.second / 2.f) - 135, 0.8f);
	Renderer::writeTextOnScreen(GameInputs::displayKey("Backward"), 365, (m_WindowSize.second / 2.f) - 135, 0.8f);
	Renderer::writeTextOnScreen(GameInputs::displayKey("Left"), -75, (m_WindowSize.second / 2.f) - 305, 0.8f);
	Renderer::writeTextOnScreen(GameInputs::displayKey("Right"), 365, (m_WindowSize.second / 2.f) - 305, 0.8f);

	Renderer::writeTextOnScreen(GameInputs::displayKey("POV"), -75, (m_WindowSize.second / 2.f) - 470, 0.8f);
	Renderer::writeTextOnScreen(GameInputs::displayKey("Pause"), 365, (m_WindowSize.second / 2.f) - 470, 0.8f);
	Renderer::writeTextOnScreen(GameInputs::displayKey("Reset"), -75, (m_WindowSize.second / 2.f) - 639, 0.8f);
	Renderer::writeTextOnScreen(GameInputs::displayKey("Respawn"), 365, (m_WindowSize.second / 2.f) - 639, 0.8f);
}

void GameUI::textOptionsSound()
{
	Renderer::writeTextOnScreen("Return", (-m_WindowSize.first / 2.f) + 55, (m_WindowSize.second / 2.f) - 140, 1);

	Renderer::writeTextOnScreen("Music", -15, (m_WindowSize.second / 2.f) - 140, 1);
	Renderer::writeTextOnScreen(std::to_string(static_cast<int>(Sound::getGain(SoundType::Background) * 10)),
	                            35, (m_WindowSize.second / 2.f) - 230, 1);
	Renderer::writeTextOnScreen("-", -95, (m_WindowSize.second / 2.f) - 215, 1);
	Renderer::writeTextOnScreen("+", 173, (m_WindowSize.second / 2.f) - 220, 1);

	Renderer::writeTextOnScreen("Sound Effects", -95, -35, 1);
	Renderer::writeTextOnScreen(std::to_string(static_cast<int>(Sound::getGain(SoundType::Effect) * 10)), 35,
	                            -125, 1);
	Renderer::writeTextOnScreen("-", -95, -120, 1);
	Renderer::writeTextOnScreen("+", 173, -125, 1);
}


//////////////////////////////////////////////////////////////////////////

void GameUI::mainMenu()
{
	static bool firstCall = true;

	if (firstCall)
	{
		m_WindowSize = WindowsEngine::getInstance().getGraphics().getWinSize();
		firstCall = false;
	}

	if (m_isOption && !m_isOptionsControls && !m_isOptionsSound)
	{
		buttonOptions();
	}
	else if (m_isOption && m_isOptionsControls)
	{
		buttonOptionsControls();
	}
	else if (m_isOption && m_isOptionsSound)
	{
		buttonOptionsSound();
	}
	else
	{
		buttonMainMenu();
	}

	wMouse->clearPress();

	UIRenderer::renderUI();

	if (m_isOption && !m_isOptionsControls && !m_isOptionsSound)
	{
		textOptionsPause();
	}
	else if (m_isOption && m_isOptionsControls)
	{
		textOptionsControls();
	}
	else if (m_isOption && m_isOptionsSound)
	{
		textOptionsSound();
	}
	else
	{
		textMainMenu();
	}

	Renderer::renderText();
}

void GameUI::inGame(GameLogic& game_logic, Camera* currentCamera, Terrain& terrain)
{
	if (game_logic.isPause() && !m_isOption)
	{
		buttonPauseInGame(currentCamera, game_logic, terrain);
	}
	else
	{
		if (m_isOption && !m_isOptionsControls && !m_isOptionsSound)
		{
			buttonOptions();
		}
		else if (m_isOption && m_isOptionsControls)
		{
			buttonOptionsControls();
		}
		else if (m_isOption && m_isOptionsSound)
		{
			buttonOptionsSound();
		}
	}

	UIRenderer::renderUI();

	if (game_logic.isPause() && !m_isOption)
	{
		textInGamePause();
	}
	else
	{
		if (m_isOption && !m_isOptionsControls && !m_isOptionsSound)
		{
			textOptionsPause();
		}
		else if (m_isOption && m_isOptionsControls)
		{
			textOptionsControls();
		}
		else if (m_isOption && m_isOptionsSound)
		{
			textOptionsSound();
		}
		else
		{
			float scaletext = min(static_cast<float>(m_WindowSize.first) / 1424.f,
			                      static_cast<float>(m_WindowSize.first) / 714.f);

			if (game_logic.isStarting() || game_logic.getDelayTime() <= 0.f && game_logic.getDelayTime() > -1.f)
			{
				std::stringstream delayStartstr{};
				XMVECTOR color{};
				if (game_logic.getDelayTime() > 0.f)
				{
					delayStartstr << static_cast<int>(game_logic.getDelayTime()) + 1;
					/*switch (static_cast<int>(game_logic.getDelayTime()) + 1) {
					case 3 :
						color = DirectX::XMVECTOR{ {1.f, 0.f, 0.f, 1.f} };
						break;
					case 2 :
						color = DirectX::XMVECTOR{ {1.f, 1.f, 0.f, 1.f} };
						break;
					case 1 :
						color = DirectX::XMVECTOR{ {1.f, 0.f, 0.f, 1.f} };
						break;
					}*/
					color = XMVECTOR{
						{
							(game_logic.getDelayTime() / 3.f),
							1.f - (game_logic.getDelayTime() / 3.f), 0.f, 1.f
						}
					};
				}
				else
				{
					delayStartstr << "GO !";
					color = XMVECTOR{{0.f, 1.f, 0.f, 1.f}};
				}

				scaletext += (1.f / (game_logic.getDelayTime() + 1.f));
				Renderer::writeTextOnScreen(delayStartstr.str(),
				                            -static_cast<int>(delayStartstr.str().size()) * scaletext * 10, 0,
				                            scaletext,
				                            color);
			}
			else if (game_logic.getDelayTime() > 0.f)
			{
				std::stringstream orderStart{"Press forward for start"};
				Renderer::writeTextOnScreen(orderStart.str(),
				                            -static_cast<int>(orderStart.str().size()) * scaletext * 10, 0, scaletext);
			}
			else if (game_logic.isGameOver() && !game_logic.isPause())
			{
				drawResult(game_logic);
			}
			else if (game_logic.isGameOver() && game_logic.isPause() && !m_isOption)
			{
				textInGamePause();
			}
			else if (!game_logic.isGameOver())
			{
				textInGame(game_logic.getPlayer(), game_logic.getElapsedTime());
			}
		}
	}

	Renderer::renderText();
}
