#include "VertexBuffer.h"

#include <d3d11.h>
#include <vector>
#include <algorithm>

#include "Vertex.h"
#include "../../Utils/Debug.h"
#include "../../Platform/WindowsEngine.h"


VertexBuffer::VertexBuffer(const std::vector<Vertex>& vertices)
	{
#ifdef D3D11_IMPL
		m_renderContext = WindowsEngine::getInstance().getGraphics().getContext();
		m_vertices = vertices;

		D3D11_BUFFER_DESC descriptor{};
		D3D11_SUBRESOURCE_DATA m_initData{};
		// -- Vertex buffer
		ZeroMemory(&descriptor, sizeof(descriptor));

		descriptor.Usage = D3D11_USAGE_IMMUTABLE;
		descriptor.ByteWidth = static_cast<UINT>(vertices.size()) * sizeof(Vertex);
		descriptor.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		descriptor.CPUAccessFlags = 0;

		ZeroMemory(&m_initData, sizeof(m_initData));
		m_initData.pSysMem = &vertices[0];

		DX_TRY(m_renderContext.device->CreateBuffer(&descriptor, &m_initData, &m_vbo));
#endif
	}


	void VertexBuffer::bind() const noexcept
	{
		constexpr UINT stride = sizeof(Vertex);
		constexpr UINT offset = 0;

#ifdef D3D11_IMPL
		m_renderContext.context->IASetVertexBuffers(0, 1, &m_vbo, &stride, &offset);
#endif
	}

void VertexBuffer::swap(VertexBuffer& other) noexcept {
	std::swap(other.m_vbo, m_vbo);
	std::swap(other.m_vertices, m_vertices);
	std::swap(other.m_renderContext, m_renderContext);
}

VertexBuffer::VertexBuffer(VertexBuffer&& other) noexcept
	:m_renderContext(other.m_renderContext)
	, m_vbo(std::exchange(other.m_vbo, nullptr))
	, m_vertices(std::exchange(other.m_vertices, {}))
{	}
VertexBuffer& VertexBuffer::operator=(VertexBuffer&& other) noexcept
{
	VertexBuffer{ std::move(other) }.swap(*this);
	return *this;
}

VertexBuffer::~VertexBuffer()
{
	DX_RELEASE(m_vbo);
}


