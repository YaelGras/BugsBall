#pragma once
#include "abstraction/Camera.h"
#include "abstraction/ComputeShader.h"
#include "abstraction/Effect.h"
#include "abstraction/FrameBuffer.h"

class VFX
{
protected:
	bool                m_isEnabled;
	std::string         m_name;
	float m_elapsedTime;

	ComputeShader		m_compute;
	FrameBuffer* gBuffer = nullptr;
	d3d11_graphics::RenderingContext m_context;
	Camera m_sceneCam;

	struct DispatchParams
	{
		int x ; int y; int z ;
		DispatchParams(): x(64),y(32),z(1) {}
		DispatchParams(int a, int b, int c): x(a),y(b),z(c) {}
	} m_params;

	ID3D11Buffer* m_cameraBuffer = nullptr;
	struct cameraBuffer
	{
		XMVECTOR position;
		XMVECTOR range; // znear/zfar
		XMMATRIX viewProj; // znear/zfar
		XMMATRIX view; // znear/zfar
	} m_cameraParams;

public:
	// make this better
	VFX()
	{
		m_name = "N/A";
		m_context = WindowsEngine::getInstance().getGraphics().getContext();

		D3D11_BUFFER_DESC constantBufferDesc = {};
		constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		constantBufferDesc.ByteWidth = sizeof(cameraBuffer);
		constantBufferDesc.CPUAccessFlags = 0;

		m_context.device->CreateBuffer(&constantBufferDesc, nullptr, &m_cameraBuffer);

	}
	VFX(const VFX& other) = delete;
	VFX& operator=(const VFX& other) = delete;

	virtual ~VFX() {}

	explicit VFX(const std::string& path, const std::string& name = "N/A")

		: m_isEnabled(true)
		, m_name(name)
		, m_compute(path)
		, m_cameraParams{}
	{
		m_context = WindowsEngine::getInstance().getGraphics().getContext();

		D3D11_BUFFER_DESC constantBufferDesc = {};
		constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		constantBufferDesc.ByteWidth = sizeof(cameraBuffer);
		constantBufferDesc.CPUAccessFlags = 0;

		m_context.device->CreateBuffer(&constantBufferDesc, nullptr, &m_cameraBuffer);
	}

	// very bad
	virtual void setGbuffer(FrameBuffer* gbuffer) { gBuffer = gbuffer; }

	virtual void setCamera(const Camera& cam)
	{
		m_sceneCam = cam;
		m_cameraParams.position = cam.getPosition();
		auto proj = PerspectiveProjection{};
		m_cameraParams.range = { proj.znear, proj.zfar};
		m_cameraParams.viewProj = cam.getVPMatrix();
		m_cameraParams.view = cam.getViewMatrix();

		m_context.context->UpdateSubresource(m_cameraBuffer, 0, nullptr, &m_cameraParams, 0, 0);
	}

	virtual void apply(ID3D11UnorderedAccessView* uav) const 
	{
		m_compute.bind({ uav });
		m_compute.dispatch(m_params.x, m_params.y, m_params.z);
		m_compute.unbind();
	}

	bool isEnabled() const noexcept { return m_isEnabled; }
	void enable() noexcept { m_isEnabled = true; }
	void disable() noexcept { m_isEnabled = false; }

	virtual void update(float deltaTime)
	{
		m_elapsedTime += deltaTime;
	}

	virtual void showImguiDebugWindow() 
	{
		ImGui::SeparatorText(m_name.c_str());
		ImGui::Checkbox(std::string("Enable " + m_name).c_str(), &m_isEnabled);
		ImGuiControlWindow();
	}

	ID3D11ShaderResourceView* getSRVOfUAV(ID3D11UnorderedAccessView* uav) const
	{
		ID3D11ShaderResourceView* srvCopy = nullptr;
		ID3D11Resource* copy = nullptr;
		ID3D11Resource* originalRes = nullptr;
		ID3D11Texture2D* originalTex = nullptr;

		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		uav->GetDesc(&uavDesc);
		uav->GetResource(&originalRes);
		originalRes->QueryInterface(&originalTex);

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Format = uavDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;

		D3D11_TEXTURE2D_DESC desc;
		originalTex->GetDesc(&desc); //Correct data gets filled out

		D3D11_MAPPED_SUBRESOURCE mappedResource{};

		ID3D11Texture2D* tex = nullptr;

		m_context.device->CreateTexture2D(&desc, nullptr, &tex);
		m_context.context->CopyResource(tex, originalTex);
		m_context.device->CreateShaderResourceView(tex, &srvDesc, &srvCopy);
		DX_RELEASE(copy);
		DX_RELEASE(tex);
		DX_RELEASE(originalTex);
		DX_RELEASE(originalRes);
		return srvCopy;
	}

protected:
	virtual void ImGuiControlWindow() {}


};
