#pragma once

#include <wrl/client.h>
#include <filesystem>
#include "../../Platform/WindowsEngine.h"

struct ID3D11ShaderResourceView;

namespace fs = std::filesystem;
class Texture 
{
private:

	std::wstring m_path;

#ifdef D3D11_IMPL
	//ID3D11Texture2D* m_texture = nullptr;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_texture = nullptr;
	d3d11_graphics::RenderingContext m_renderContext;
	D3D11_TEXTURE2D_DESC m_desc;
#endif

public:



	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// -- Basic operations

	ID3D11ShaderResourceView* getTexture() const { return m_texture.Get(); }

	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// -- Getters
	///

	[[nodiscard]] int getWidth() const noexcept { return m_desc.Width; }
	[[nodiscard]] int getHeight() const noexcept { return m_desc.Height; }

	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// -- Constructor and stuff

	Texture() = default;
	Texture(const std::wstring& path, bool activateMipmap = true);
	Texture(const std::string& path, bool activateMipmap = true);
	Texture(int width, int height);
	Texture(ID3D11ShaderResourceView*);
	Texture(nullptr_t) {};

	void swap(Texture& other) noexcept;
	Texture(const Texture& other);
	Texture& operator=(const Texture& other);
	Texture(Texture&& other) noexcept;
	Texture& operator=(Texture&& other) noexcept;
	~Texture();



};