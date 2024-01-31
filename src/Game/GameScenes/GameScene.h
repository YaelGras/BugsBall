#pragma once

#include "Scene.h"

#include <filesystem>

#include "../../Graphics/World/WorldRendering/Terrain.h"
#include "../../Graphics/abstraction/Camera.h"
#include "../../Physics/World/PhysicalHeightMap.h"
#include "abstraction/DeferredRenderer.h"
#include "World/Fireflies.h"

#include "../../Graphics/World/WorldRendering/Skybox.h"
#include "../../Graphics/World/Player.h"


#include "../Game/GameLogic.h"

class GameScene : public Scene {


private:
	GameLogic m_gameLogic;

	DeferredRenderer m_renderer;

	// -- Effect and skybox
	Skybox  m_skybox {"res/textures/garden.dds"};

	// -- Camera
	Camera* currentCamera = nullptr;

	Texture m_screenShot;
	FrameBuffer m_fbo;

	Player m_debugCam;
	bool m_isDebug = false;

	RadialBlur* e_blur;
	SpeedLines* e_lines;
	Fireflies m_flies;


	Terrain m_terrain{ "res/textures/heightmap.png" };
	PhysicalHeightMap phm;



	std::vector<Billboard> m_billboardsTrees;
	Material m_trees[4] = 
	{
		{
			{TextureType::ALBEDO,	Texture{"res/textures/trees/1.dds"}},
			{TextureType::NORMAL,	Texture{"res/textures/trees/1n.dds"}},
			{TextureType::AO,		Texture{"res/textures/trees/1ao.dds"}},
		},
		{
			{TextureType::ALBEDO,	Texture{"res/textures/trees/2.dds", false}},
			{TextureType::NORMAL,	Texture{"res/textures/trees/2n.dds", false}},
			{TextureType::AO,		Texture{"res/textures/trees/2ao.dds", false}},
		},
		{
			{TextureType::ALBEDO,	Texture{"res/textures/trees/3.dds", false}},
			{TextureType::NORMAL,	Texture{"res/textures/trees/3n.dds", false}},
			{TextureType::AO,		Texture{"res/textures/trees/3ao.dds", false}},
		},
		{
			{TextureType::ALBEDO,	Texture{"res/textures/trees/4.dds", false}},
			{TextureType::NORMAL,	Texture{"res/textures/trees/4n.dds", false}},
			{TextureType::AO,		Texture{"res/textures/trees/4ao.dds", false}},
		},
	};

public:

	GameScene();

	~GameScene();

	float s1(float v) { return 3 * v * v - 2 * v * v * v; }
	
	float smoothstep(float a, float b, float x)
	{
		return s1(std::clamp((x - a) / (b - a), 0.f, 1.f));
	}

	virtual void onUpdate(float deltaTime) override;

	virtual void onRender() override;

	virtual void onImGuiRender() override;
	virtual void onSceneTransitionConstruction() override;
	virtual void onSceneTransitionDestruction() override;

};
