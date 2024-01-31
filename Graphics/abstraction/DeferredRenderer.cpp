#include "DeferredRenderer.h"
#include "Camera.h"

#include "Renderer.h"
#include "World/Frustum.h"
#include "World/Material.h"
#include "World/WorldRendering/Skybox.h"
#include "World/WorldRendering/Terrain.h"
#include "../Platform/WindowsEngine.h"
#include "FrameBufferStack.h"

struct lightStruct
{
	std::vector<hlsl_GenericLight> lights;
}static s_lights;

static XMVECTOR sunLookat{ { 321.f,-379.f,702.F} };

float yaw = 0;
float pitch = 0;
float roll = 0;

static std::optional<hlsl_GenericLight> lightToBillboard = std::nullopt;

DeferredRenderer::DeferredRenderer()
	: m_gBuffer{ 6 }
	, m_sunShadows{ 1, WindowsEngine::getInstance().getGraphics().getWinSize().first * 4, WindowsEngine::getInstance().getGraphics().getWinSize().second * 4 }

	{
		/*	Normal
		 *	Albedo
		 *	Position
		 *	Specular
		 *	AO
		 *	Roughness
		 *	+ depth
		 */

		m_context = WindowsEngine::getInstance().getGraphics().getContext();


		//auto winSize = WindowsEngine::getInstance().getGraphics().getWinSize();
		//m_sunShadows = FrameBuffer{ 1, winSize.first * 2, winSize.second * 2 };


		m_gPass.loadEffectFromFile("res/effects/gPass.fx");
		m_lightPass.loadEffectFromFile("res/effects/lightPass.fx");
		m_deferredTerrainFx.loadEffectFromFile("res/effects/deferredTerrain.fx");
		// add vfx pass

		//-------------------------------------------------------------------------------------------//

		InputLayout vertexLayout;
		vertexLayout.pushBack<3>(InputLayout::Semantic::Position);
		vertexLayout.pushBack<3>(InputLayout::Semantic::Normal);
		vertexLayout.pushBack<2>(InputLayout::Semantic::Texcoord);

		m_lightPass.addNewCBuffer("cameraParams", sizeof(cameraParams)); // we need VP and camera pos
		m_lightPass.addNewCBuffer("lightsBuffer", sizeof(hlsl_GenericLight) * 16); // we need VP and camera pos

		m_gPass.addNewCBuffer("cameraParams", sizeof(cameraParams)); // we only need VP matrix
		m_gPass.addNewCBuffer("meshParams", sizeof(meshParams));

		m_deferredTerrainFx.addNewCBuffer("meshParams", sizeof(meshParams));
		m_deferredTerrainFx.addNewCBuffer("cameraParams", sizeof(cameraParams));

		m_gPass.bindInputLayout(vertexLayout);
		m_deferredTerrainFx.bindInputLayout(vertexLayout);

		//-------------------------------------------------------------------------------------------//

		m_pipeline.setGbuffer(&m_gBuffer);

		//-------------------------------------------------------------------------------------------//


		m_sunCamera.setPosition({ 325.f,147.f,801.F });
		m_sunCamera.setProjection<OrthographicProjection>(OrthographicProjection{-800,800,-800,800, -8,140});
		m_sunShadowsEffects.loadEffectFromFile("res/effects/sunPass.fx");
		m_sunShadowsEffects.addNewCBuffer("cameraParams", sizeof(cameraParams));
		m_sunShadowsEffects.addNewCBuffer("meshParams", sizeof(meshParams));
		m_lightPass.addNewCBuffer("sunParams", sizeof(sunParams));
		m_sunShadowsEffects.bindInputLayout(vertexLayout);
		m_isShadowPass = false;

	}


	void DeferredRenderer::update(Camera& cam, float dt /*=0*/)
	{

		m_billboardsRenderer.clear();
		m_lightBillboardsRenderer.clear();
		m_billboardsRenderer.update(cam);
		m_lightBillboardsRenderer.update(cam);

		m_sunShadows.clear();
		m_sunCamera.lookAt(sunLookat);
		m_sunCamera.updateCam();
		updateSunPass();

		updateGeometryPass(cam);
		updateLightPass(cam);
		m_pipeline.setCamera(cam);
		m_pipeline.update(dt);
		static FLOAT rgba[4] = { 0,0,0,1 };
		m_context.context->ClearUnorderedAccessViewFloat(m_context.postProcessUAV, rgba);
		m_context.context->ClearRenderTargetView(m_context.postProcessRTV, rgba);
		// VFX PASS
	}


	void DeferredRenderer::renderDeferred(const std::function<void()>& renderFn, Camera& cam) 	{
		computeGeometryPass(renderFn);
		combineAndApplyLights(cam);

	}


	void DeferredRenderer::renderDeferredVFX(const std::function<void()>& renderFn, Camera& cam)
	{

		static ID3D11RenderTargetView* nullrtv = nullptr;

		// -- Shadow pass
		m_isShadowPass = true;
		m_sunShadows.bindDepthOnly();
		renderFn();
		m_sunShadows.unbind();
		m_isShadowPass = false;
		WindowsEngine::getInstance().getGraphics().setDefaultViewport();

		// -- Geometry pass
		m_gBuffer.bind();
		renderFn();
		m_gBuffer.unbind();

		// Light pass
		m_context.context->OMSetRenderTargets(1, &m_context.postProcessRTV, nullptr);

		m_lightPass.bindTexture("sunDepth", m_sunShadows.getDepth().getSrv());
		combineAndApplyLights(cam);


		m_context.context->OMSetRenderTargets(1, &m_context.postProcessRTV, m_gBuffer.getDepth().getView());

		// Billboards + VFX
		renderBillboard(cam);
		applyPostEffects();

		// Output to screen / last framebuff
		FrameBufferStack::getInstance().rebind();
		if (auto& fbs = FrameBufferStack::getInstance(); fbs.getCount() >= 1) fbs.rebind();
		else Renderer::setBackbufferToDefault();
		Renderer::blitTexture(Texture(m_context.postProcessSRV));
		Renderer::clearDepth();

//#ifdef NDEBUG
//		m_billboardsRenderer.clear();
//		if (lightToBillboard.has_value())
//		{
//			static Texture plTex("res/textures/ampoule.dds");
//			static Texture slTex("res/textures/spot.dds");
//			float size = XMVectorGetX(XMVector3Length(cam.getPosition() - lightToBillboard.value().position)) / 10.f;
//			m_billboardsRenderer.addImpostor(cam.getPosition(), size, lightToBillboard.value().position, (lightToBillboard.value().type==1 ? plTex : slTex));
//			if (lightToBillboard.value().type == 2)
//				Renderer::addDebugLine(lightToBillboard.value().position,
//					lightToBillboard.value().position + XMVector3Normalize(lightToBillboard.value().direction));
//		}
//		renderBillboard(cam);
//

//#endif

	}


	void DeferredRenderer::renderMesh(Camera& cam, const Mesh& mesh) const
	{

		if (m_isShadowPass)
		{
			Renderer::renderMesh(m_sunCamera, mesh, m_sunShadowsEffects);
			return;
		}

		if (!mesh.getMaterials().empty())
		{

			const Material& mat = mesh.getMaterials()[0];
			m_gPass.bindTexture("ambiantOcclusion", *mat.queryTexture<TextureType::AO>());
			m_gPass.bindTexture("albedo", *mat.queryTexture<TextureType::ALBEDO>());
			m_gPass.bindTexture("normalMap", *mat.queryTexture<TextureType::NORMAL>());
			m_gPass.bindTexture("roughness",* mat.queryTexture<TextureType::ROUGHNESS>());
			m_gPass.bindTexture("specular",* mat.queryTexture<TextureType::SPECULAR>());
		}
		Renderer::renderMesh(cam, mesh, m_gPass);
		m_gPass.unbindTexture("ambiantOcclusion");
		m_gPass.unbindTexture("albedo");
		m_gPass.unbindTexture("normalMap");
		m_gPass.unbindTexture("roughness");
		m_gPass.unbindTexture("specular");

	}

	void DeferredRenderer::renderTerrain(Camera& cam, Terrain& terrain) const
	{

		if (m_isShadowPass)
		{
			//for (auto&& chunk : terrain.getMesh())
			//{
			//	//Renderer::renderMesh(m_sunCamera, chunk, m_sunShadowsEffects);
			//}
			return;
		}


		static Texture m_grassTexture{ L"res/textures/cobblea.dds" };		
		static Texture m_grassTexture_normal{ L"res/textures/cobblen.dds" };

		
		static Texture m_rockTexture{ L"res/textures/rra.dds" };
		static Texture m_rockTexture_normal{ L"res/textures/rrn.dds" };
		
		static Texture m_moss{ L"res/textures/sa.dds" };
		static Texture m_moss_normal{ L"res/textures/s.dds" };

		m_deferredTerrainFx.bindTexture("grassTexture", m_grassTexture.getTexture());
		m_deferredTerrainFx.bindTexture("rockTexture", m_rockTexture.getTexture());
		m_deferredTerrainFx.bindTexture("snowTexture", m_moss.getTexture());
		m_deferredTerrainFx.bindTexture("snow_normal", m_moss_normal.getTexture());
		m_deferredTerrainFx.bindTexture("grass_normal", m_grassTexture_normal.getTexture());
		m_deferredTerrainFx.bindTexture("rock_normal", m_rockTexture_normal.getTexture());

		m_deferredTerrainFx.updateConstantBuffer(cameraParams{
			XMMatrixTranspose(cam.getVPMatrix()), cam.getPosition()
			}, "cameraParams");

		m_deferredTerrainFx.apply();

		Frustum f = Frustum::createFrustumFromPerspectiveCamera(cam);
		for (auto&& chunk : terrain.getMesh())
		{
			if (f.isOnFrustum(chunk.getBoundingBox()))
			{
				Renderer::renderMesh(cam, chunk, m_deferredTerrainFx);
			}
		}	

		m_deferredTerrainFx.unbindResources();
	}

	void DeferredRenderer::renderSkybox(Camera& cam, const Skybox& skybox) const
	{
		if (m_isShadowPass)	return;
		if (cam.getProjection<OrthographicProjection>().type == ProjectionType::ORTHOGRAPHIC) return;
		m_skyboxSRV = m_gBuffer.bindUnlitRTV();
		skybox.renderSkybox(cam);
	}


	void DeferredRenderer::updateLightPass(Camera& cam)
	{

		m_lightPass.updateConstantBuffer(cameraParams{
			cam.getVPMatrix(), cam.getPosition()
			}, "cameraParams");
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		auto& proj = m_sunCamera.getProjection<OrthographicProjection>();

		sunParams sp = { XMMatrixTranspose(m_sunCamera.getVPMatrix()) , {proj.znear, proj.zfar}, m_sunCamera.getPosition() };

		m_lightPass.updateConstantBuffer(sp, "sunParams");
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	}
	void DeferredRenderer::updateGeometryPass(Camera& cam) const
	{
		m_gPass.updateConstantBuffer(cameraParams{
			XMMatrixTranspose(cam.getVPMatrix()), cam.getPosition()
			}, "cameraParams");

	}

	/* Fills the gBuffer with the scene informations */
	// Use deferredRenderer.renderMesh()
	void DeferredRenderer::computeGeometryPass(const std::function<void()>& renderFn) const
	{
		m_gBuffer.bind();
		renderFn();
		m_gBuffer.unbind();
	}

	void DeferredRenderer::combineAndApplyLights(Camera& camera) const
	{

		uploadLights();

		m_lightPass.bindTexture("normal", m_gBuffer.getResource(0));
		m_lightPass.bindTexture("albedo", m_gBuffer.getResource(1));
		m_lightPass.bindTexture("position", m_gBuffer.getResource(2));
		m_lightPass.bindTexture("specular", m_gBuffer.getResource(3));
		m_lightPass.bindTexture("ambiantOcclusion", m_gBuffer.getResource(4));
		m_lightPass.bindTexture("roughness", m_gBuffer.getResource(5));
		m_lightPass.bindTexture("unlitTexture", m_skyboxSRV);
		m_lightPass.apply();

		Renderer::draw(6);

		m_lightPass.unbindTexture("normal");
		m_lightPass.unbindTexture("albedo");
		m_lightPass.unbindTexture("position");
		m_lightPass.unbindTexture("specular");
		m_lightPass.unbindTexture("ambiantOcclusion");
		m_lightPass.unbindTexture("roughness");
		m_lightPass.unbindTexture("unlitTexture");
		m_lightPass.unbindResources();

	}


	void DeferredRenderer::applyPostEffects() 
	{
		m_pipeline.renderPipeline();

	}


	void DeferredRenderer::uploadLights() const
	{

		s_lights.lights = m_lights.convertLights();
		m_lightPass.updateConstantBuffer(s_lights.lights.data()[0], "lightsBuffer");
	}



	void DeferredRenderer::showDebugWindow()
	{

		
		ImGui::Begin("Deferred Shading debug window");
		if (ImGui::Button("Recompile shader"))
		{
			m_deferredTerrainFx.recompile();
			m_gPass.recompile();
			m_lightPass.recompile();
			m_billboardsRenderer.recompile();
			m_lightBillboardsRenderer.recompile();
		}
		ImGui::SeparatorText("Sun Vision");
		ImGui::Image((void*)m_sunShadows.getDepth().getSrv(), ImVec2(311, 173));
		ImGui::SeparatorText("GBUFFER");

		for (ID3D11ShaderResourceView* srv : m_gBuffer.getShaderResourceViews())
		{
			ImGui::Image((void*)srv, ImVec2(311, 173));
		}
		ImGui::Image((void*)m_skyboxSRV, ImVec2(311, 173));
		


		ImGui::End();
		ImGui::Begin("Deferred Controls");
		static float sunstrenght = 1.f;
		static float sunPos[4] = { 325.f,147.f,801.F };
		if (ImGui::DragFloat("Sun strength", &sunstrenght, 0.05f, 0.f, 2.f))
		{
			m_lightPass.setUniformVector("sunStrength", { sunstrenght });
			m_gPass.setUniformVector("sunStrength", { sunstrenght });
			m_deferredTerrainFx.setUniformVector("sunStrength", { sunstrenght });


		}

		ImGui::DragFloat("yaw ", &yaw, 0.1f);
		ImGui::DragFloat("pitch ", &pitch, 0.1f);
		ImGui::DragFloat("roll ", &roll, 0.1f);

		static float sunDir[4] = { 100.0f, 100.f, 10.0F };
		if (ImGui::DragFloat4("Sun Dir", &sunDir[0]))
		{
			m_deferredTerrainFx.setUniformVector("sunDir", { sunDir[0],sunDir[1] ,sunDir[2] ,sunDir[3] });
			m_lightPass.setUniformVector("sunDir", { sunDir[0],sunDir[1] ,sunDir[2] ,sunDir[3 ]});
			m_gPass.setUniformVector("sunDir", { sunDir[0],sunDir[1] ,sunDir[2] ,sunDir[3] } );
			m_lightBillboardsRenderer.getEffect().setUniformVector("sunDir", { sunDir[0],sunDir[1] ,sunDir[2] ,sunDir[3] });
		}

		if (ImGui::DragFloat4("Sun Position", &sunPos[0]))
		{
			m_sunCamera.setPosition({ sunPos[0],sunPos[1] ,sunPos[2] ,sunPos[3] });
		}
		static float _zn = -8.F;
		static float _zf = 136.F;
		static float _width = 800.f;
		static float _height =  800.f;
		if (
			ImGui::DragFloat("sun zNear", &_zn, 0.1f, -10.F, _zf-1.F) + ImGui::DragFloat("sun zFar", &_zf, 1.f, _zn+1.F, 200.F) +
			ImGui::DragFloat("sun width", &_width, 0.1f, 1.F, 1000) + ImGui::DragFloat("sun height", &_height, 0.1f, 1.F, 1000.f)
			)
		{
			m_sunCamera.setProjection(OrthographicProjection{ -_width, _width, -_height, _height,_zn, _zf });
		}
		ImGui::DragFloat3("lookAtSun", &sunLookat.vector4_f32[0]);


		ImGui::End();

	///////////////
		
		lightToBillboard = m_lights.showDebugWindow();
		m_pipeline.showImguiDebugWindow();

	}
