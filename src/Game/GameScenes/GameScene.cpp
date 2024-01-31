#include "GameScene.h"
#include <World/Managers/MeshManager.h>
#include <stbi/stb_image.h>

#include "abstraction/2D/UI/UIRenderer.h"
#include "../CameraController.h"
#include "../GameUI.h"
#include "../../Platform/IO/Sound.h"

GameScene::GameScene()
{
	e_blur = m_renderer.getPipeline().queryVisualEffect<RadialBlur>(EffectType::RadialBlur);
	e_lines = m_renderer.getPipeline().queryVisualEffect<SpeedLines>(EffectType::SpeedLines);


	m_gameLogic.init(this, e_blur, e_lines, currentCamera, m_terrain);

	auto m = MeshManager::getMeshReference("NPC").get();
	m_gameLogic.MoveAllToStartCheckpoint(m_terrain);

	//// Create terrain collisions
	phm.setTerrain(&m_terrain);
	CameraController::setTerrain(&m_terrain);

	// Attach mouse for UI (will move somewhere else)
	UIRenderer::init();
	UIRenderer::attachMouse(wMouse.get());

	Renderer::setBackbufferToDefault();

	m_gameLogic.restartGame(currentCamera);
	m_renderer.getSunCamera().setProjection(OrthographicProjection{ -800,800,-800,800, -8,140 });
	m_renderer.getSunCamera().setPosition({ 325.f,138.f,665.F });
	m_renderer.getSunCamera().lookAt({ 321.f,-379.f,702.F });


	
	auto lights{ JsonParser::getLights() };
	auto& LM = m_renderer.getLightManager();
	std::ranges::for_each(lights, [&](const FormatJsonLight& l)
		{
			switch (l.type)
			{
			case TypeLight::DIRECTIONAL:
			{
				DirectionalLight dir;
				dir.ambiant = l.ambiant;
				dir.diffuse = l.diffuse;
				dir.direction = l.direction;
				dir.strength = l.strength;
				dir.isOn = true;

				LM.addDirlight(std::move(dir));
				break;
			}
			case TypeLight::SPOTLIGHT:
			{
				SpotLight spot;
				spot.ambiant = l.ambiant;
				spot.diffuse = l.diffuse;
				spot.direction = l.direction;
				spot.outsideAngle = l.falloff;
				spot.position = l.position;
				spot.insideAngle = l.radius;
				spot.strength = l.strength;
				spot.specularFactor = l.specular;
				spot.isOn = true;
				LM.addSpotlight(std::move(spot));
				break;
			}

			case TypeLight::POINT:
			{
				PointLight point;
				point.ambiant = l.ambiant;
				point.diffuse = l.diffuse;
				point.position = l.position;
				point.range = l.range;
				point.distance = l.falloff;
				point.isOn = true;
				point.specularFactor = l.specular;
				LM.addPointlight(std::move(point));
				break;
			}
			}
		});


	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Tree generation from trees.png
	// Redpixel indicates the position of a tree, and it looks at the closest green pixel to determine its direction

	int w, h, channel;

	unsigned char* data = stbi_load("res/textures/trees.png", &w, &h, &channel, 3);
	if (!data) throw std::exception();
	std::vector<XMVECTOR> redPixels;
	std::vector<XMVECTOR> greenPixels;

	for (int i = 0; i < w * h * 3; i += 3)
	{
		int red = data[i];
		int green = data[i + 1];
		int blue = data[i + 2];

		if (red == 255 && blue == 0 && green == 0)
			redPixels.push_back(XMVECTOR{ static_cast<float>((i / 3) % w),  static_cast<float>((i / 3) / w) });

		if (green == 255 && blue == 0 && red == 0)
			greenPixels.push_back(XMVECTOR{ static_cast<float>((i / 3) % w),  static_cast<float>((i / 3) / w) });

	}


	for (auto& p : redPixels)
	{
		auto closestGreen = *std::ranges::min_element(greenPixels, std::less{}, [&](auto green) {
			return XMVectorGetX(XMVector2Length(green - p));
			});
		XMVECTOR direction = closestGreen - p;
		XMVECTOR normaleeeee_yeaaha = XMVector3Normalize({ -XMVectorGetX(direction),0, -XMVectorGetY(direction) });
		int size = std::rand() % 50 + 50;
		float redvalue = std::rand() / (float)RAND_MAX;
		float greenvalue = std::rand() / (float)RAND_MAX;
		float bluevalue = std::rand() / (float)RAND_MAX;
		m_billboardsTrees.push_back(
			Billboard{
				{
					m_terrain.getParams().xyScale * XMVectorGetX(p),
					//static_cast<float>(m_terrain.getParams().scaleFactor) + size/2 - size/8,
					m_terrain.getWorldHeightAt({m_terrain.getParams().xyScale * XMVectorGetX(p),0,m_terrain.getParams().xyScale * XMVectorGetY(p)}) + size / 2 ,
					m_terrain.getParams().xyScale * XMVectorGetY(p)
				},
				{	1 - redvalue / 2.f,1 - greenvalue / 1.5f,bluevalue,1		},
				{static_cast<float>(size),static_cast<float>(size),static_cast<float>(size),static_cast<float>(size)},
				&m_trees[0],
				normaleeeee_yeaaha
			});
	}

	stbi_image_free(data);

	auto aabb = MeshManager::getMeshReference("Pedestal")->getBoundingBox();
}

