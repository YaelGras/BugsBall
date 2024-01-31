#pragma once

#include <vector>
#include "Vertex.h"
#include "../../Platform/WindowsEngine.h"

struct ID3D11Buffer;

class VertexBuffer
{
	std::vector<Vertex> m_vertices;

	// -- Buffer stuff
	ID3D11Buffer* m_vbo				= nullptr;


#ifdef D3D11_IMPL
	d3d11_graphics::RenderingContext m_renderContext;
#endif

public:

	VertexBuffer(const std::vector<Vertex>& vertices);


	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// -- Basic operations

	[[nodiscard]] size_t getVerticesCount()			const noexcept { return m_vertices.size(); }
	[[nodiscard]] std::vector<Vertex> getVertices() const noexcept { return m_vertices; }
	void bind() const noexcept;

	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// -- Constructors, move and stuff

	VertexBuffer() = default;
	void swap(VertexBuffer& other) noexcept;
	VertexBuffer(const VertexBuffer&) = delete;
	VertexBuffer& operator=(const VertexBuffer&) = delete;
	VertexBuffer(VertexBuffer&& other) noexcept;
	VertexBuffer& operator=(VertexBuffer&& other) noexcept;
	~VertexBuffer();




};

