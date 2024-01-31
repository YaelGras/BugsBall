#include "FrameBuffer.h"
#include "DepthBuffer.h"
#include <d3d11.h>

#include "FrameBufferStack.h"



#pragma warning(suppress : 6387) // removes useless intellisense warning



FrameBuffer::FrameBuffer(int texCount/*=1*/, int width, int height)
	: m_texCount(texCount)
	{

#ifdef D3D11_IMPL
		auto& gfx = WindowsEngine::getInstance().getGraphics();
		m_renderContext = WindowsEngine::getInstance().getGraphics().getContext();

		// Get current fbo desc
		D3D11_TEXTURE2D_DESC dsc{};
		ID3D11Texture2D* bb = nullptr;
		m_renderContext.swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<LPVOID*>(&bb));
		bb->GetDesc(&dsc);
		bb->Release();

		m_width = width;
		m_height = height;
		if (width == -1 || height == -1)
		{
			
			m_width = dsc.Width;
			m_height = dsc.Height;
		} 
		dsc.Width = m_width;
		dsc.Height = m_height;
		dsc.ArraySize = 1;
		dsc.BindFlags |= D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		dsc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;


		ID3D11Texture2D** textureArray = new ID3D11Texture2D * [m_texCount];
		ID3D11Texture2D* unlitTexture = nullptr;

		for (int i = 0; i < m_texCount; ++i)
			m_renderContext.device->CreateTexture2D(&dsc, nullptr, &textureArray[i]);

		m_renderContext.device->CreateTexture2D(&dsc, nullptr, &unlitTexture);



		m_depth = DepthBuffer{ m_renderContext.device, m_renderContext.context };
		m_depth.initDepthBuffer(m_width, m_height);


		D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc32{};
		renderTargetViewDesc32.Format = dsc.Format;
		renderTargetViewDesc32.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		renderTargetViewDesc32.Texture2D.MipSlice = 0;

		m_rtv.resize(m_texCount);
		m_srv.resize(m_texCount);
		for (int i = 0; i < m_texCount; ++i)
			m_renderContext.device->CreateRenderTargetView(textureArray[i], &renderTargetViewDesc32, &m_rtv[i]);

		m_renderContext.device->CreateRenderTargetView(unlitTexture, &renderTargetViewDesc32, &m_unlitSrv.first);

		//Shader Resource View Description
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc{};
		shaderResourceViewDesc.Format = dsc.Format;
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		shaderResourceViewDesc.Texture2D.MipLevels = 1;



		for (int i = 0; i < m_texCount; i++)
		{
			m_renderContext.device->CreateShaderResourceView(textureArray[i], &shaderResourceViewDesc, &m_srv[i]);
		}
		m_renderContext.device->CreateShaderResourceView(unlitTexture, &shaderResourceViewDesc, &m_unlitSrv.second);
		for (int i = 0; i < m_texCount; ++i)
			textureArray[i]->Release();

		unlitTexture->Release();
		delete[] textureArray;

#endif
	}

	void FrameBuffer::clear() const noexcept
	{
		m_depth.clearDepth();
		const float color[4] = { 0,0,0,1 };
		for (int i = 0; i < m_texCount; i++)
		{
			m_renderContext.context->ClearRenderTargetView(m_rtv[i], color);
		}
		m_renderContext.context->ClearRenderTargetView(m_unlitSrv.first, color);


	}

	void FrameBuffer::bind() const noexcept
	{
#ifdef D3D11_IMPL
		m_depth.bind();
		m_renderContext.context->OMSetRenderTargets(m_texCount, m_rtv.data(), m_depth.getView());
		FrameBufferStack::getInstance().pushFBO(this);
#endif

	}

	void FrameBuffer::bindViewOnly() const noexcept
	{
#ifdef D3D11_IMPL
		m_depth.bind();
		m_renderContext.context->OMSetRenderTargets(m_texCount, m_rtv.data(), nullptr);
		FrameBufferStack::getInstance().pushFBO(this);
#endif

	}


void FrameBuffer::bindDepthOnly() const noexcept
	{
#ifdef D3D11_IMPL

		D3D11_VIEWPORT vp{};
		vp.Width = (FLOAT)(m_width);
		vp.Height = (FLOAT)(m_height);
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;

		m_renderContext.context->RSSetViewports(1, &vp);

		m_depth.bind();
		m_renderContext.context->OMSetRenderTargets(0, nullptr, m_depth.getView());
		FrameBufferStack::getInstance().pushFBO(this);
#endif

	}


	void FrameBuffer::bindCached() const noexcept
	{
#ifdef D3D11_IMPL
		m_depth.bind();
		m_renderContext.context->OMSetRenderTargets(m_texCount, m_rtv.data(), m_depth.getView()); // might not be what i need
#endif

	}

	ID3D11ShaderResourceView* FrameBuffer::bindUnlitRTV() const
	{
		m_renderContext.context->OMSetRenderTargets(1, &m_unlitSrv.first, m_depth.getView());
		return m_unlitSrv.second;
	}

	void FrameBuffer::unbind()
	{

		FrameBufferStack::getInstance().popFBO();

	}