GameScene::~GameScene()
{

	PhysicsEngine::setRunningState(false);
	PhysicsEngine::resetScene();
	PhysicsEngine::setRunningState(true);
	MeshManager::clearCurrentSceneMesh(this);

}

void GameScene::onUpdate(float deltaTime)
{
	// Handle pausing
	m_gameLogic.handleKeyboardInputsMenu(currentCamera, m_terrain, m_isDebug);

	if (m_gameLogic.getInGame())
		m_gameLogic.SoundUpdate();

	PhysicsEngine::setRunningState(!m_gameLogic.isPause());
	if (m_gameLogic.isPause()) return;

	m_gameLogic.updatePositionPlayer(deltaTime, m_terrain);
	if (m_isDebug) m_debugCam.step(deltaTime);
	Camera& cam = (m_isDebug) ? m_debugCam.getCamera() : m_gameLogic.getPlayer().getCurrentCamera();


	// Increase the timer if the run is still going : Not all have finished and we are not paused
	if (m_gameLogic.getResult().size() != 1 + m_gameLogic.getNPC().size()
		&& m_gameLogic.isPlaying()) m_gameLogic.step(deltaTime);

	// Finished run effect
	if (m_gameLogic.isGameOver()) { m_gameLogic.getPlayer().getCurrentCamera().lookAt(m_gameLogic.getPlayer().getPosition()); }

	// Update player and deferred renderer (it is not static yet)
	m_flies.update(deltaTime, { 320,70,437 });
	m_renderer.update(cam, deltaTime);


	// VFX Updates
	const float speed = m_gameLogic.getPlayer().getObject().getLinearVelocityMag();
	constexpr float vmin = 60.F;
	constexpr float vmax = 100.f;
	constexpr float maxRadius = 12.f;
	constexpr float minRadius = 16.f;
	float t = smoothstep(vmin, vmax, speed);

	const float lineRadius = std::lerp(minRadius, maxRadius, t);
	if (!m_gameLogic.isGameOver()) e_lines->enable();
	e_lines->setOpacity(t);
	e_lines->setRadius(lineRadius);



	auto f = m_gameLogic.getPlayer().getObject().getLinearVelocityMag() / m_gameLogic.getPlayer().getMaxVelocity();
	m_gameLogic.getPlayer().getCurrentCamera().setFieldOfView(std::lerp(XM_PIDIV4, XM_PIDIV2, f));
	f /= 5.f;
	e_blur->setIntensity(f);

	// -- Countdown timer
	m_gameLogic.startTimer(deltaTime);

	m_gameLogic.changeColorPassedCheckpointTree(m_renderer);

	m_gameLogic.backMainMenu();

}

