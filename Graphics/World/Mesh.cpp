#pragma once
#include "Mesh.h"

#include <directXmath.h>
#include <d3d11.h>
#include <vector>
#include <utility>
#include <algorithm>

#include "../Renderer.h"
#include "../abstraction/IndexBuffer.h"
#include "../abstraction/VertexBuffer.h"
#include "../World/Material.h"

#include "../../Utils/Transform.h"
#include "../../Utils/AABB.h"

using namespace DirectX;

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<IndexBuffer::size_type>& indices)
{
	m_vbo = VertexBuffer(vertices);
	m_ibo = IndexBuffer(indices);
	m_submeshes = std::vector<IndexBuffer::size_type>{ 0, (IndexBuffer::size_type)(indices.size()) };
	computeBoundingBox();
};

Mesh::Mesh(	const std::vector<Vertex>& vertices,
			const std::vector<IndexBuffer::size_type>& indices,
			const std::vector<IndexBuffer::size_type>& submeshIndices)
	{
		m_vbo = VertexBuffer(vertices);
		m_ibo = IndexBuffer(indices);
		m_submeshes = submeshIndices;
		m_materials = {};
		computeBoundingBox();
	};

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<IndexBuffer::size_type>& indices,
		const std::vector<IndexBuffer::size_type>& submeshIndices,
		const std::vector<Material>& materials,
		const std::vector<uint16_t>& submeshMats
)
{
	m_vbo = VertexBuffer(vertices);
	m_ibo = IndexBuffer(indices);
	m_submeshes = submeshIndices;
	m_materials = materials;
	m_submeshMats = submeshMats;
	computeBoundingBox();
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Mesh::draw() const {

	m_vbo.bind();
	m_ibo.bind();

	for (int index = 0; index < m_submeshes.size() - 1; ++index)
	{
		size_t indexCount = m_submeshes[index + 1] - m_submeshes[index];
		Renderer::drawIndexed(indexCount, m_submeshes[index], 0);
	}

}

// Very inaccurate
void Mesh::computeBoundingBox() noexcept
{
	XMVECTOR maxp{0.f,0.f,0.f};
	XMVECTOR minp{0.f,0.f,0.f};

	for (const Vertex& v : m_vbo.getVertices())
	{
		minp = XMVectorSetX(minp, min(XMVectorGetX(minp), XMVectorGetX(v.position)));
		minp = XMVectorSetY(minp, min(XMVectorGetY(minp), XMVectorGetY(v.position)));
		minp = XMVectorSetZ(minp, min(XMVectorGetZ(minp), XMVectorGetZ(v.position)));
		
		maxp = XMVectorSetX(maxp, max(XMVectorGetX(maxp), XMVectorGetX(v.position)));
		maxp = XMVectorSetY(maxp, max(XMVectorGetY(maxp), XMVectorGetY(v.position)));
		maxp = XMVectorSetZ(maxp, max(XMVectorGetZ(maxp), XMVectorGetZ(v.position)));

	}

	m_boundingBox = AABB::makeAABBFromPoints(minp, maxp);
	BoundingSphere bs = makeSphereFromAABB(m_boundingBox);
	m_boundingBox = makeAABBFromSphere(bs);


}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Mesh::swap(Mesh& other) noexcept
{
	std::swap(other.m_transform, m_transform);
	std::swap(other.m_ibo, m_ibo);
	std::swap(other.m_vbo, m_vbo);
	std::swap(other.m_materials, m_materials);
	std::swap(other.m_submeshMats, m_submeshMats);
	std::swap(other.m_submeshes, m_submeshes);
	std::swap(other.m_boundingBox, m_boundingBox);
}


Mesh::Mesh(Mesh&& other) noexcept
	: m_vbo			(std::exchange(other.m_vbo,			{}))
	, m_ibo			(std::exchange(other.m_ibo,			{}))
	, m_materials	(std::exchange(other.m_materials,	{}))
	, m_submeshes	(std::exchange(other.m_submeshes,	{}))
	, m_submeshMats	(std::exchange(other.m_submeshMats,	{}))
	, m_transform	(std::exchange(other.m_transform,	{}))
	, m_boundingBox	(std::exchange(other.m_boundingBox,	{}))
{
	
}

Mesh& Mesh::operator=(Mesh&& other) noexcept {
	Mesh{ std::move(other) }.swap(*this);
	return *this;
}
