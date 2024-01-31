#pragma once

#include <vector>
#include "../../Platform/WindowsEngine.h"

struct ID3D11Buffer;

class IndexBuffer
{
public:
	using size_type = uint32_t;
private:

	// -- Store indices
	std::vector<size_type> m_indices;

	// -- Buffer stuff

#ifdef D3D11_IMPL
	d3d11_graphics::RenderingContext m_renderContext;
	ID3D11Buffer* m_ibo				= nullptr;
#endif

public:

	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// -- Basic operations

	[[nodiscard]] size_t					getBufferSize() const;
	[[nodiscard]] std::vector<size_type>	getIndices() const noexcept;
	[[nodiscard]] size_t					getIndicesCount() const noexcept;
	void bind() const;

	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// -- Constructors and other stuff
	///
	IndexBuffer() = default;
	explicit IndexBuffer(const std::vector<size_type>& indices);

	void swap(IndexBuffer& other) noexcept;
	IndexBuffer(const IndexBuffer&) = delete;
	IndexBuffer& operator=(const IndexBuffer&) = delete;

	IndexBuffer(IndexBuffer&& other) noexcept;
	IndexBuffer& operator=(IndexBuffer&& other) noexcept;
	~IndexBuffer();






};