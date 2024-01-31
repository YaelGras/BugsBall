#include "Scene.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

#include "SceneTransition.h"

SceneTransition* SceneManager::s_currentTransition = nullptr;
std::vector<SceneBuilder> SceneManager::s_availableScenes{};
size_t SceneManager::s_activeSceneIndex		= -1;
Scene* SceneManager::s_activeScene			= nullptr;

void SceneManager::onRender() 
{
    if (s_currentTransition) return;
	s_activeScene->onRender();

}


void SceneManager::onUpdate(float deltaTime)
{
    if (s_currentTransition)
    {
		// returns true if the the transition is still ongoing
        bool onGoing = s_currentTransition->step(deltaTime);

        if (s_currentTransition->getHalfLifeReached()) {
            s_currentTransition->getSourceScene()->onSceneTransitionDestruction();
            s_currentTransition->getDestinationScene()->onSceneTransitionConstruction();
        }

        if (!onGoing)
        {
            s_currentTransition->release();
            delete s_currentTransition;
            s_currentTransition = nullptr;
        }
        return;
    }

	s_activeScene->onUpdate(deltaTime);
}

bool SceneManager::onImGuiRender()
{
    Scene* currentScene = s_activeScene;

#ifdef _DEBUG
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    s_activeScene->onImGuiRender();

    ImGui::Begin("Scenes");
    for (size_t i = 0; i < s_availableScenes.size(); i++) {
        auto& [name, provider] = s_availableScenes[i];
        if (ImGui::Button(name.c_str()))
            switchToScene(i);
        if (s_activeSceneIndex == i) {
            ImGui::SameLine();
            ImGui::Text("< active");
        }
    }
    ImGui::End();
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
#endif

    return currentScene != s_activeScene;
}

void SceneManager::registerScene(const std::string& name, const SceneBuildFn& provider)
{
    s_availableScenes.emplace_back(std::make_pair(name, provider));
}


void SceneManager::switchToScene(size_t sceneIndex)
{
	auto& [name, provider] = s_availableScenes[sceneIndex];
    Scene* destination = provider();

    if (!s_currentTransition && s_activeScene)
    {
        s_currentTransition = new SceneTransition(s_activeScene, destination, TransitionCurve::LINEAR, TransitionType::FADE);
        s_currentTransition->setDuration(2);
        //std::cout << "Created a new scene transition" << std::endl;
    }

    s_activeSceneIndex = sceneIndex;
    s_activeScene = destination;
}



void SceneManager::switchToScene(const std::string& n)
{
    auto it = std::ranges::find_if(s_availableScenes, [&](SceneBuilder p) {return p.first == n; });
    auto& [name, provider] = *it;
    Scene* destination = provider();

    if (!s_currentTransition && s_activeScene)
    {
        s_currentTransition = new SceneTransition(s_activeScene, destination, TransitionCurve::LINEAR, TransitionType::FADE);
        s_currentTransition->setDuration(2);
        
    }
    s_activeSceneIndex = std::distance(s_availableScenes.begin(), it);
    s_activeScene = destination;
}