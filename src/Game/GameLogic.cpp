#include "GameLogic.h"

#include "../../Platform/IO/Sound.h"
#include "GameScenes/GameScene.h"

int GameLogic::m_nbNPC{0};
float GameLogic::m_time_draw_cp_passed{};
float GameLogic::m_time_draw_cp_missed{};
float GameLogic::m_time_draw_fail_finish{};
//////////////////////////////////////////////////////////////////////////

void GameLogic::initDeathBox(Terrain& terrain)
{
	const float scale = terrain.getParams().xyScale;
	const float length = terrain.getParams().height * scale;
	const float width = terrain.getParams().width * scale;
	const float height = terrain.getHeightmap().getMaxHeight() * terrain.getParams().scaleFactor;

	TriggerBox* borders[5];
	borders[0] = (new TriggerBox{
		PhysicalObject::fVec3{length / 2.f, -2.f, width / 2.f}, PhysicalObject::fVec3{length, 1.f, width}
	});
	borders[1] = (new TriggerBox{
		PhysicalObject::fVec3{0.f, height / 2.f, width / 2.f}, PhysicalObject::fVec3{1.f, 1.5f * height, width / 1.5f}
	});
	borders[2] = (new TriggerBox{
		PhysicalObject::fVec3{length / 2.f, height / 2.f, 0.f}, PhysicalObject::fVec3{length / 1.5f, 1.5f * height, 1.f}
	});
	borders[3] = (new TriggerBox{
		PhysicalObject::fVec3{length, height / 2.f, width / 2.f},
		PhysicalObject::fVec3{1.f, 1.5f * height, width / 1.5f}
	});
	borders[4] = (new TriggerBox{
		PhysicalObject::fVec3{length / 2.f, height / 2.f, width},
		PhysicalObject::fVec3{length / 1.5f, 1.5f * height, 1.f}
	});
	int i = 0;
	for (auto border : borders)
	{
		border->setName(std::string("DeathBox_") + std::to_string(i++));
		border->setTriggerCallback([&](int i) { m_isInBound.emplace_back(i); });
		m_trigger_box.push_back(border);
	}
}

void GameLogic::initBoost()
{
	const auto boost = new TriggerBox{{180.f, 40.f, 335.f}, {50, 10, 1}};
	boost->setTriggerCallback([&](int i)
	{
		m_boosted.emplace_back(i);
	});
	boost->setName(std::string("Boost_0"));
	m_trigger_box.push_back(boost);
}

void GameLogic::createFinishLine(RadialBlur* e_blur, SpeedLines* e_lines, Camera* currentCamera)
{
	// -- Set the finish line
	const auto finish = new TriggerBox{{480.f, 5.f, 580.f}, {75, 2, 75,}};
	finish->setTriggerCallback([&, e_blur, e_lines](int i)
	{
		if (m_result.contains(i)) return;
		if (i == 0 && !m_checkpoints.allCheckpointsPassed(0))
		{
			setFailFinish();
			return;
		}
		if (!m_checkpoints.allCheckpointsPassed(i)) return;

		float time = m_elapsedTime;
		m_result[i] =
			std::pair<int, float>(
				static_cast<int>(m_result.size() + 1),
				time);

		if (i == 0)
			EndGame(e_blur, e_lines, currentCamera);
		else
			m_npc[i - 1].disableUpdate();
		std::cout << "Player " << i << " has finished " << m_result[i].first << " in : "
			<< static_cast<int>(m_elapsedTime / 60) << "m "
			<< static_cast<int>(m_elapsedTime) % 60 << "s "
			<< static_cast<int>(m_elapsedTime * 1000) % 1000 << "ms \n";
	});

	finish->setName("Finish");
	m_trigger_box.push_back(finish);
}

void GameLogic::createCourbeIA()
{
	std::vector<FormatJson> checkpointInfos;
	JsonParser cp_parser{"res/json/Courbe.json"};
	cp_parser.openFile();
	cp_parser.getControleIA(checkpointInfos);

	m_trace.setControle(checkpointInfos);
}

void GameLogic::MoveToLastCheckpoint(const int& id)
{
	const XMVECTOR t = m_checkpoints.getPositionLastCP(id);
	if (id == 0)
	{
		//std::cout << "MoveToLastCheckpoint for m_player " << id << std::endl;
		m_player.setTranslation(XMVectorGetX(t), XMVectorGetY(t), XMVectorGetZ(t));
		m_player.setForward(m_checkpoints.getDirectionLastCP(0));
	}
	else
	{
		m_npc[id - 1].setPosition(XMVectorGetX(t), XMVectorGetY(t), XMVectorGetZ(t));
		//std::cout << "MoveToLastCheckpoint for " << id << std::endl;
	}
}

