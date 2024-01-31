#pragma once

#pragma warning(suppress : 6387) // removes useless intellisense warning

struct ID3D11DepthStencilState;
struct ID3D11DepthStencilView;
struct ID3D11Texture2D;
struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11ShaderResourceView;


class DepthBuffer {

private:

	ID3D11Texture2D*			m_depthTexture	= nullptr;
	ID3D11DepthStencilView*		m_depthStencil	= nullptr;
	ID3D11DepthStencilState*	m_state			= nullptr;
	ID3D11ShaderResourceView*	m_srv			= nullptr;

	ID3D11Device*				device			= nullptr;
	ID3D11DeviceContext*		context			= nullptr;


public:

	///////////////////////////////////////////////////////////////////////////////////////////////
	/// Operations

	void initDepthBuffer(int width, int height);

	void bind() const noexcept;
	void clearDepth() const noexcept;

	///////////////////////////////////////////////////////////////////////////////////////////////
	/// Getters

	[[nodiscard]] ID3D11DepthStencilState*	getState()		const noexcept { return m_state; }
	[[nodiscard]] ID3D11Texture2D*			getTexture()	const noexcept { return m_depthTexture; }
	[[nodiscard]] ID3D11DepthStencilView*	getView()		const noexcept { return m_depthStencil; }
	[[nodiscard]] ID3D11ShaderResourceView*	getSrv()		const noexcept { return m_srv; }

	///////////////////////////////////////////////////////////////////////////////////////////////
	/// Constructors and stuff

	DepthBuffer() = default;
	DepthBuffer(ID3D11Device* _device, ID3D11DeviceContext* _context);
	DepthBuffer(const DepthBuffer&) = delete;
	DepthBuffer& operator=(const DepthBuffer&) = delete;
	void swap(DepthBuffer& other) noexcept;
	DepthBuffer(DepthBuffer&&)noexcept;
	DepthBuffer& operator=(DepthBuffer&&)noexcept;
	~DepthBuffer();

};