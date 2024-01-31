#pragma once

#include <DirectXMath.h>

using namespace DirectX;

struct Vertex 
{
	XMVECTOR position;
	XMVECTOR normal;
	XMVECTOR uv;
};

struct SpriteVertex
{
	XMVECTOR position;
	XMVECTOR uv;
};

struct TexturedSpriteVertex
{
	XMVECTOR position;
	XMVECTOR uv;
	XMVECTOR texId;
};