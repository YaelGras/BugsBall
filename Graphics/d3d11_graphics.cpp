#include "d3d11_graphics.h"

#include <d3d11.h>
#include "../Utils/Debug.h"

d3d11_graphics::d3d11_graphics(HWND hWnd, GRAPHICS_MODE mode)
{
	m_cdsMode = mode;

	UINT deviceFlags{};
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_11_1
	};

#if _DEBUG
	deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif;
	int width = 0;
	int height = 0;
	;		// Setup the descriptor
	DXGI_SWAP_CHAIN_DESC sd{};
	ZeroMemory(&sd, sizeof(sd));
	switch (m_cdsMode)
	{
	case GRAPHICS_MODE::WINDOWED:
		{
			
			RECT rc;
			GetClientRect(hWnd, &rc);
			width = rc.right - rc.left;
			height = rc.bottom - rc.top;
			sd.BufferCount = 1; // 1 front+ 1 back
			sd.BufferDesc.Width = width;
			sd.BufferDesc.Height = height;
			sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			sd.BufferDesc.RefreshRate.Numerator = 60;
			sd.BufferDesc.RefreshRate.Denominator = 1;
			sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // render output
			sd.OutputWindow = hWnd;
			sd.SampleDesc.Count = 1; // AA ...
			sd.SampleDesc.Quality = 0;
			sd.Windowed = TRUE;
			sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
			break;
		}
	case GRAPHICS_MODE::FULLSCREEN:
		{
			

			RECT rc;
			GetClientRect(hWnd, &rc);
			width = rc.right - rc.left;
			height = rc.bottom - rc.top;
			sd.BufferCount = 1; // 1 front+ 1 back
			sd.BufferDesc.Width = width;
			sd.BufferDesc.Height = height;
			sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			sd.BufferDesc.RefreshRate.Numerator = 60;
			sd.BufferDesc.RefreshRate.Denominator = 1;
			sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // render output
			sd.OutputWindow = hWnd;
			sd.SampleDesc.Count = 1; // AA ...
			sd.SampleDesc.Quality = 0;
			sd.Windowed = FALSE;
			sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
			break;
		}
	}
	m_width = width;
	m_height = height;

	RECT rcClient, rcWindow;
	POINT ptDiff; GetClientRect(hWnd, &rcClient);
	GetWindowRect(hWnd, &rcWindow);
	ptDiff.x = (rcWindow.right - rcWindow.left) - rcClient.right;
	ptDiff.y = (rcWindow.bottom - rcWindow.top) - rcClient.bottom;

	MoveWindow( hWnd, rcWindow.left, rcWindow.top, width + ptDiff.x, height + ptDiff.y, TRUE);


	DX_TRY_CODE(D3D11CreateDeviceAndSwapChain
	(
		nullptr, // adapter
		D3D_DRIVER_TYPE_HARDWARE, // driver type
		nullptr, // software
		deviceFlags,
		featureLevels,
		ARRAYSIZE(featureLevels),
		D3D11_SDK_VERSION,
		&sd, // descriptor
		&m_context.swapChain,
		&m_context.device,
		nullptr, // output, fills with the actual flags (useless)
		&m_context.context
	), FAIL);

	m_context.swapChain->SetFullscreenState(FALSE, nullptr);

	///////////////////////////////////////////////////////////////// BackBuffer

	ID3D11Texture2D* pBackBuffer = nullptr;


	DX_TRY(m_context.swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer));
	DX_TRY(m_context.device->CreateRenderTargetView(pBackBuffer, nullptr, &m_context.rtv));
	DX_RELEASE(pBackBuffer);

	///////////////////////////////////////////////////////////////// Post process
	///
	///
	DXGI_FORMAT textureFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	uavDesc.Format = textureFormat;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Format = textureFormat;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	shaderResourceViewDesc.Format = textureFormat;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;

	D3D11_TEXTURE2D_DESC vfxTextureDesc = {};
	vfxTextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_RENDER_TARGET;
	vfxTextureDesc.Usage = D3D11_USAGE_DEFAULT ;
	vfxTextureDesc.Width = width;
	vfxTextureDesc.Height = height;
	vfxTextureDesc.Format = textureFormat;
	vfxTextureDesc.ArraySize = 1;
	vfxTextureDesc.MipLevels = 1;
	vfxTextureDesc.SampleDesc.Count = 1;


	m_context.device->CreateTexture2D(&vfxTextureDesc, nullptr, &m_context.postProcessTexture);
	DX_TRY(m_context.device->CreateRenderTargetView(m_context.postProcessTexture, &rtvDesc, &m_context.postProcessRTV));
	DX_TRY(m_context.device->CreateShaderResourceView(m_context.postProcessTexture, &shaderResourceViewDesc, &m_context.postProcessSRV));
	DX_TRY(m_context.device->CreateUnorderedAccessView(m_context.postProcessTexture, &uavDesc, &m_context.postProcessUAV));

	///////////////////////////////////////////////////////////////// DEPTH BUFFER

	m_depth = DepthBuffer{m_context.device, m_context.context};
	m_depth.initDepthBuffer(width, height);
	m_depth.bind();

	m_context.context->OMSetRenderTargets(1, &m_context.rtv, m_depth.getView());

	///////////////////////////////////////////////////////////////// ViewPort

	D3D11_VIEWPORT vp{};
	vp.Width = (FLOAT)(m_width);
	vp.Height = (FLOAT)(m_height);
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;

	m_context.context->RSSetViewports(1, &vp);

	///////////////////////////////////////////////////////////////// Rasterizer

	setBackfaceCull(true);

	///////////////////////////////////////////////////////////////// Blending
	D3D11_BLEND_DESC blendDesc{};
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	m_context.device->CreateBlendState(&blendDesc, &m_blendState);
	m_context.context->OMSetBlendState(m_blendState, nullptr, 0xffffffff);

}

