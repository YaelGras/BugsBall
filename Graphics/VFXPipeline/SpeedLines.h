#pragma once
#include "VFX.h"


class SpeedLines : public VFX
{
private:


	struct speedLinesParams
	{

		float radius;
		float edges;
		float opacity;
		float elapsedTime;


	} m_speedLinesParams;
	static_assert(sizeof(speedLinesParams) % 16 == 0);

	ID3D11Buffer* m_speedLinesBuffer = nullptr;

public:
	SpeedLines()
		: VFX{"res/compute/speedLines.hlsl", "Speed Liens"}
	{

		m_speedLinesParams.radius = 12.0f;
		m_speedLinesParams.edges = 0.3f;
		m_speedLinesParams.elapsedTime = 0;
		m_speedLinesParams.opacity = 1;


		m_isEnabled = true;

		D3D11_BUFFER_DESC paramsDesc = {};
		paramsDesc.Usage = D3D11_USAGE_DEFAULT;
		paramsDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		paramsDesc.ByteWidth = sizeof(speedLinesParams);
		paramsDesc.CPUAccessFlags = 0;
		m_context.device->CreateBuffer(&paramsDesc, nullptr, &m_speedLinesBuffer);

	}


	virtual ~SpeedLines() override
	{
		DX_RELEASE(m_speedLinesBuffer);
	}

	void apply(ID3D11UnorderedAccessView* uav) const override
	{
		static const auto winSize = WindowsEngine::getInstance().getGraphics().getWinSize();

		ID3D11SamplerState* nullSampler = nullptr;
		ID3D11ShaderResourceView* nullSrv = nullptr;
		ID3D11Buffer* nullBuffer = nullptr;

		m_context.context->UpdateSubresource(m_speedLinesBuffer, 0, nullptr, &m_speedLinesParams, 0, 0);
		m_context.context->CSSetConstantBuffers(1, 1, &m_speedLinesBuffer);

		m_compute.bind({ uav });
		m_compute.dispatch(winSize.first / (8), winSize.second / (8), m_params.z);
		m_compute.unbind();


		m_context.context->CSSetShaderResources(0, 1, &nullSrv);
		m_context.context->CSSetSamplers(0, 1, &nullSampler);
		m_context.context->CSSetConstantBuffers(1, 1, &nullBuffer);
	}

	void update(float deltaTime) override
	{
		m_speedLinesParams.elapsedTime += deltaTime;
	}

	void setRadius(float val)	{ m_speedLinesParams.radius = val;	}
	void setOpacity(float val)	{ m_speedLinesParams.opacity = val;	}
	void setEdges (float val)	{ m_speedLinesParams.edges = val;	}

	void ImGuiControlWindow() override
	{
		ImGui::DragFloat("Radius##speedlines", &m_speedLinesParams.radius, 0.1f, 0, 20);
		ImGui::DragFloat("Edges", &m_speedLinesParams.edges, 0.05f, 0, 1);
	}


};
