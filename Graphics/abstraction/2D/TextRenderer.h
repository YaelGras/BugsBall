#pragma once


#define _XM_NO_INTRINSICS_
#include <directXMath.h>

#include "../Platform/WindowsEngine.h"
#include "abstraction/Effect.h"
#include "abstraction/Texture.h"

using namespace DirectX;

class TextRenderer {

public:

	struct CharInstance
	{
		XMVECTOR position;
		XMVECTOR size;
		XMVECTOR uv;
		XMVECTOR color;
	};

private:

	ID3D11Buffer* m_instancedBuffer = nullptr;
	ID3D11Buffer* m_vertexBuffer	= nullptr;
	ID3D11BlendState* m_blendState	= nullptr;

	d3d11_graphics::RenderingContext m_renderContext;

	size_t m_instanceCount;
	InputLayout m_layout;
	Effect m_spriteEffect;
	Texture m_font = Texture(L"res/textures/inv_font.dds");

	std::vector<CharInstance> m_allText;

public:

	void clear();
	void writeTextOnScreen(const std::string& text, int screenX, int screenY, float scale, XMVECTOR color = { {1,1,1,1} });
	void render() const;

private:
	void bind() const ;
	void resizeBuffers();
	std::vector<CharInstance> generateText(const std::string& text, int x, int y, float scale, XMVECTOR color = { {1,1,1,1} });

public:
	TextRenderer();
	~TextRenderer();
};