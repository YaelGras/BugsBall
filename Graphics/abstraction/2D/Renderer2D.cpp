#include "Renderer2D.h"
#pragma once

#include <vector>
#include <map>
#include <array>
#include <unordered_map>

// make this an instanced
#define _XM_NO_INTRINSICS_
#include <DirectXMath.h>

#include "abstraction/VertexBuffer.h"
#include "abstraction/IndexBuffer.h"
#include "abstraction/Effect.h"
#include "abstraction/Texture.h"


Renderer2D::Renderer2D()
{
	m_quadEffect.loadEffectFromFile("res/effects/quad.fx");

	InputLayout layout;
	layout.pushBack<4>(InputLayout::Semantic::Position); // (x,y,texId,w)
	layout.pushBack<2>(InputLayout::Semantic::Texcoord);

	m_quadEffect.bindInputLayout(layout);

	m_batchedIndices.resize(MaxIndicesCount);
	int offset = 0;

	for (int i = 0; i < MaxIndicesCount; i += 6)
	{
		m_batchedIndices[i + 0] = 0 + offset;
		m_batchedIndices[i + 1] = 1 + offset;
		m_batchedIndices[i + 2] = 2 + offset;

		m_batchedIndices[i + 3] = 2 + offset;
		m_batchedIndices[i + 4] = 3 + offset;
		m_batchedIndices[i + 5] = 0 + offset;

		offset += 4;
	}
	m_ibo = IndexBuffer(m_batchedIndices);
}

Renderer2D::~Renderer2D()
{
	DX_RELEASE(m_vbo);
}

void Renderer2D::clear()
{
	m_batchedVertices.clear();
	m_batchedIndices.clear();
	m_textureMap.clear();
}


void Renderer2D::batchQuad(XMVECTOR position, XMVECTOR size, const Texture& texture,
                           XMVECTOR uvoffset /*= { 0,0,0,0 }*/)
{
	const auto winSize = WindowsEngine::getInstance().getGraphics().getWinSize();
	if (m_textureMap.size() >= 7)
	{
		CreateVertexBuffer();
		renderBatchedQuads();
	}

	if (!m_textureMap.contains(&texture))
	{
		m_textureMap[&texture] = static_cast<int>(m_textureMap.size());
	}


	float quadTexId = static_cast<float>(m_textureMap[&texture]);

	TexturedSpriteVertex quadVertices[4] = {};
	XMVECTOR positions[4] =
	{
		{
			(XMVectorGetX(position)) / winSize.first * 2 - 1, -2 * (XMVectorGetY(position) / winSize.second) + 1,
			quadTexId
		},


		{
			(XMVectorGetX(position) + XMVectorGetX(size)) / winSize.first * 2 - 1,
			-2 * (XMVectorGetY(position) / winSize.second) + 1,
			quadTexId
		},

		{
			(XMVectorGetX(position) + XMVectorGetX(size)) / winSize.first * 2 - 1,
			-2 * ((XMVectorGetY(position) + XMVectorGetY(size)) / winSize.second) + 1,
			quadTexId
		},

		{
			(XMVectorGetX(position)) / winSize.first * 2 - 1,
			-2 * ((XMVectorGetY(position) + XMVectorGetY(size)) / winSize.second) + 1,
			quadTexId
		}
	};

	XMVECTOR uv[4] =
	{
		{0, 1},
		{1, 1},
		{1, 0},
		{0, 0}
	};


	m_batchedVertices.push_back(TexturedSpriteVertex{positions[0], uv[0]});
	m_batchedVertices.push_back(TexturedSpriteVertex{positions[1], uv[1]});
	m_batchedVertices.push_back(TexturedSpriteVertex{positions[2], uv[2]});
	m_batchedVertices.push_back(TexturedSpriteVertex{positions[3], uv[3]});
}


void Renderer2D::renderBatchedQuads()
{
	CreateVertexBuffer();
	if (m_batchedVertices.empty()) return;
	bindTextures();
	m_quadEffect.apply();

	constexpr UINT stride = sizeof(TexturedSpriteVertex);
	constexpr UINT offset = 0;

	WindowsEngine::getInstance().getGraphics().getContext().context->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	WindowsEngine::getInstance().getGraphics().getContext().context->IASetInputLayout(m_quadEffect.getVertexLayout());
	WindowsEngine::getInstance().getGraphics().getContext().context->IASetVertexBuffers(0, 1, &m_vbo, &stride, &offset);

	m_ibo.bind();
	WindowsEngine::getInstance().getGraphics().getContext().context->DrawIndexed(
		static_cast<UINT>(m_batchedVertices.size() * 1.5f), 0, 0);
	m_batchedVertices.clear();
}

void Renderer2D::bindTextures() const
{
	std::vector<ID3D11ShaderResourceView*> srvs;
	srvs.resize(8);
	for (auto& p : m_textureMap)
	{
		srvs[p.second] = p.first->getTexture();
	}
	m_quadEffect.bindTextureArray("tex_slots", srvs);
}


void Renderer2D::CreateVertexBuffer()
{
	if (m_batchedVertices.empty()) return;
	DX_RELEASE(m_vbo);
	D3D11_BUFFER_DESC descriptor{};
	D3D11_SUBRESOURCE_DATA m_initData{};
	// -- Vertex buffer
	ZeroMemory(&descriptor, sizeof(descriptor));

	descriptor.Usage = D3D11_USAGE_IMMUTABLE;
	descriptor.ByteWidth = static_cast<UINT>(m_batchedVertices.size()) * sizeof(TexturedSpriteVertex);
	descriptor.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	descriptor.CPUAccessFlags = 0;

	ZeroMemory(&m_initData, sizeof(m_initData));
	m_initData.pSysMem = m_batchedVertices.data();

	WindowsEngine::getInstance().getGraphics().getContext().device->CreateBuffer(&descriptor, &m_initData, &m_vbo);
}
