#pragma once

///////////////////////////////////////////////////////////////////
//																///
//	MY BELOVED TEST SCENE !!! DO NOT TOUCH MY BELOVED SCENE !	///
//																///
///////////////////////////////////////////////////////////////////

#include "Scene.h"
#include "abstraction/DeferredRenderer.h"

#include "World/Billboard.h"
#include "VFXPipeline/VFXPipeline.h"
#include "World/Fireflies.h"
#include "World/Player.h"
#include "World/WorldRendering/Skybox.h"


class VFXScene : public Scene {

private:

	Player m_player;
	DeferredRenderer m_renderer;
	Skybox m_skybox;
	Texture bread{ "res/textures/breadbug.dds" };
	Texture bread2{ "res/textures/breadbug_alpha.dds" };

	Texture arbre{ "res/textures/arbre.dds" };
	Texture testNormal{ "res/textures/arbre_n.dds" };
	Texture testAo{ "res/textures/arbre_o.dds" };

	Texture arbre2{ "res/textures/arbre2.dds" };
	Texture testNormal2{ "res/textures/arbre2_n.dds" };
	Texture testAo2{ "res/textures/arbre2_ao.dds" };

	bool renderAsSun;
	Fireflies m_flies;
	AABB m_tmp{ {-0.95f, 0.f, -0.54f},{1.61f, 1.64f, 1.23f} };

	


public:

	VFXScene()
	{
		MeshManager::loadMeshFromFile("bunny","res/mesh/bunny.obj");
		MeshManager::loadToCurrentScene(this, "bunny");
		m_renderer.getPipeline().queryVisualEffect<SpeedLines>(EffectType::SpeedLines)->disable();
		m_renderer.getPipeline().queryVisualEffect<RadialBlur>(EffectType::RadialBlur)->disable();
		renderAsSun = false;
	}

	virtual void onUpdate(float deltaTime) override
	{
		if (!renderAsSun) {
			m_player.step(deltaTime);
			m_player.getCamera().updateCam();
		}
		Camera& cam = (renderAsSun) ? m_renderer.getSunCamera() : m_player.getCamera();
		m_renderer.update(m_player.getCamera(), deltaTime);
		m_flies.update(deltaTime, cam.getPosition());
	}

	virtual void onRender() override
	{
		Renderer::clearScreen();
		m_renderer.clear();
		d3d11_graphics::RenderingContext& ctx = WindowsEngine::getInstance().getGraphics().getContext();

		Camera& cam = (renderAsSun) ? m_renderer.getSunCamera() :  m_player.getCamera();
		if (WindowsEngine::getInstance().getGraphics().hasResized())
		{
			auto winSize = WindowsEngine::getInstance().getGraphics().getWinSize();
			PerspectiveProjection::s_baseAspectRatio = (float)winSize.first / (float)winSize.second;
			cam.setProjection<PerspectiveProjection>({});
			WindowsEngine::getInstance().getGraphics().resizeHasBeenHandled();			
		}

		auto f = m_flies.getFireflies();
		static Material m_arbreMat{ {TextureType::ALBEDO, arbre}, {TextureType::AO, testAo}, {TextureType::NORMAL, testNormal} };
		static Material m_arbreMat2{ {TextureType::ALBEDO, arbre2}, {TextureType::AO, testAo2}, {TextureType::NORMAL, testNormal2} };
		static Material m_breadbugMat{ {TextureType::ALBEDO, bread2} };
		m_renderer.renderDeferredVFX
		(
			[&]()->void
			{
				m_renderer.addImpostor(cam.getPosition(), 10, { 0,-2,15 }, m_breadbugMat, { 1,15,1,1 });
				m_renderer.lightBillboard(Billboard{ { 0,10,15 }, {1,1,1,1}, {100,100,100,100}, &m_arbreMat, {1,0,0} });
				m_renderer.lightBillboard(Billboard{ { 0,10,-15 }, {1,1,1,1}, {100,100,100,100}, &m_arbreMat2, {0,0,1}});
				MeshManager::renderCurrentSceneLoadedMeshes(this, cam, [&](Camera& c, const Mesh& m)-> void {m_renderer.renderMesh(c, m); });


				m_renderer.renderLightBillboards(cam);
				m_renderer.renderSkybox(cam, m_skybox);

			}, cam
		);

	}
	virtual void onImGuiRender() override
	{
		m_renderer.showDebugWindow();
		Renderer::showImGuiDebugData();
		MeshManager::onImGuiRender(this);
		ImGui::Checkbox("Sun view", &renderAsSun);
	}
	~VFXScene() {
		MeshManager::clearCurrentSceneMesh(this);
	}
};