void GameScene::onRender() {

	// Clear everything
	Renderer::clearScreen();
	Renderer::clearText();
	m_renderer.clear();

	Camera& cam = (m_isDebug) ? m_debugCam.getCamera() : m_gameLogic.getPlayer().getCurrentCamera();

	if (m_gameLogic.isPause() || m_gameLogic.isGameOver())  m_fbo.bind(); // < Used to darken the screen


	static Material m_shadowMat{ {TextureType::ALBEDO, { "res/textures/player_shadow.dds" }} };
	static Material m_bushMat{
		{TextureType::ALBEDO, Texture{"res/textures/trees/bush.dds"}},
		{TextureType::NORMAL, Texture{"res/textures/trees/bush_n.dds"}},
		{TextureType::AO, Texture{"res/textures/trees/bush_ao.dds"}},
	};

	m_renderer.renderDeferredVFX([&]() -> void
		{
			////////////////////////////////////////////////////////////
			/// -- Render terain and scene meshes
			m_renderer.renderTerrain(cam, m_terrain); // < This has frustum culling
			MeshManager::renderCurrentSceneLoadedMeshes(this, cam, [&](Camera& c, const Mesh& m)-> void {m_renderer.renderMesh(c, m); });

			////////////////////////////////////////////////////////////
			/// -- Render player, AI and push shadow billboards
			if (m_gameLogic.getPlayer().currentViewIsThirdPerson()) m_renderer.renderMesh(cam, m_gameLogic.getPlayer().getMesh());
			m_renderer.addBillboard(
				Billboard{
				XMVectorSetY(m_gameLogic.getPlayer().getPosition(), m_terrain.getWorldHeightAt(m_gameLogic.getPlayer().getPosition()) + 0.1),
				{1,1,1,1},
				{1,1,1,1},
				&m_shadowMat,
				-m_terrain.getNormalAt(m_gameLogic.getPlayer().getPosition())
				});
			std::ranges::for_each(m_gameLogic.getNPC(), [&](const NPC& ia) {

				m_renderer.renderMesh(*currentCamera, ia.getMesh());
				m_renderer.addBillboard(
					Billboard{
					XMVectorSetY(ia.getMesh().getTransform().getTranslation(), m_terrain.getWorldHeightAt(ia.getMesh().getTransform().getTranslation()) + 0.1),
					{1,1,1,1},
					{1,1,1,1},
					&m_shadowMat,
					-m_terrain.getNormalAt(ia.getMesh().getTransform().getTranslation())
					});
				});


			////////////////////////////////////////////////////////////
			/// -- Push Fireflies + Trees 
			std::ranges::for_each(m_flies.getFireflies(), [&](Billboard& i) { m_renderer.addImpostor(cam.getPosition(), i); });
			std::ranges::for_each(m_billboardsTrees, [&](Billboard& t) {m_renderer.lightBillboard(t);});
			static float bushHeight = 250.f;

			////////////////////////////////////////////////////////////
			/// -- Push billoard bushes (borders of the map)
			m_renderer.lightBillboard(Billboard{
				{
					0,
					m_terrain.getParams().scaleFactor + bushHeight / 2.f,
					(float)m_terrain.getParams().width / 2.f
				},

				{ 1,1,1,1 },

				XMVECTOR{(float)m_terrain.getParams().width,bushHeight},

				&m_bushMat, { 1,0,0 } });
			m_renderer.lightBillboard(Billboard{
				{
					(float)m_terrain.getParams().width / 2.f,
					m_terrain.getParams().scaleFactor + bushHeight / 2.f,
					0
				},
				{ 1,1,1,1 },

				XMVECTOR{(float)m_terrain.getParams().width,bushHeight},
				&m_bushMat, { 0,0,1 } });
			m_renderer.lightBillboard(Billboard{
			{
			(float)m_terrain.getParams().width,
			m_terrain.getParams().scaleFactor + bushHeight / 2.f,
			(float)m_terrain.getParams().width / 2.f
			},

		{ 1,1,1,1 },

		XMVECTOR{(float)m_terrain.getParams().width,bushHeight},

		&m_bushMat, { 1,0,0 } });
			m_renderer.lightBillboard(Billboard{
				{
					(float)m_terrain.getParams().width / 2.f,
					m_terrain.getParams().scaleFactor + bushHeight / 2.f,
					(float)m_terrain.getParams().width
				},
				{ 1,1,1,1 },

				XMVECTOR{(float)m_terrain.getParams().width,bushHeight},
				&m_bushMat, { 0,0,1 } });

			////////////////////////////////////////////////////////////
			/// -- Render billboards and skybox
			m_renderer.renderLightBillboards(cam);
			m_renderer.renderSkybox(cam, m_skybox);

		}, cam);


	// Darken the screen if its a pause or we have finished
	if (m_gameLogic.isPause() || m_gameLogic.isGameOver())
	{
		m_fbo.unbind();
		Renderer::setBackbufferToDefault();
		m_screenShot = Texture(m_fbo.getResource(0));
		Renderer::blitTexture(m_screenShot, DirectX::XMVECTOR{ {1.F,1.F,1.F,0.5F} });
	}

	auto pedestal = MeshManager::getMeshReference("Pedestal");

	GameUI::inGame(m_gameLogic, currentCamera, m_terrain);
}

void GameScene::onImGuiRender()
{

#ifdef _DEBUG
	static float playerSpeed = 0.05f;

	MeshManager::onImGuiRender(this);
	m_renderer.showDebugWindow();
	ImGui::Begin("InGame scene debug");
	if (ImGui::DragFloat("Player speed", &playerSpeed, 0.01f, 0, 3.f))
	{
		m_debugCam.setSpeed(playerSpeed);
	}

	ImGui::Checkbox("Enable debug cam", &m_isDebug);
	ImGui::DragFloat3("Free cam pos", m_debugCam.getCamera().getPosition().vector4_f32);
	ImGui::End();
#endif
}

void GameScene::onSceneTransitionDestruction() {
	UIRenderer::cleanUp();
	m_gameLogic.setInGame(false);
	Sound::stopAllSounds();
}

 void GameScene::onSceneTransitionConstruction() {
	UIRenderer::init();
	m_gameLogic.setInGame(true);
}