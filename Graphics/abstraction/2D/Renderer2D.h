#pragma once

#include <vector>
#include <unordered_map>

// make this an instanced
#define _XM_NO_INTRINSICS_
#include <DirectXMath.h>

#include "abstraction/VertexBuffer.h"
#include "abstraction/IndexBuffer.h"
#include "abstraction/Texture.h"
#include "abstraction/Effect.h"


struct Quad 
{
	XMVECTOR position; // (x,y,size_x,size_y)
	XMVECTOR size;
	XMVECTOR uv_v4; // (u,v,size_u,size_v)
	Texture* texture = nullptr;
	XMVECTOR color = { 0,0,0,1 };
};

class Renderer2D {

private:

	Effect m_quadEffect;

	static constexpr size_t MaxQuadCount = 1000;
	static constexpr size_t MaxVertexCount = MaxQuadCount * 4;
	static constexpr size_t MaxIndicesCount = MaxQuadCount * 6;


	ID3D11Buffer* m_vbo = nullptr; // sadly my vertex buffers don't take generic vertices
	IndexBuffer m_ibo;

	std::vector<TexturedSpriteVertex> m_batchedVertices;
	std::vector<IndexBuffer::size_type> m_batchedIndices;

	std::unordered_map<const Texture*, int> m_textureMap; // holds up to 8 textures and remember its id

public:

	Renderer2D();
	~Renderer2D();


	////////////////////////////////////////////////////////////
	///
	
	void clear();
	void batchQuad(XMVECTOR position, XMVECTOR size, const Texture& texture, XMVECTOR uvoffset = { 0,0,0,0 });
	void renderBatchedQuads();

private:
	void CreateVertexBuffer();
	void bindTextures() const;
};