void GameLogic::EndGame(RadialBlur* e_blur, SpeedLines* e_lines, Camera* currentCamera)
{
	if (m_checkpoints.allCheckpointsPassed(0))
	{
		e_blur->disable();
		e_lines->disable();
		if (!m_player.currentViewIsThirdPerson()) m_player.switchView(); // re-render player
		currentCamera = &m_player.getThirdPersonCam();
		currentCamera->setPosition(XMVECTOR{{{463.f, 87.F, 505.f, 0.F}}});
		currentCamera->lookAt(m_player.getPosition());
		currentCamera->updateCam();
		m_player.lockCameras(true);
		m_player.setPlayable(false);
		m_isGameOver = true;
	}
	else
		std::cout << "You need to pass all checkpoint !" << std::endl;
}

void GameLogic::handleKeyboardInputsMenu(Camera* currentCamera, Terrain& terrain, bool isDebug)
{
	if (!m_isStarting && m_delayStart > 0.f && !isDebug)
	{
		if (wKbd->isKeyPressed(GameInputs::queryDirectionInput(Direction::FORWARD)))
		{
			m_isStarting = true;
		}
	}


	if (!wKbd->isKeyPressed(GameInputs::queryCommandInput(Command::PAUSE))) m_hasEscBeenReleased = true;

	if (wKbd->isKeyPressed(GameInputs::queryCommandInput(Command::PAUSE)) && m_hasEscBeenReleased && !m_isGameOver)
	{
		m_hasEscBeenReleased = false;
		m_isPaused = !m_isPaused;
	}

	//Si touche m appuyer on stop toutes les musiques
	if (wKbd->isKeyPressed('M'))
	{
		if ((m_elapsedTime - m_delay_mute) > 1.0f)
		{
			m_delay_mute = m_elapsedTime;
			m_mute = !m_mute;
		}
	}


	if (wKbd->isKeyPressed(GameInputs::queryCommandInput(Command::RESPAWN)))
	{
		m_player.setPlayable(true);
		if (m_isGameOver)
		{
			restartGame(currentCamera);
			MoveAllToStartCheckpoint(terrain);
		}
		else
			MoveToLastCheckpoint(0);
	}

	if (wKbd->isKeyPressed(GameInputs::queryCommandInput(Command::RESET)))
	{
		m_player.setPlayable(true);
		restartGame(currentCamera);
		MoveAllToStartCheckpoint(terrain);
	}

#ifndef NDEBUG
	if (wKbd->isKeyPressed(Keyboard::letterCodeFromChar('n')))
	{
		m_player.setPlayable(true);
		m_checkpoints.validNCheckpoints(4);
		MoveToLastCheckpoint(0);
	}

	if (wKbd->isKeyPressed(GameInputs::queryCommandInput(Command::NEXT_CP)) && timercheckpointPassed == 0)
	{
		m_player.setPlayable(true);
		m_checkpoints.validNextCheckpoints();
		MoveToLastCheckpoint(0);
		timercheckpointPassed = 10;
	}
	if (wKbd->isKeyPressed(GameInputs::queryCommandInput(Command::LAST_CP)) && timercheckpointPassed == 0)
	{
		m_player.setPlayable(true);
		m_checkpoints.resetLastCheckpoint();
		MoveToLastCheckpoint(0);
		timercheckpointPassed = 10;
	}
	if (timercheckpointPassed > 0) timercheckpointPassed--;
#endif // DEBUG
}

