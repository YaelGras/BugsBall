#pragma once
#include <map>
#include <utility>

#include "CheckpointController.h"
#include "NPC.h"
#include "Cloporte.h"

#include "../../Graphics/World/WorldRendering/Terrain.h"
#include "abstraction/DeferredRenderer.h"
#include "../../Physics/World/TriggerBox.h"
#include "World/Managers/MeshManager.h"

#include "../../Platform/IO/JsonParser.h"

#include "VFXPipeline/RadialBlur.h"
#include "VFXPipeline/SpeedLines.h"
#include "../../Physics/World/PhysicalHeightMap.h"

//class GameScene;

class GameLogic
{
private:
	static int m_nbNPC;
	static float m_time_draw_cp_passed;
	static float m_time_draw_cp_missed;
	static float m_time_draw_fail_finish;
	const float boostValue = 1.4f;

	std::vector<TriggerBox*> m_trigger_box;
	std::vector<int> m_isInBound;
	std::vector<int> m_boosted;

	std::map<int, std::pair<int, float>> m_result;

	Trace m_trace;

	std::vector<NPC> m_npc;

	CheckpointController m_checkpoints;

	JsonParser m_parser{ "res/json/Position.json" };
	std::vector<FormatJson> m_objs = m_parser.getObjs();
	std::vector<StaticObject> m_staticObject;

	Cloporte m_player;
	Camera m_endCamera;

	// Delay start
	float m_delayStart = 3.0f; 
	bool m_isStarting = false; 

	bool m_isGameOver = false; 
	float m_elapsedTime = 0; 

	//Pause
	// sadly i have not done inputs that well so WM_CHARDOWN is not correct
	bool m_isPaused = false; 
	bool m_hasEscBeenReleased = false;

	bool m_decompt = false;

	bool m_mute = false;
	float m_delay_mute = 0.f;

	bool m_endSound = false;

	bool m_inGame = true;


	void initDeathBox(Terrain& terrain);

	void initBoost();

	void createFinishLine(RadialBlur* e_blur, SpeedLines* e_lines, Camera* currentCamera);

	void createCourbeIA();

	void MoveToLastCheckpoint(const int& id);

	void EndGame(RadialBlur* e_blur, SpeedLines* e_lines, Camera* currentCamera);
public:
	GameLogic() = default;

	~GameLogic() = default;

#ifndef NDEBUG
	int timercheckpointPassed = 0;

#endif // DEBUG
	void handleKeyboardInputsMenu( Camera* currentCamera, Terrain& terrain, bool isDebug);

	void importObjects(Scene* scene);

	void MoveAllToStartCheckpoint(Terrain& terrain);

	void restartGame(Camera* currentCamera);

	void updatePositionPlayer(const float deltaTime, const Terrain& terrain);

	void createNPC();

	void startTimer(const float deltaTime);


	void init(Scene* scene, RadialBlur* e_blur, SpeedLines* e_lines, Camera* currentCamera, Terrain& terrain);

	//////////////////////////////////////////////////////////////////////////

	[[nodiscard]] const std::map<int, std::pair<int, float>>& getResult() { return m_result; }

	[[nodiscard]] const std::vector<NPC>& getNPC() { return m_npc; }

	[[nodiscard]] Cloporte& getPlayer() { return m_player; }
	[[nodiscard]] bool isPause()	const noexcept {return m_isPaused; }
	[[nodiscard]] bool isGameOver() const noexcept { return m_isGameOver; }
	[[nodiscard]] bool isStarting() const noexcept { return m_isStarting; }
	[[nodiscard]] bool isPlaying() const noexcept { return !isStarting()  	&& !(getDelayTime() > 0.f)	&& !isPause(); }
	[[nodiscard]] float getDelayTime() const noexcept { return m_delayStart; }
	[[nodiscard]] float getElapsedTime() const noexcept { return m_elapsedTime; }

	//////////////////////////////////////////////////////////////////////////

	void step(float elapsedTime) { m_elapsedTime += elapsedTime; }

	//////////////////////////////////////////////////////////////////////////

	void setPause(bool pause) { m_isPaused = pause; }
	static void setNbNPC(int nb) { m_nbNPC = nb; }

	// Pour les affichages de l'avancé des checkpoints
	static void setCheckpointPassed() {
		m_time_draw_cp_passed = 1.f; 
	}
	static void setCheckpointMissed() {
		m_time_draw_cp_missed = 1.f; 
	}
	static bool isCheckpointPassed() {
		return m_time_draw_cp_passed > 0.f; 
	}
	static bool isCheckpointMissed() {
		return m_time_draw_cp_missed > 0.f; 
	}
	static void setFailFinish() {
		m_time_draw_fail_finish = 1.f; 
	}
	static bool isFailFinish() {
		return m_time_draw_fail_finish > 0.f; 
	}

	// Pour la modification de la lumiere autour de l'arbre 
	void changeColorPassedCheckpointTree(
		DeferredRenderer& m_renderer);

	void SoundUpdate();

	bool getMute() const { return m_mute; }

	bool getInGame() const { return m_inGame; }
	void setInGame(bool inGame) { m_inGame = inGame; }

	void backMainMenu();

};
