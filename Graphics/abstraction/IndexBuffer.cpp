#include "IndexBuffer.h"


#include <d3d11.h>
#include <vector>
#include <algorithm>

#include "../../Utils/Debug.h"
#include "../../Platform/WindowsEngine.h"



IndexBuffer::IndexBuffer(const std::vector<size_type>& indices)
	{
#ifdef D3D11_IMPL

		m_renderContext = WindowsEngine::getInstance().getGraphics().getContext();
		m_indices = indices;

		D3D11_BUFFER_DESC m_descriptor{};
		D3D11_SUBRESOURCE_DATA m_initData{};

		// -- Index buffer

		ZeroMemory(&m_descriptor, sizeof(m_descriptor));

		m_descriptor.Usage = D3D11_USAGE_IMMUTABLE;
		m_descriptor.ByteWidth = static_cast<UINT>(m_indices.size()) * sizeof(size_type);
		m_descriptor.BindFlags = D3D11_BIND_INDEX_BUFFER;
		m_descriptor.CPUAccessFlags = 0;

		ZeroMemory(&m_initData, sizeof(m_initData));
		m_initData.pSysMem = &indices[0];

		DX_TRY(m_renderContext.device->CreateBuffer(&m_descriptor, &m_initData, &m_ibo));
#endif
	}

	void IndexBuffer::swap(IndexBuffer& other) noexcept {
		std::swap(other.m_ibo, m_ibo);
		std::swap(other.m_indices, m_indices);
		std::swap(other.m_renderContext, m_renderContext);
	}


	size_t IndexBuffer::getBufferSize() const { return m_indices.size(); }

	std::vector<IndexBuffer::size_type> IndexBuffer::getIndices() const noexcept {
		return m_indices;
	}

	size_t IndexBuffer::getIndicesCount() const noexcept { return m_indices.size(); }

	void IndexBuffer::bind() const
	{
#ifdef D3D11_IMPL
		m_renderContext.context->IASetIndexBuffer(m_ibo, DXGI_FORMAT_R32_UINT, 0);
#endif	
	}




IndexBuffer::IndexBuffer(IndexBuffer&& other) noexcept
	: m_indices(std::exchange(other.m_indices, {}))
	, m_renderContext(other.m_renderContext)
	, m_ibo(std::exchange(other.m_ibo, nullptr))
{	}

IndexBuffer& IndexBuffer::operator=(IndexBuffer&& other) noexcept
	{
		IndexBuffer{ std::move(other) }.swap(*this);
		return *this;
	}

	IndexBuffer::~IndexBuffer()
	{
		DX_RELEASE(m_ibo);
	}