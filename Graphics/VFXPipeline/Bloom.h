#pragma once

#include "VFX.h"

class Bloom : public VFX
{
private:

	const int MIP_COUNT = 7;
	float m_filterRadius;
	float m_exposure;
	float m_bloomStrength;

	ComputeShader m_upSampling{"res/compute/upsampling.hlsl"};
	ComputeShader m_downSampling{"res/compute/downsampling.hlsl"};
	ComputeShader m_finalPass{"res/compute/final_bloom.hlsl"};

	ID3D11UnorderedAccessView* m_uav = nullptr;

	ID3D11Buffer* m_bloomConstantBuffer = nullptr;
	ID3D11SamplerState* m_sampler = nullptr;

	struct BloomMip
	{
		int res_x;
		int res_y;
		ID3D11UnorderedAccessView* texture_uav = nullptr;
		ID3D11ShaderResourceView* texture_srv = nullptr;

		BloomMip(int w, int h, d3d11_graphics::RenderingContext& renderContext,
		         D3D11_UNORDERED_ACCESS_VIEW_DESC mainUavDesc)
		{
			res_x = w;
			res_y = h;

			constexpr static uint32_t s_pixel = 0xffffffff;

			D3D11_SUBRESOURCE_DATA initData = {&s_pixel, sizeof(uint32_t), 0};

			D3D11_TEXTURE2D_DESC desc = {};
			desc.Width = w;
			desc.Height = h;
			desc.MipLevels = desc.ArraySize = 1;
			desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
			desc.SampleDesc.Count = 1;
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_RENDER_TARGET;

			ID3D11Texture2D* tex;
			HRESULT hr = renderContext.device->CreateTexture2D(&desc, &initData, &tex);

			D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
			uavDesc.Format = desc.Format;
			uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
			uavDesc.Texture2D.MipSlice = 0;

			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
			srvDesc.Format = uavDesc.Format;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MipLevels = 1;

			renderContext.device->CreateShaderResourceView(tex, &srvDesc, &texture_srv);
			renderContext.device->CreateUnorderedAccessView(tex, &uavDesc, &texture_uav);

			DX_RELEASE(tex);
		}
	};

	std::vector<BloomMip> m_mips;

private:

public:
	Bloom(ID3D11UnorderedAccessView* baseUav) : VFX{}
	{
		m_filterRadius = 0.005f;
		m_exposure = 1.13f;
		m_bloomStrength = 0.070f;
		m_name = "Bloom";

		D3D11_UNORDERED_ACCESS_VIEW_DESC baseDesc;
		D3D11_TEXTURE2D_DESC originalTexDesc;
		ID3D11Texture2D* originalTex = nullptr;
		ID3D11Resource* originalRes = nullptr;

		baseUav->GetDesc(&baseDesc);
		baseUav->GetResource(&originalRes);
		originalRes->QueryInterface(&originalTex);
		originalTex->GetDesc(&originalTexDesc);

		const int width = originalTexDesc.Width;
		const int height = originalTexDesc.Height;

		for (int i = 0; i < MIP_COUNT; i++)
		{
			double coef = std::pow(2, i + 1);

			auto bm = BloomMip{
				static_cast<int>(width / coef),
				static_cast<int>(height / coef),
				m_context, baseDesc
			};

			m_mips.push_back(bm);
		}

		DX_RELEASE(originalTex)
		DX_RELEASE(originalRes)

		D3D11_BUFFER_DESC firstPassBufferDesc = {};
		firstPassBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		firstPassBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		firstPassBufferDesc.ByteWidth = sizeof(XMVECTOR);
		firstPassBufferDesc.CPUAccessFlags = 0;
		m_context.device->CreateBuffer(&firstPassBufferDesc, nullptr, &m_bloomConstantBuffer);

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


		m_isEnabled = true;
	}


