#pragma once
#include "VFX.h"


class TiltShift : public VFX
{
private:
	struct tiltShiftParams
	{
		int sampleCount = 8;
		float blurStrength  = .6F;
		float radius  = .2F;
		float pad;

	} m_tilt_shift_params;
	static_assert(sizeof(tiltShiftParams) % 16 == 0);
	ID3D11Buffer* m_paramsBuffers = nullptr;

	ComputeShader m_gaussianBlur{"res/compute/gaussianBlur.hlsl"};
public:

	TiltShift()
		: VFX("res/compute/tiltshift.hlsl", "TiltShift")
		, m_tilt_shift_params{}
	{


		D3D11_BUFFER_DESC paramsBuffersDesc = {};
		paramsBuffersDesc.Usage = D3D11_USAGE_DEFAULT;
		paramsBuffersDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		paramsBuffersDesc.ByteWidth = sizeof(tiltShiftParams);
		paramsBuffersDesc.CPUAccessFlags = 0;

		m_context.device->CreateBuffer(&paramsBuffersDesc, nullptr, &m_paramsBuffers);


	}

	virtual ~TiltShift() override
	{
		DX_RELEASE(m_paramsBuffers);
	}

	virtual void apply(ID3D11UnorderedAccessView* uav) const override
	{

		static const auto winSize = WindowsEngine::getInstance().getGraphics().getWinSize();

		ID3D11ShaderResourceView* nullSrv = nullptr;
		ID3D11ShaderResourceView* srvCopy = nullptr;
		ID3D11ShaderResourceView* blurSrv = nullptr;
		ID3D11Buffer* nullBuffer = nullptr;

		////////////////////////////////////////////////////////////////////////////////////////////////
		
		srvCopy = getSRVOfUAV(uav);
		m_context.context->UpdateSubresource(m_paramsBuffers, 0, nullptr, &m_tilt_shift_params, 0, 0);

		////////////////////////////////////////////////////////////////////////////////////////////////

		m_compute.bind({ uav });

		if (gBuffer)
		{
				
			ID3D11ShaderResourceView* worldPosSRV = gBuffer->getResource(2);
			m_context.context->CSSetShaderResources(0, 1, &worldPosSRV);
			m_context.context->CSSetShaderResources(1, 1, &srvCopy);

		}
		m_context.context->CSSetConstantBuffers(0, 1, &m_cameraBuffer);
		m_context.context->CSSetConstantBuffers(1, 1, &m_paramsBuffers);
		m_compute.dispatch(
			static_cast<int>(ceil(winSize.first / 8u)), 
			static_cast<int>(ceil(winSize.second / 4u)), 
			static_cast<int>(m_params.z));

		m_context.context->CSSetConstantBuffers(0, 1, &nullBuffer);
		m_context.context->CSSetShaderResources(0, 1,&nullSrv);		 
		m_compute.unbind();
		
		DX_RELEASE(srvCopy);
	}



protected:
		virtual void ImGuiControlWindow() override
		{

			ImGui::DragInt("Blur sample count", & m_tilt_shift_params.sampleCount,1,0, 30);
			ImGui::DragFloat("Blur strength", &m_tilt_shift_params.blurStrength, 0.2f, 0, 20);
			ImGui::DragFloat("Radius", &m_tilt_shift_params.radius, 0.05f, 0, 1);

		}

};
