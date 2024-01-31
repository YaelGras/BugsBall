#pragma once

/////////////////////////////// DEAD SCENE :(

#include "Scene.h"

#include "../../Graphics/World/Player.h"
#include "../../Graphics/World/WorldRendering/Skybox.h"


struct cbufferTmp {
	XMMATRIX viewProj;
	XMVECTOR lightPos;
	XMVECTOR cameraPos;
	XMVECTOR sunColor = { 1.f,1.f,.8f,1.f };
	XMVECTOR sunStrength = { 0.75f };
};


class Parallax : public Scene {

private:

	Skybox m_skybox;
	Player m_player;

	Mesh m_cube = Cube::getInvertedCubeMesh();
	Effect baseMesh;
	Texture brick{ "res/textures/brick.dds" };
	Texture height{ "res/textures/brick_h.dds" };
	Texture ao{ "res/textures/brick_ao.dds" };
	Texture normal{ "res/textures/brick_n.dds" };
	Texture spexcular{ "res/textures/brick_s.dds" };
	float elapsed = 0;
	bool enableRotation = false;
public:

	Parallax() {

		baseMesh.loadEffectFromFile("res/effects/parallaxTest.fx");
		InputLayout layout;
		layout.pushBack<3>(InputLayout::Semantic::Position);
		layout.pushBack<3>(InputLayout::Semantic::Normal);
		layout.pushBack<2>(InputLayout::Semantic::Texcoord);
		baseMesh.bindInputLayout(layout);
		baseMesh.addNewCBuffer("worldParams", sizeof(cbufferTmp));
		baseMesh.addNewCBuffer("meshParams", sizeof(XMMATRIX));
		m_cube.getTransform().setScale({ 30,30,30 });
	}

	virtual void onUpdate(float deltaTime) override {
	
		m_player.step(deltaTime);
		Camera& cam = m_player.getCamera();
		if (enableRotation) elapsed += deltaTime;
		if(enableRotation) m_cube.getTransform().setRotation({0, cos(elapsed),0 });
		cbufferTmp sp;

		sp.viewProj = XMMatrixTranspose(cam.getVPMatrix());
		sp.lightPos = XMVectorSet(-10.0f, 10.0f, -10.0f, 1.0f);
		sp.cameraPos = cam.getPosition();
		
		baseMesh.updateConstantBuffer(sp, "worldParams");

	
	}

	virtual void onRender() override {
		Camera& cam = m_player.getCamera();
		Renderer::clearScreen();
		
		baseMesh.bindTexture("tex", brick);
		baseMesh.bindTexture("heightmap", height);
		baseMesh.bindTexture("ao", ao);
		baseMesh.bindTexture("normalMap", normal);
		baseMesh.bindTexture("specular", spexcular);
		Renderer::renderMesh(cam, m_cube, baseMesh);
		m_skybox.renderSkybox(cam);
	}

	virtual void onImGuiRender() override
	{	
		static float heightScale = 0.05f;
		static float minLayer = 4.0f;
		static float maxLayer = 128.0f;
		if (ImGui::Button("Recompile effect"))
		{
			baseMesh.recompile();
			baseMesh.setUniformFloat("heightScale", heightScale);
			baseMesh.setUniformFloat("minLayer", minLayer);
			baseMesh.setUniformFloat("maxLayer", maxLayer);
		}

		ImGui::Checkbox("Enable Rotation", &enableRotation);

		static XMVECTOR sunPos = { 100,100,100 };
		if (ImGui::DragFloat3("Sun Position", &sunPos.vector4_f32[0]))
		{
			baseMesh.setUniformVector("sunPos", sunPos);

		}

		if (
			ImGui::DragFloat("heightScale", &heightScale, 0.01F, 0, 1) +
			ImGui::DragFloat("minLayer", &minLayer, 1, 0, 128) +
			ImGui::DragFloat("maxLayer", &maxLayer, 1, 0, 128)
			)
		{
			baseMesh.setUniformFloat("heightScale", heightScale);
			baseMesh.setUniformFloat("minLayer", minLayer);
			baseMesh.setUniformFloat("maxLayer", maxLayer);
		}

	}



};