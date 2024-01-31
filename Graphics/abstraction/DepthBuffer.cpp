#include "DepthBuffer.h"

#include <d3d11.h>
#include "../Utils/Debug.h"

#pragma warning(suppress : 6387) // removes useless intellisense warning


DepthBuffer::DepthBuffer(ID3D11Device* _device, ID3D11DeviceContext* _context)
{
	device = _device;
	context = _context;
}

DepthBuffer::~DepthBuffer()
{
	DX_RELEASE(m_state)
	DX_RELEASE(m_depthStencil)
	DX_RELEASE(m_depthTexture)
}

void DepthBuffer::swap(DepthBuffer& other) noexcept
{
	std::swap(m_depthTexture, other.m_depthTexture);
	std::swap(m_depthStencil, other.m_depthStencil);
	std::swap(m_state, other.m_state);
	std::swap(device, other.device);
	std::swap(context, other.context);
}

DepthBuffer::DepthBuffer(DepthBuffer&& other) noexcept
	: m_depthTexture(std::exchange(other.m_depthTexture, nullptr))
	, m_depthStencil(std::exchange(other.m_depthStencil, nullptr))
	, m_state(std::exchange(other.m_state, nullptr))
	, device(std::exchange(other.device, nullptr))
	, context(std::exchange(other.context, nullptr))
{}

DepthBuffer& DepthBuffer::operator=(DepthBuffer&& other) noexcept
{
	DepthBuffer{ std::move(other) }.swap(*this);
	return *this;

}

void DepthBuffer::initDepthBuffer(int width, int height)
{

	D3D11_TEXTURE2D_DESC depthTextureDesc;
	ZeroMemory(&depthTextureDesc, sizeof(depthTextureDesc));
	depthTextureDesc.Width = width;
	depthTextureDesc.Height = height;
	depthTextureDesc.MipLevels = 1;
	depthTextureDesc.ArraySize = 1;

	depthTextureDesc.Format = DXGI_FORMAT_R32_TYPELESS;

	depthTextureDesc.SampleDesc.Count = 1;
	depthTextureDesc.SampleDesc.Quality = 0;
	depthTextureDesc.Usage = D3D11_USAGE_DEFAULT ;
	depthTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	depthTextureDesc.CPUAccessFlags = 0;
	depthTextureDesc.MiscFlags = 0;

	device->CreateTexture2D(&depthTextureDesc, nullptr, &m_depthTexture);

	D3D11_DEPTH_STENCIL_DESC dsDesc;

	// Depth test parameters
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

	// Stencil test parameters
	dsDesc.StencilEnable = true;
	dsDesc.StencilReadMask = 0xFF;
	dsDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing
	dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing
	dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Create depth stencil state

	device->CreateDepthStencilState(&dsDesc, &m_state);

	D3D11_DEPTH_STENCIL_VIEW_DESC descDSView;
	ZeroMemory(&descDSView, sizeof(descDSView));
	descDSView.Format = DXGI_FORMAT_D32_FLOAT;
	descDSView.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSView.Texture2D.MipSlice = 0;


	device->CreateDepthStencilView(m_depthTexture, &descDSView, &m_depthStencil);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;

	device->CreateShaderResourceView(m_depthTexture, &srvDesc, &m_srv);


}

void DepthBuffer::bind() const noexcept
{
	context->OMSetDepthStencilState(m_state, 1);
}

void DepthBuffer::clearDepth() const noexcept {
	if (m_depthStencil)
		context->ClearDepthStencilView(m_depthStencil, D3D11_CLEAR_DEPTH, 1.0f, 0);
}
