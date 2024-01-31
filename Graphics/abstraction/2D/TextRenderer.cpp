#include "TextRenderer.h"
#include "Font.h"
#include "abstraction/Vertex.h"

void TextRenderer::clear()
{
	m_allText.clear();
	m_instanceCount = 0;
	DX_RELEASE(m_instancedBuffer);
}

void TextRenderer::writeTextOnScreen(const std::string& text, int screenX, int screenY, float scale, XMVECTOR color)
{
	auto res = generateText(text, screenX, screenY, scale, color);
	m_allText.insert(m_allText.end(), res.begin(), res.end());
	m_instanceCount = m_allText.size();
	resizeBuffers();
}

void TextRenderer::resizeBuffers()
{
	DX_RELEASE(m_instancedBuffer);
	{
		D3D11_BUFFER_DESC desc_instance{};
		D3D11_SUBRESOURCE_DATA initInstance{};
		// -- Vertex buffer

		desc_instance.Usage = D3D11_USAGE_DEFAULT;
		desc_instance.ByteWidth = static_cast<UINT>(m_allText.size() + 1) * sizeof(CharInstance);
		desc_instance.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		desc_instance.CPUAccessFlags = 0;
		desc_instance.StructureByteStride = 0;
		desc_instance.MiscFlags = 0;
		initInstance.pSysMem = m_allText.data();
		m_renderContext.device->CreateBuffer(&desc_instance, &initInstance, &m_instancedBuffer);
	}

}

std::vector<TextRenderer::CharInstance> TextRenderer::generateText(const std::string& text, int x, int y, float scale, XMVECTOR color)
{
	const auto winSize = WindowsEngine::getInstance().getGraphics().getWinSize();
	std::vector<CharInstance> res;
	float x_offset = 0;
	for (char c : text)
	{
		CharInfos ch = fontChars[c];
		CharInstance instance;

		const float xpos = (x + x_offset );// / winSize.first;
		const float ypos = (y - (ch.height - ch.originY));// / winSize.second;

		instance.position =
		{ (2 * xpos / (winSize.first)),
		((2 * ypos / (winSize.second))),
		0 };

		instance.color = color;
		instance.size = { scale * ch.width / fontWidth, scale * ch.height / fontHeight ,1 };
		instance.uv = {
			(ch.x) / fontWidth,
			(ch.y) / fontHeight,
			(ch.width) / fontWidth,
			(ch.height) / fontHeight };


		x_offset += (ch.advance + ch.originX) * scale;
		res.push_back(instance);


	}

	/*std::for_each(res.begin(), res.end(), [&winSize](CharInstance& instance) {
		instance.position -= {winSize.first / 2.0f, winSize.second / 2.0f, 0.0f };
		});*/

	return res;
}

TextRenderer::TextRenderer() {
	m_renderContext = WindowsEngine::getInstance().getGraphics().getContext();
	{
		const SpriteVertex vertices[6]
		{
			{{0,0},{0,1}},
			{{1,1},{1,0}},
			{{1,0},{1,1}},
			{{0,1},{0,0}},
			{{1,1},{1,0}},
			{{0,0},{0,1}},
		};

		D3D11_BUFFER_DESC descriptor{};
		D3D11_SUBRESOURCE_DATA m_initData{};
		// -- Vertex buffer

		descriptor.Usage = D3D11_USAGE_IMMUTABLE;
		descriptor.ByteWidth = static_cast<UINT>(6) * sizeof(SpriteVertex);
		descriptor.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		descriptor.CPUAccessFlags = 0;
		m_initData.pSysMem = &vertices[0];
		m_renderContext.device->CreateBuffer(&descriptor, &m_initData, &m_vertexBuffer);
	}

	/////


	m_layout.pushBack<3>(InputLayout::Semantic::Position);
	m_layout.pushBack<2>(InputLayout::Semantic::Texcoord);

	m_layout.pushBackInstanced<3>("INSTANCE_POS");	// instance pos
	m_layout.pushBackInstanced<2>("INSTANCE_SIZE");	// instance size
	m_layout.pushBackInstanced<4>("INSTANCE_UV");	// instance uv
	m_layout.pushBackInstanced<4>("INSTANCE_COL");	// instance color


	m_spriteEffect.loadEffectFromFile("res/effects/font.fx");

	m_spriteEffect.bindInputLayout(m_layout);

	/////////////////////////////////////////////////////////////////
	D3D11_BLEND_DESC blendDesc{};
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	m_renderContext.device->CreateBlendState(&blendDesc, &m_blendState);

}

void TextRenderer::bind() const
{
	unsigned int strides[2];
	unsigned int offsets[2];
	ID3D11Buffer* bufferPointers[2];

	strides[0] = sizeof(SpriteVertex);
	strides[1] = sizeof(CharInstance);

	offsets[0] = 0;
	offsets[1] = 0;

	bufferPointers[0] = m_vertexBuffer;
	bufferPointers[1] = m_instancedBuffer;

	m_renderContext.context->IASetVertexBuffers(0, 2, bufferPointers, strides, offsets);

}

void TextRenderer::render() const {
	bind();
	m_spriteEffect.bindTexture("tex", m_font.getTexture());
	m_renderContext.context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_renderContext.context->IASetInputLayout(m_spriteEffect.getVertexLayout());
	m_renderContext.context->OMSetBlendState(m_blendState, nullptr, 0xffffffff);
	m_spriteEffect.apply();
	m_renderContext.context->DrawInstanced(6, 
		static_cast<UINT>(m_instanceCount), 0, 0);

	m_spriteEffect.unbindResources();

}

TextRenderer:: ~TextRenderer()
{
	DX_RELEASE(m_instancedBuffer);
	DX_RELEASE(m_vertexBuffer);
	DX_RELEASE(m_blendState);
}

