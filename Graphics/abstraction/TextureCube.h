#pragma once

#include "../../Platform/WindowsEngine.h"

struct ID3D11ShaderResourceView;
struct ID3D11Resource;
struct ID3D11Texture2D;

class TextureCube {

private:

	std::string m_path;

#ifdef D3D11_IMPL
	ID3D11ShaderResourceView* m_srv = nullptr;
	ID3D11Resource* resource = nullptr;
	ID3D11Texture2D* m_tex = nullptr;
	d3d11_graphics::RenderingContext m_renderContext;
#endif

public:

	TextureCube() = default;
	TextureCube(const std::string& path);

	ID3D11ShaderResourceView* getResourceView() const { return m_srv; }

};