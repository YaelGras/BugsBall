#pragma once

#include "VFX.h"


class RadialBlur : public VFX
{

private:

	struct radialBlurParams
	{

		XMVECTOR m_intensity;
		XMVECTOR m_sampleCount;
		XMVECTOR m_radius;
		XMVECTOR m_centerPoint;

	} m_blurParams ;
	static_assert(sizeof(radialBlurParams) % 16 == 0);

	ID3D11Buffer* m_radialBlurConstantBuffer = nullptr;
	ID3D11SamplerState* m_sampler = nullptr;

public:

	virtual ~RadialBlur() override
	{
		DX_RELEASE(m_radialBlurConstantBuffer);
		DX_RELEASE(m_sampler);
	}

	RadialBlur()
		: VFX("res/compute/radialblur.hlsl", "Radial blur")
	{
		m_blurParams.m_intensity = { 0.F };
		m_blurParams.m_sampleCount = { 6.F };
		m_blurParams.m_radius = { 0.5F };
		m_blurParams.m_centerPoint = {0.5f,0.5f};

		D3D11_BUFFER_DESC paramsDesc = {};
		paramsDesc.Usage = D3D11_USAGE_DEFAULT;
		paramsDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		paramsDesc.ByteWidth = sizeof(radialBlurParams);
		paramsDesc.CPUAccessFlags = 0;
		m_context.device->CreateBuffer(&paramsDesc, nullptr, &m_radialBlurConstantBuffer);

		D3D11_SAMPLER_DESC samplerDesc{};
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.MipLODBias = 0.0f;
		samplerDesc.MaxAnisotropy = 1;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		samplerDesc.BorderColor[0] = 0;
		samplerDesc.BorderColor[1] = 0;
		samplerDesc.BorderColor[2] = 0;
		samplerDesc.BorderColor[3] = 0;
		samplerDesc.MinLOD = 0;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

		m_context.device->CreateSamplerState(&samplerDesc, &m_sampler);

	}

	virtual void apply(ID3D11UnorderedAccessView* uav) const override
	{
		static const auto winSize = WindowsEngine::getInstance().getGraphics().getWinSize();
		ID3D11ShaderResourceView* srvCopy = nullptr;
		ID3D11SamplerState* nullSampler = nullptr;
		ID3D11ShaderResourceView* nullSrv = nullptr;
		ID3D11Buffer* nullBuffer = nullptr;

		m_context.context->CSSetSamplers(0, 1, &m_sampler);

		srvCopy = getSRVOfUAV(uav);

		m_context.context->UpdateSubresource(m_radialBlurConstantBuffer, 0, nullptr, &m_blurParams, 0, 0);
		m_context.context->CSSetConstantBuffers(1, 1, &m_radialBlurConstantBuffer);

		m_context.context->CSSetShaderResources(0, 1, &srvCopy);

		m_compute.bind({ uav });

		m_compute.dispatch(
			static_cast<int>(ceil(winSize.first / (8))),
			static_cast<int>(ceil(winSize.second / (8))),
			static_cast<int>(ceil(m_params.z))); 
		m_compute.unbind();


		DX_RELEASE(srvCopy);

		m_context.context->CSSetShaderResources(0, 1, &nullSrv);
		m_context.context->CSSetSamplers(0, 1, &nullSampler);
		m_context.context->CSSetConstantBuffers(1, 1, &nullBuffer);


	}

	void setIntensity(float value)
	{
		m_blurParams.m_intensity = { value,value ,value ,value };
	}


	virtual void ImGuiControlWindow() override
	{
		ImGui::DragFloat("Intensity", &m_blurParams.m_intensity.vector4_f32[0], 0.05f, 0, 1);
		ImGui::DragFloat("Radius##radial", &m_blurParams.m_radius.vector4_f32[0], 0.01f, 0, 1);
		ImGui::DragFloat("Sample Count", &m_blurParams.m_sampleCount.vector4_f32[0], 1, 0, 10);
		ImGui::DragFloat2("CenterPoint", &m_blurParams.m_centerPoint.vector4_f32[0], 0.05f, 0, 1.f);

	}


};