	void downSample(ID3D11UnorderedAccessView* uav) const
	{
		XMVECTOR firstPass = XMVECTOR{ {1.5f, 0, 0, 0} };
		ID3D11ShaderResourceView* srvCopy = nullptr;

		ID3D11SamplerState* nullSampler = nullptr;
		ID3D11RenderTargetView* nullRTV = nullptr;
		ID3D11ShaderResourceView* nullSrv = nullptr;
		ID3D11Buffer* nullBuffer = nullptr;

		static const auto winSize = WindowsEngine::getInstance().getGraphics().getWinSize();
		m_context.context->OMSetRenderTargets(1, &nullRTV, nullptr);


		m_context.context->CSSetSamplers(0, 1, &m_sampler);
		for (int bloomPass = 0; bloomPass < MIP_COUNT; bloomPass++)
		{

			if (bloomPass == 0)
			{
				srvCopy = getSRVOfUAV(uav);
				m_context.context->CSSetShaderResources(0, 1, &srvCopy);
				firstPass = {1.5f, winSize.first / 8.f, winSize.second / 4.f, 0};
			}
			else
			{
				m_context.context->CSSetShaderResources(0, 1, &m_mips[bloomPass - 1].texture_srv);

				firstPass = XMVECTOR{{1.5f,	m_mips[bloomPass - 1].res_y / 4.F,	m_mips[bloomPass - 1].res_x / 8.f,	0}};
			}

			// Update first pass stuff
			m_context.context->UpdateSubresource(m_bloomConstantBuffer, 0, nullptr, &firstPass, 0, 0);
			m_context.context->CSSetConstantBuffers(1, 1, &m_bloomConstantBuffer);
			firstPass -= XMVECTOR{ {1, 0, 0, 0} };

			// Bind and compute dispatch params
			m_downSampling.bind({m_mips[bloomPass].texture_uav});
			m_downSampling.dispatch(
				static_cast<int>(ceil(winSize.first / (8 * (std::pow(2, bloomPass + 1))))),
				static_cast<int>(ceil(winSize.second / (8 * (std::pow(2, bloomPass + 1))))),
				m_params.z);

			m_downSampling.unbind();
			DX_RELEASE(srvCopy)
			m_context.context->CSSetShaderResources(0, 1, &nullSrv);
		}

		m_context.context->CSSetShaderResources(0, 1, &nullSrv);
		m_context.context->CSSetSamplers(0, 1, &nullSampler);
		m_context.context->CSSetConstantBuffers(1, 1, &nullBuffer);
	}

	void upSample() const
	{
		static const auto winSize = WindowsEngine::getInstance().getGraphics().getWinSize();
		ID3D11SamplerState* nullSampler = nullptr;
		ID3D11ShaderResourceView* nullSrv = nullptr;
		ID3D11Buffer* nullBuffer = nullptr;
		XMVECTOR radius = XMVECTOR{ { m_filterRadius, m_filterRadius, m_filterRadius, m_filterRadius} };
		// m_bloomConstantBuffer holds filter radius
		m_context.context->UpdateSubresource(m_bloomConstantBuffer, 0, nullptr, &radius, 0, 0);
		m_context.context->CSSetConstantBuffers(1, 1, &m_bloomConstantBuffer);
		m_context.context->CSSetSamplers(0, 1, &m_sampler);


		for (size_t bloomPass = m_mips.size()-1; bloomPass > 0; bloomPass--)
		{
			const BloomMip& mip = m_mips[bloomPass];
			const BloomMip& nextMip = m_mips[bloomPass - 1];


			m_context.context->CSSetShaderResources(0, 1, &mip.texture_srv);

			m_upSampling.bind({nextMip.texture_uav});


			m_upSampling.dispatch(
				static_cast<int>(ceil(winSize.first / (8 * (std::pow(2, bloomPass - 1))))),
				static_cast<int>(ceil(winSize.second / (8 * (std::pow(2, bloomPass - 1))))),
				m_params.z);

			m_upSampling.unbind();
		}

		m_context.context->CSSetShaderResources(0, 1, &nullSrv);
		m_context.context->CSSetSamplers(0, 1, &nullSampler);
		m_context.context->CSSetConstantBuffers(1, 1, &nullBuffer);
	}


	void combineSourceAndLastMip(ID3D11UnorderedAccessView* uav) const
	{
		// m_bloomConstantBuffer holds {strength, exposure, 0,0};

		ID3D11SamplerState* nullSampler = nullptr;
		ID3D11ShaderResourceView* nullSrv = nullptr;
		ID3D11Buffer* nullBuffer = nullptr;

		const XMVECTOR finalBloomParams = XMVECTOR{{ m_bloomStrength, m_exposure}};
		m_context.context->UpdateSubresource(m_bloomConstantBuffer, 0, nullptr, &finalBloomParams, 0, 0);
		m_context.context->CSSetConstantBuffers(1, 1, &m_bloomConstantBuffer);
		m_context.context->CSSetShaderResources(0, 1, &m_mips[0].texture_srv);
		m_context.context->CSSetSamplers(0, 1, &m_sampler);

		m_finalPass.bind({uav});
		m_finalPass.dispatch(m_params.x, m_params.y, m_params.z);
		m_finalPass.unbind();

		m_context.context->CSSetShaderResources(0, 1, &nullSrv);
		m_context.context->CSSetSamplers(0, 1, &nullSampler);
		m_context.context->CSSetConstantBuffers(1, 1, &nullBuffer);
	}

	void apply(ID3D11UnorderedAccessView* uav) const override
	{
		downSample(uav);
		upSample();
		combineSourceAndLastMip(uav);
	}


	void ImGuiControlWindow() override
	{
		ImGui::DragFloat("Filter Radius", &m_filterRadius, 0.01f, 0, 2);
		ImGui::DragFloat("Bloom Strength", &m_bloomStrength, 0.01f, 0, 2);
		ImGui::DragFloat("ACES Exposure", &m_exposure, 0.01f, 0, 2);
		ImGui::Image(m_mips[0].texture_srv, ImVec2(311, 173));
	}
};