void GameLogic::importObjects(Scene* scene)
{
	std::vector<FormatJson> checkpointInfos;
	JsonParser cp_parser{"res/json/Checkpoints.json"};
	cp_parser.openFile();
	cp_parser.getCheckpoints(checkpointInfos);

	m_checkpoints.addCheckpointFromJson(checkpointInfos);

	for (const FormatJson& obj : m_objs)
	{
		MeshManager::loadMeshFromFile(obj.nameObj, obj.pathObj);
		MeshManager::loadToCurrentScene(scene, obj.nameObj);

		// This is assuming the mesh is used once ...
		auto m = MeshManager::getMeshReference(obj.nameObj);
		m->getTransform().setPosition(obj.positionObj);
		m->getTransform().setRotation(obj.forwardObj);
		m->getTransform().setScale(obj.scaleObj);

		// wtf is this if else
		if (static_cast<int>(XMVectorGetW(obj.scaleShape)) == 3)
		{
			// wtf is scaleShape ???

			// why does a trigger box need a mesh ???
			auto t = new TriggerBox(
				PhysicalObject::fVec3(XMVectorGetX(obj.positionObj), // 
				                      XMVectorGetY(obj.positionObj), // 
				                      XMVectorGetZ(obj.positionObj)),
				//	how about a fVec3 implicit constructor with XMVECTOR
				PhysicalObject::fVec3(XMVectorGetX(obj.scaleShape), // 
				                      XMVectorGetY(obj.scaleShape), // 
				                      XMVectorGetZ(obj.scaleShape))); // 

			t->setTriggerCallback([&](int i) { m_isInBound.push_back(i); });
			m_trigger_box.push_back(t);
		}

		else
		{
			m_staticObject.emplace_back(StaticObject{});

			StaticObject& s = m_staticObject.back();

			switch (static_cast<int>(XMVectorGetW(obj.scaleShape)))
			{
			case 0:
				s.addShape(physx_shape::getCube(PhysicsEngine::fVec3(XMVectorGetX(obj.scaleShape),
				                                                     XMVectorGetY(obj.scaleShape),
				                                                     XMVectorGetZ(obj.scaleShape))));
				break;
			case 1:
				s.addShape(physx_shape::getBall(XMVectorGetX(obj.scaleShape)));
				break;
			case 2:
				s.addShape(physx_shape::getCapsule(XMVectorGetX(obj.scaleShape), XMVectorGetY(obj.scaleShape)));
				break;
			case 4:
				s.addShape(physx_shape::getTriangleMesh(m.get(),
					PhysicalObject::fVec3(XMVectorGetX(obj.scaleObj),
						XMVectorGetY(obj.scaleObj),
						XMVectorGetZ(obj.scaleObj))));
				break;
			}

			s.setMesh(m.get());

			s.setName(obj.nameObj);
		}
	}
}

void GameLogic::MoveAllToStartCheckpoint(Terrain& terrain)
{
	const size_t nbNPC = m_npc.size();
	const float espacament = (25.f) / min(nbNPC, 10);
	//int nbLigne = nbNPC+1 / 10;
	m_checkpoints.resetAllCheckpoints();

	const XMVECTOR cpStart = m_checkpoints.getPositionLastCP(0);

	m_player.setForward(m_checkpoints.getDirectionLastCP(0));
	float z = XMVectorGetZ(cpStart);
	const XMVECTOR pos = XMVectorSet(525.f, 60.f, z, 0);
	m_player.setTranslation(
		525.f,
		terrain.getWorldHeightAt(pos) + 1.5f,
		z
	);

	for (int i = 0; i < nbNPC; i++)
	{
		auto& ia = m_npc[i];

		float x = 525.f + espacament * static_cast<float>((i + 1) % 10);
		z = XMVectorGetZ(cpStart) + 2.f * static_cast<float>((i + 1) / 10);
		XMVECTOR pos = XMVectorSet(x, 60.f, z, 0);
		ia.setPosition(x, terrain.getWorldHeightAt(pos) + 1.5f, z);
	}
}

void GameLogic::restartGame(Camera* currentCamera)
{
	m_isGameOver = false;
	m_checkpoints.resetAllCheckpoints();
	if (currentCamera) currentCamera->updateCam();
	m_player.lockCameras(false);
	std::ranges::for_each(m_npc, [&](NPC& ia)
	{
		ia.enableUpdate();
	});
	m_elapsedTime = 0;
	m_result.clear();
	m_delayStart = 3.f;
	m_isStarting = false;

	if(!m_mute)
		Sound::stopAllSounds();

	m_decompt = false;
	m_endSound = false;
}

void GameLogic::updatePositionPlayer(const float deltaTime, const Terrain& terrain)
{
	m_player.setPlayable(m_delayStart <= 0.f);

	if (!m_isInBound.empty())
	{
		for (const auto& id : m_isInBound)
		{
			MoveToLastCheckpoint(id);
		}
	}

	if (!m_boosted.empty())
	{
		for (const auto& id : m_boosted)
		{
			if (id == 0)
				m_player.addBoost(boostValue);
			else
				m_npc[id - 1].addBoost(boostValue);
		}
	}

	m_isInBound.clear();
	m_boosted.clear();


	// Get the ground normal if we are close to the ground (used for basic camera mouvement and ground checks, this will move)
	const XMVECTOR groundNormal = (XMVectorGetY(m_player.getPosition()) - terrain.
		                              getWorldHeightAt(m_player.getPosition()) < 5.f)
		                              ? terrain.getNormalAt(m_player.getPosition())
		                              : XMVECTOR{{0.F, 1.f, 0.F, 0.f}};
	m_player.setGroundVector(groundNormal);

	m_player.update(deltaTime);
	if (m_delayStart <= 0.f)
		for (auto& ia : m_npc)
		{
			ia.update();
		}
}

