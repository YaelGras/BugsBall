#pragma once

#include "../Cube.h"
#include "../Mesh.h"
#include "..\..\abstraction\Effect.h"
#include "../../abstraction/Camera.h"
#include "../../abstraction/TextureCube.h"

#include "../../Renderer.h"

#include <string>

class Skybox {


private:

	struct SkyboxParam {
		XMMATRIX viewProj{};
		XMVECTOR camPos{};
	};
	Mesh	m_mesh		= Cube::getCubeMesh();
	Effect	m_skyboxPass;


	TextureCube m_tex;



public:

	Skybox(const std::string& path = "res/textures/garden.dds") {

		if (!std::filesystem::exists(path)) throw std::runtime_error("No skybox found");
		m_tex = std::move(TextureCube{ path });
		m_skyboxPass.loadEffectFromFile("res/effects/skybox.fx");
		
		m_skyboxPass.addNewCBuffer("SkyboxCbuffer", sizeof(SkyboxParam));


		InputLayout testlayout;
		testlayout.pushBack<3>(InputLayout::Semantic::Position);
		m_skyboxPass.bindInputLayout(testlayout);

	}

	const Mesh& getMesh() const { return m_mesh; }


	void renderSkybox(Camera& camera) const 
	{
		static SkyboxParam skypar{};

		m_skyboxPass.bindTexture("tex", m_tex.getResourceView());

		skypar.viewProj = XMMatrixTranspose(camera.getVPMatrix());
		skypar.camPos = camera.getPosition();
		m_skyboxPass.updateConstantBuffer(skypar, "SkyboxCbuffer");
		m_skyboxPass.apply();
		
		Renderer::renderCubemap(camera, m_mesh, m_skyboxPass);
		m_skyboxPass.unbindTexture("tex");
		m_skyboxPass.unbindResources();
		
	}

};