d3d11_graphics::~d3d11_graphics() {

#ifdef _DEBUG
	ID3D11Debug* debugDev = nullptr;
	m_context.device->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&debugDev));
	debugDev->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
	DX_RELEASE(debugDev);
#endif

	DX_RELEASE(m_context.postProcessUAV);
	DX_RELEASE(m_context.rtv);
	DX_RELEASE(m_context.context);
	DX_RELEASE(m_context.swapChain);
	DX_RELEASE(m_context.device);
}

void d3d11_graphics::clearFramebuffer() const noexcept
{
	static constexpr FLOAT rgba[4] = { 0,0,0,1 };
	m_context.context->ClearRenderTargetView(m_context.rtv, rgba);
	m_context.context->ClearUnorderedAccessViewFloat(m_context.postProcessUAV, rgba);

}

void d3d11_graphics::present() const noexcept
{
	m_context.swapChain->Present(1, 0);
}

void d3d11_graphics::setDefaultViewport()
{
	D3D11_VIEWPORT vp{};
	vp.Width = (FLOAT)(m_width);
	vp.Height = (FLOAT)(m_height);
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;

	m_context.context->RSSetViewports(1, &vp);
}

void d3d11_graphics::setBackfaceCull(bool cullBackfaces/*=true*/)
{
	// no release cause idk
	static ID3D11RasterizerState* cullState = nullptr;
	static ID3D11RasterizerState* noCullState = nullptr;
	if (!cullState || !noCullState)
	{		
		D3D11_RASTERIZER_DESC descCull, descNoCull;
		ZeroMemory(&descCull, sizeof(descCull));
		ZeroMemory(&descNoCull, sizeof(descNoCull));
		descCull.FillMode = descNoCull.FillMode = D3D11_FILL_SOLID;
		descCull.CullMode = D3D11_CULL_BACK;
		descNoCull.CullMode = D3D11_CULL_NONE;
		m_context.device->CreateRasterizerState(&descNoCull, &noCullState);
		m_context.device->CreateRasterizerState(&descCull, &cullState);
	}

	(cullBackfaces) ?
		m_context.context->RSSetState(cullState) :
		m_context.context->RSSetState(noCullState);
}