void GameLogic::createNPC()
{
	for (int i = 1; i < m_nbNPC + 1; i++)
	{
		m_checkpoints.addNewPlayer(i);

		NPC ia{};
		ia.setTrace(m_trace);

		m_npc.push_back(std::move(ia));
	}
}

void GameLogic::startTimer(const float deltaTime)
{
	if (m_time_draw_cp_missed > 0.f)
		m_time_draw_cp_missed -= deltaTime;
	if (m_time_draw_cp_passed > 0.f)
		m_time_draw_cp_passed -= deltaTime;
	if (m_time_draw_fail_finish > 0.f)
		m_time_draw_fail_finish -= deltaTime;

	PhysicsEngine::setRunningState(m_delayStart <= 0.f);
	if (m_delayStart > -1.f && m_delayStart <= 1.f || m_isStarting)
	{
		if (!m_decompt)
		{
			Sound::playSound(SoundType::Start);
			m_decompt = true;
		}
		m_delayStart -= deltaTime;
	}
	if (m_delayStart <= 0.f && m_isStarting)
	{
		m_isStarting = false;
		
	}
}

void GameLogic::init(Scene* scene, RadialBlur* e_blur, SpeedLines* e_lines, Camera* currentCamera, Terrain& terrain)
{
	NPC::resetId();
	m_time_draw_cp_passed = -.5f;
	m_time_draw_cp_missed = -.5f;
	m_endCamera.setProjection<PerspectiveProjection>({});
	m_endCamera.setPosition(XMVECTOR{{463.f, 87.F, 505.f, 0.F}});


	importObjects(scene);

	createFinishLine(e_blur, e_lines, currentCamera);
	initDeathBox(terrain);
	initBoost();

	m_checkpoints.addNewPlayer(0);
	if (m_nbNPC > 0)
	{
		createCourbeIA();
		createNPC();
	}
}

void GameLogic::changeColorPassedCheckpointTree(
	DeferredRenderer& m_renderer)
{
	const bool isPassed = m_checkpoints.isCheckpointPassed(0, 4);
	auto& SL = m_renderer.getLightManager().getAllSpotLights();
	std::ranges::for_each(SL, [&](auto& s)
	{
		if (std::abs(DirectX::XMVectorGetX(s.position) - 160.f) < .01f
			&& std::abs(DirectX::XMVectorGetY(s.position) - 100.f) < .01f
			&& std::abs(DirectX::XMVectorGetZ(s.position) - 379.f) < .01f)
		{
			s.diffuse = DirectX::XMVectorSetX(s.diffuse, isPassed ? 0.f : 1.f);
			s.diffuse = DirectX::XMVectorSetY(s.diffuse, isPassed ? 1.f : 0.f);
		}
		else if (std::abs(DirectX::XMVectorGetX(s.position) - 148.f) < .01f
			&& std::abs(DirectX::XMVectorGetY(s.position) - 100.f) < .01f
			&& std::abs(DirectX::XMVectorGetZ(s.position) - 478.f) < .01f)
		{
			s.diffuse = DirectX::XMVectorSetX(s.diffuse, isPassed ? 1.f : 0.f);
			s.diffuse = DirectX::XMVectorSetY(s.diffuse, isPassed ? 0.f : 1.f);
		}
	});
}

void GameLogic::SoundUpdate()
{
	if (m_mute)
	{
		Sound::stopAllSounds();
	}
	else
	{
		if (m_elapsedTime > 0 && !Sound::isPlaying(SoundType::Background) && !m_mute && !m_isGameOver)
		{
			Sound::playSoundLoop(SoundType::Background);
		}

		if (m_isPaused && (!Sound::isPaused(SoundType::Background) || !Sound::isPaused(SoundType::Start)))
		{
			Sound::pauseAllSounds();
		}

		if (!m_isPaused && (Sound::isPaused(SoundType::Background) || Sound::isPaused(SoundType::Start)))
		{
			Sound::resumeAllSounds();
		}

		if (m_isGameOver && !m_endSound)
		{
			Sound::stopAllSounds();
			Sound::playSound(SoundType::End);
			m_endSound = true;

		}
	}
}

void GameLogic::backMainMenu()
{
	if (isGameOver() && wKbd->isKeyPressed(VK_ESCAPE))
	{
		SceneManager::switchToScene("MainMenu");
	}
}
