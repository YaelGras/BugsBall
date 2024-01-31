#pragma once

#include "Scene.h"
#include "Renderer.h"

#include "abstraction/abstraction_core.h"

#include "../../Platform/IO/Sound.h"

#include "../GameUI.h"


class MainMenu : public Scene
{
private:
	Texture m_background{"res/textures/Menu_screen.dds"};
	Texture m_logo{"res/textures/logo.dds"};
	
	FrameBuffer m_fbo;

	bool m_sound = true;
	
	Texture m_screenShot;
	float m_elapsedTime = 0;
	Texture m_bgs[4] =
	{
		{L"res/textures/backgrounds/1.dds"},
		{L"res/textures/backgrounds/2.dds"},
		{L"res/textures/backgrounds/3.dds"},
		{L"res/textures/backgrounds/4.dds"}
	};
	

public:
	MainMenu()
	{
		UIRenderer::attachMouse(wMouse.get());
		UIRenderer::init();
	}

	~MainMenu() {
		Renderer::cleanupAfterSceneDeletion();
	}

	void onUpdate(float deltaTime) override
	{
		m_elapsedTime += deltaTime;

		if (!Sound::isPlaying(SoundType::MainScreen) && m_sound && !Sound::isPlaying(SoundType::Background))
		{
			Sound::playSound(SoundType::MainScreen);
			m_sound = false;
		}
	}

	inline float smoothstep(float a, float b, float x)
	{
		const float t = std::clamp((x - a) / (b - a), 0.F, 1.f);
		return t * t * (3.0f - (2.0f * t));
	}

	void onRender() override
	{
		const auto winSize = WindowsEngine::getInstance().getGraphics().getWinSize();
		constexpr auto size = 500;
		Renderer::clearText();
		Renderer::clearScreen(0, 0, 0, 1);
		
		
		// -- Darken + loop through backgrounds
		float blendTime = 2.F;
		float holdTime = 3.F;
		
		int currentSecond = std::floor(std::fmod(m_elapsedTime/(blendTime + holdTime), 4.f));
		float currentPercentage = std::fmod(m_elapsedTime/(blendTime + holdTime), 4.f) - currentSecond;
		
		float blendFactor = blendTime / (blendTime + holdTime);
		float currentAlpha = smoothstep(0, blendFactor, currentPercentage);
		
		Renderer::blitTexture(m_bgs[currentSecond], XMVECTOR{1.f, 1.f, 1.f, 1 -  currentAlpha });
		Renderer::blitTexture(m_bgs[(currentSecond+1)%4], XMVECTOR{1.f, 1.f, 1.f, currentAlpha });
		
		
		Renderer::renderQuadOnScreen(XMVECTOR{(winSize.first / 2.f) - size / 2.F, -100}, XMVECTOR{size, size}, m_logo,
		                             XMVECTOR{});
		Renderer::renderBatch2D();
		
		if (GameUI::isOption()) m_fbo.bind(); // < Used to darken the screen
		if (GameUI::isOption())
		{
			m_fbo.unbind();
			Renderer::setBackbufferToDefault();
			m_screenShot = Texture(m_fbo.getResource(0));
			Renderer::blitTexture(m_screenShot, DirectX::XMVECTOR{ {1.F,1.F,1.F,0.5F} });
		}

		UIRenderer::clear();
		GameUI::mainMenu();
	}

	void onImGuiRender() override
	{}

	virtual void onSceneTransitionDestruction() override {
		UIRenderer::cleanUp();
		Sound::stopAllSounds();
	}

	virtual void onSceneTransitionConstruction() override {

		UIRenderer::init();
		m_sound = true;
	}

};
