#pragma once
#pragma warning(suppress : 6387) // removes useless intellisense warning

#include <filesystem>
#include <utility>

#include "../../Platform/WindowsEngine.h"

class DepthBuffer;
struct ID3D11RenderTargetView;
struct ID3D11ShaderResourceView;

class FrameBuffer 

{

private:

	int m_texCount;
	int m_width, m_height;

	DepthBuffer m_depth;

	std::vector<ID3D11RenderTargetView*> m_rtv;
	std::vector<ID3D11ShaderResourceView*> m_srv;

	std::pair<ID3D11RenderTargetView*, ID3D11ShaderResourceView*> m_unlitSrv{nullptr, nullptr};
	
#ifdef D3D11_IMPL
	d3d11_graphics::RenderingContext m_renderContext;
#endif

public:
	FrameBuffer(int texCount = 1, int width = -1, int height = -1);
	~FrameBuffer() {
		DX_RELEASE(m_unlitSrv.first);
		DX_RELEASE(m_unlitSrv.second);
		std::ranges::for_each(m_srv, [](auto&& r) {DX_RELEASE(r)});
		std::ranges::for_each(m_rtv, [](auto&& r) {DX_RELEASE(r)});
	}


	void clear() const noexcept;
	void bind() const noexcept;
	void bindViewOnly() const noexcept;
	void bindDepthOnly() const noexcept;
	void bindCached() const noexcept;

	const std::vector<ID3D11RenderTargetView*>& getRenderTargetViews()		const noexcept { return m_rtv;  }
	const std::vector<ID3D11ShaderResourceView*>& getShaderResourceViews()	const noexcept { return m_srv;  }

	[[nodiscard]] int getWidth() const noexcept { return m_width; }
	[[nodiscard]] int getHeight() const noexcept { return m_height; }

	ID3D11ShaderResourceView* getResource(const int slot = 0)						{ return m_srv[slot]; }
	const ID3D11ShaderResourceView* getResource(const int slot = 0) const			{ return m_srv[slot]; }
	const DepthBuffer& getDepth()									const noexcept	{ return m_depth; }

	ID3D11ShaderResourceView* bindUnlitRTV() const;
	static void unbind();
};