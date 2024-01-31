#pragma once

#define _XM_NO_INTRINSICS_
#include <DirectXMath.h>

#include "Effect.h"
#include "FrameBuffer.h"
#include "../World/Lights/LightManager.h"
#include "VFXPipeline/VFXPipeline.h"
#include "World/Billboard.h"
#include <d3d11.h>


class Mesh;
class Camera;
class Skybox;
class Terrain;

struct ID3D11ShaderResourceView;

struct meshParams
{
	DirectX::XMMATRIX worldMat;
};
struct cameraParams
{
	DirectX::XMMATRIX viewProj;
	DirectX::XMVECTOR cameraPos; // la position de la caméra 
};
struct sunParams
{
	DirectX::XMMATRIX sunViewProj; // la matrice totale 
	DirectX::XMVECTOR sunRange;
	DirectX::XMVECTOR actualsunPos;
};



class DeferredRenderer
{
private:

	FrameBuffer m_gBuffer;
	FrameBuffer m_sunShadows; // 1 rtv, 1 depth stencil
	Effect m_sunShadowsEffects;
	Camera m_sunCamera;
	bool m_isShadowPass;

	Effect m_gPass, m_lightPass, m_VFXPass;
	Effect m_deferredTerrainFx;
	LightManager m_lights;
	mutable ID3D11ShaderResourceView* m_skyboxSRV = nullptr;

	d3d11_graphics::RenderingContext m_context;
	VFXPipeline m_pipeline;


	BillboardRenderer m_lightBillboardsRenderer{"res/effects/billboardsDeferred.fx"}; // todo use defines
	BillboardRenderer m_billboardsRenderer{}; // todo use defines

public:

	void update(Camera& cam, float dt  =0);
	void renderDeferred(const std::function<void()>& renderFn, Camera& cam) ;
	void renderDeferredVFX(const std::function<void()>& renderFn, Camera& cam) ;

	void renderTerrain(Camera& cam, Terrain& terrain) const;
	void renderMesh(Camera& cam, const Mesh& mesh)const ;
	void renderSkybox(Camera& cam, const Skybox& skybox)const;

	void addBillboard(const Billboard& billboard)
	{
		m_billboardsRenderer.addBillboard(billboard);
	}


	void addImpostor(const XMVECTOR& camPos, const Billboard& impostor)
	{
		m_billboardsRenderer.addImpostor(impostor, camPos);
	}

	void addImpostor(const XMVECTOR& camPos, float size, XMVECTOR worldPos, const Material& mat, XMVECTOR color = { 1,1,1,1 })
	{
		m_billboardsRenderer.addImpostor(camPos, size, worldPos, mat, color);
	}

	void lightImpostor(const XMVECTOR& camPos, float size, XMVECTOR worldPos, const Material& mat, XMVECTOR color = { 1,1,1,1 })
	{
		m_lightBillboardsRenderer.addImpostor(camPos, size, worldPos, mat, color);
	}
	void lightBillboard(const Billboard& billboard)
	{
		m_lightBillboardsRenderer.addBillboard(billboard);
	}

	LightManager& getLightManager() { return m_lights; }





	void clear() noexcept
	{
		m_gBuffer.clear();
		m_billboardsRenderer.clear();
	}



	ID3D11DepthStencilView* getDSV() const { return m_gBuffer.getDepth().getView(); }
	Camera& getSunCamera() { return m_sunCamera; };
	const VFXPipeline& getPipeline() const noexcept { return m_pipeline; }



	
	void uploadLights() const;
	void showDebugWindow();

	void renderLightBillboards(Camera& cam)  
	{
		if (m_isShadowPass) return;
		WindowsEngine::getInstance().getGraphics().setBackfaceCull(false);
		m_lightBillboardsRenderer.render(cam);
		WindowsEngine::getInstance().getGraphics().setBackfaceCull(true);
	}

private:

	void updateSunPass()
	{

		cameraParams cp{ XMMatrixTranspose(m_sunCamera.getVPMatrix()), m_sunCamera.getPosition() };
		m_sunShadowsEffects.updateConstantBuffer(cp, "cameraParams");
	}
	void updateLightPass(Camera& cam);
	void updateGeometryPass(Camera& cam) const;

	/* Fills the gBuffer with the scene informations */
	// Use deferredRenderer.renderMesh()
	void computeGeometryPass(const std::function<void()>& renderFn) const;
	void combineAndApplyLights(Camera& camera) const;
	void applyPostEffects();
	void renderBillboard(Camera& cam) const
	{
		if (m_isShadowPass) return;
		m_billboardsRenderer.render(cam);
	}



	//////////////////////////////////////////////////////////////////////////////////
	///
public:
	DeferredRenderer();
	DeferredRenderer(const DeferredRenderer&) = delete;
	DeferredRenderer(DeferredRenderer&&) = delete;
	DeferredRenderer& operator=(const DeferredRenderer&) = delete;
	DeferredRenderer& operator=(DeferredRenderer&&) = delete;
	~DeferredRenderer() = default;

};
