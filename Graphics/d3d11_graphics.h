#pragma once

#include <d3d11.h>
#include <utility>

#include "imgui.h"
#include "imgui_internal.h"
#include "abstraction/DepthBuffer.h"


enum GRAPHICS_MODE { WINDOWED, FULLSCREEN };

class d3d11_graphics {

public:

	struct RenderingContext
	{
		ID3D11Device* device = nullptr;
		ID3D11DeviceContext* context = nullptr;
		IDXGISwapChain* swapChain = nullptr;
		ID3D11RenderTargetView* rtv = nullptr;

		// Post process
		ID3D11Texture2D* postProcessTexture = nullptr;
		ID3D11RenderTargetView* postProcessRTV = nullptr;
		ID3D11UnorderedAccessView* postProcessUAV = nullptr;
		ID3D11ShaderResourceView* postProcessSRV = nullptr;
	};

	RenderingContext& getContext()  { return m_context; }


private:

	DepthBuffer m_depth;
	RenderingContext m_context;
	int m_width{}, m_height{};
	GRAPHICS_MODE m_cdsMode = GRAPHICS_MODE::FULLSCREEN;
	bool m_handleResize = false;
	ID3D11BlendState* m_blendState = nullptr;

public:

	///////////////////////////////////////////////////////////////////////////
	/// -- Constructors

	d3d11_graphics(HWND hWnd, GRAPHICS_MODE mode = GRAPHICS_MODE::FULLSCREEN);
	d3d11_graphics(const d3d11_graphics&) = delete;
	d3d11_graphics& operator=(const d3d11_graphics&) = delete;
	~d3d11_graphics();

	///////////////////////////////////////////////////////////////////////////
	/// -- Basic operations

	void clearDepth() const noexcept { m_depth.clearDepth(); }
	void clearFramebuffer() const noexcept;
	void present()const noexcept;
	void bindBlendState() const noexcept
	{
		m_context.context->OMSetBlendState(m_blendState, nullptr, 0xffffffff);
	}
	void setDefaultViewport();
	void setBackfaceCull(bool cullBackfaces = true);
	///////////////////////////////////////////////////////////////////////////
	/// -- Getters

	const DepthBuffer &getDepthBuffer()				const noexcept { return m_depth; }
	ID3D11Device* getDevice()						const noexcept { return m_context.device; }
	IDXGISwapChain* getSwapChain()					const noexcept { return m_context.swapChain; }
	ID3D11DeviceContext* getImmediateContext()		const noexcept { return m_context.context; }
	ID3D11RenderTargetView* getRenderTargetView()	const noexcept { return m_context.rtv; }
	std::pair<int, int> getWinSize()				const noexcept { return { m_width, m_height }; }
	void setWinSize(int w, int h)
	{
		m_context.context->OMSetRenderTargets(0, NULL, NULL);
		m_context.context->ClearState();
		m_width = w;
		m_height = h;

		DXGI_MODE_DESC newDesc;
		newDesc.Width = w;
		newDesc.Height = h;
		newDesc.RefreshRate.Numerator = 60;
		newDesc.RefreshRate.Denominator = 1;

		m_context.swapChain->ResizeBuffers(0, 0,0, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);

		ID3D11Texture2D* pBackBuffer = nullptr;
		m_context.rtv->Release();
		m_context.swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
		m_context.device->CreateRenderTargetView(pBackBuffer, nullptr, &m_context.rtv);
		pBackBuffer->Release();


		m_depth = DepthBuffer{ m_context.device, m_context.context };
		m_depth.initDepthBuffer(w, h);
		m_depth.bind();

		m_context.context->OMSetRenderTargets(1, &m_context.rtv, m_depth.getView());


		m_context.swapChain->ResizeTarget(&newDesc);
		m_handleResize = true;

		setDefaultViewport();

	}
	bool hasResized() const noexcept { return m_handleResize; }
	void resizeHasBeenHandled() { m_handleResize = true; }
};