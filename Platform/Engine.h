 #pragma once
#include "../Utils/Singleton.h"
#include "../Utils/Timer.h"
#include "../Graphics/d3d11_graphics.h"

#include "imgui.h"

#include "../src/Scenes/Scene.h"
#include "../Physics/PhysicEngine.h"


constexpr Timer::count_type FPS_COUNT = 144;
constexpr Timer::value_type FRAME_TIME = 1.0 / static_cast<Timer::value_type>(FPS_COUNT);

template<class _RenderingDevice= d3d11_graphics>
concept canRender = requires (_RenderingDevice gfx) {

	{gfx.present()} -> std::same_as<void>;

};

template <class _Platform, class _RenderingDevice= d3d11_graphics>
	requires canRender<_RenderingDevice>
class Engine : public Singleton<_Platform>
{
public:
	virtual void run()
	{
		m_nextTime = static_cast<Timer::count_type>(getTimePlatform()); // not sure about this cast
		m_previousTime = m_nextTime;
		bool isRunning = true;

		while (isRunning)
		{
			// Propre à la plateforme - (Conditions d'arrêt, interface, messages)
			isRunning = runPlatform();

			// appeler la fonction d'animation
			if (isRunning)
			{

				isRunning = animation();		

			}

		}
	}

	virtual int init()
	{
		// Propre à la plateforme
		initPlatform();

		// * Initialisation du dispositif de rendu
		p_renderingDevice = createRenderingDevicePlatform(GRAPHICS_MODE::WINDOWED);

		// * Initialisation de la scène

		// * Initialisation des paramètres de l'animation et 
		//   préparation de la première image
		initAnimation();

			

		return 0;
	}

	virtual bool animation() { 

		static bool resetDt = false;
		m_nextTime = m_clock.getTimeCount();
		float deltaTime = static_cast<float>(m_clock.getTimeBetweenCounts(m_previousTime, m_nextTime));
		if (resetDt)
		{
			deltaTime = 0;
			m_previousTime = m_nextTime;
			resetDt = false;
		}
		if (deltaTime >= FRAME_TIME) {

			
			SceneManager::onUpdate(deltaTime);
			PhysicsEngine::onUpdate(deltaTime);
			SceneManager::onRender();
			if (SceneManager::onImGuiRender()) // < returns true if we switch scene
			{
				resetDt = true;
			}
			ImGui::UpdatePlatformWindows();
			p_renderingDevice->present();
			m_previousTime = m_nextTime;

		}


		return true;
	
	}

	virtual int initAnimation()
	{
		m_nextTime = static_cast<Timer::count_type>(getTimePlatform()); // not sure about this cast
		m_previousTime = m_nextTime;
		// première Image
		renderScene();
		return true;
	}

	virtual _RenderingDevice* createRenderingDevicePlatform(GRAPHICS_MODE cdsMode) = 0;



protected:
	virtual ~Engine() = default;

	// Spécifiques - Doivent être implantés
	virtual bool runPlatform() = 0;
	virtual int initPlatform() = 0;

	virtual bool renderScene()
	{
		beginRenderScenePlatform();
		endRenderScenePlatform();
		return true;
	}

	virtual void beginRenderScenePlatform() = 0;
	virtual void endRenderScenePlatform() = 0;

	// Gets the time using platform specific calls
	virtual Timer::value_type getTimePlatform() const = 0;

protected:
	// Variables pour le temps de l’animation
	Timer::count_type m_nextTime{};
	Timer::count_type m_previousTime{};
	Timer m_clock{};

	_RenderingDevice* p_renderingDevice = nullptr; // This will store a Graphics() on windows

};


