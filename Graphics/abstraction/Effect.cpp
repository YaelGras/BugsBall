#include "Effect.h"
#include "../../Platform/WindowsEngine.h"

#include <d3dcompiler.h>
#include <optional>

#include "Texture.h"

Effect::Effect() 
	: m_effect(nullptr)
	, m_pass(nullptr)
	, m_technique(nullptr)
{
#ifdef D3D11_IMPL
		m_renderContext = WindowsEngine::getInstance().getGraphics().getContext();
#endif
}

Effect::~Effect() {
	std::ranges::for_each(m_constantBuffers, [&](auto&& p) { DX_RELEASE(p.second); });
	DX_RELEASE(m_vertexLayout);
	DX_RELEASE(m_technique);
	DX_RELEASE(m_pass);
	DX_RELEASE(m_effect);
}

	void Effect::loadEffectFromFile(const fs::path& pathToEffect) {


		ID3DBlob* blob = nullptr;
		ID3DBlob* compilationErrorMessage = nullptr;


		HRESULT hr = D3DCompileFromFile(pathToEffect.c_str(),
			0, 0, 0, "fx_5_0", 0, 0, &blob, &compilationErrorMessage);

		if (hr != S_OK) {
			const void* errorBuffer = compilationErrorMessage->GetBufferPointer();
			const char* errorMessage = static_cast<const char*>(errorBuffer);
			throw std::runtime_error(errorMessage);
		}

		D3DX11CreateEffectFromMemory(
			blob->GetBufferPointer(), blob->GetBufferSize(), 0, m_renderContext.device, &m_effect);

		blob->Release();

		m_technique = m_effect->GetTechniqueByIndex(0);
		m_pass = m_technique->GetPassByIndex(0);

		m_path = pathToEffect;

	}

void Effect::recompile()
{
	unbindResources();
	ID3DBlob* blob = nullptr;

	ID3DBlob* compilationErrorMessage;
	HRESULT hr = D3DCompileFromFile(m_path.c_str(), 0, 0, 0, "fx_5_0", 0, 0, &blob, &compilationErrorMessage);

	if (hr != S_OK) {
		const void* errorBuffer = compilationErrorMessage->GetBufferPointer();
		const char* errorMessage = static_cast<const char*>(errorBuffer);
		throw std::runtime_error(errorMessage);
	}
	DX_RELEASE(m_technique);
	DX_RELEASE(m_pass);
	DX_RELEASE(m_effect);
	hr = D3DX11CreateEffectFromMemory(blob->GetBufferPointer(), blob->GetBufferSize(), 0, m_renderContext.device, &m_effect);


	if (hr != S_OK) {
		const void* errorBuffer = compilationErrorMessage->GetBufferPointer();
		const char* errorMessage = static_cast<const char*>(errorBuffer);
		throw std::runtime_error(errorMessage);
	}

	blob->Release();


	m_technique = m_effect->GetTechniqueByIndex(0);
	m_pass = m_technique->GetPassByIndex(0);
	if (m_layout.getElements().size()) bindInputLayout(m_layout);

}

	void Effect::bindInputLayout(const InputLayout& layout) {

		D3DX11_PASS_SHADER_DESC passDesc;
		D3DX11_EFFECT_SHADER_DESC effectDesc;
		m_layout = InputLayout{ layout };

		m_pass->GetVertexShaderDesc(&passDesc);

		passDesc.pShaderVariable->GetShaderDesc(passDesc.ShaderIndex, &effectDesc);
		const void* vsCodePtr = effectDesc.pBytecode;
		unsigned vsCodeLen = effectDesc.BytecodeLength;

		DX_RELEASE(m_vertexLayout);
		auto hr = m_renderContext.device->CreateInputLayout(
			m_layout.asInputDesc(),
			static_cast<UINT>(m_layout.getElements().size()), vsCodePtr, static_cast<SIZE_T>(vsCodeLen),
			&m_vertexLayout
		);

		if (hr != S_OK) throw std::runtime_error("Error creating vertex layout");
		

	}

	
	// Returns the slot number of the newly created cbuffer
	void Effect::addNewCBuffer(const std::string& name, uint32_t structSize)
	{
		ID3D11Buffer* constBuffer;

		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = structSize;
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = 0;
		m_renderContext.device->CreateBuffer(&bd, nullptr, &constBuffer);
		m_constantBuffers[name] = constBuffer;


	}

	void Effect::apply() const {
		m_pass->Apply(0, m_renderContext.context);
	}

	void Effect::bindTexture(const std::string& uniformName, const ID3D11ShaderResourceView* tex) const
	{
		if (!tex) return;
		m_effect->GetVariableByName(uniformName.c_str())->AsShaderResource()->SetResource(
			(ID3D11ShaderResourceView*)(tex));
	}



	void Effect::bindTextureArray(const std::string& uniformName, const std::vector<const Texture*>& tex) const
	{
		if (tex.empty() || !tex.data()) return;

		std::vector<ID3D11ShaderResourceView*> srvs(tex.size());
		std::ranges::transform(tex, srvs.begin(), [&](const Texture* t) { return (t) ? t->getTexture() : nullptr; });
		m_effect->GetVariableByName(uniformName.c_str())->AsShaderResource()->SetResourceArray(
			(ID3D11ShaderResourceView**)(srvs.data())
			, 0, static_cast<uint32_t>(srvs.size()));
	}

	void Effect::bindTextureArray(const std::string& uniformName, const std::vector<ID3D11ShaderResourceView*>& tex) const
	{
		if (tex.empty() || !tex.data()) return;
		m_effect->GetVariableByName(uniformName.c_str())->AsShaderResource()->SetResourceArray(
			(ID3D11ShaderResourceView**)(tex.data())
			,0, static_cast<uint32_t>(tex.size()));
	}

	void Effect::bindTexture(const std::string& uniformName, const Texture& tex) const
	{
		m_effect->GetVariableByName(uniformName.c_str())->AsShaderResource()->SetResource(tex.getTexture());
	}

	void Effect::setUniformVector(const std::string& uniformName,const DirectX::XMVECTOR& value) const
	{
		m_effect->GetVariableByName(uniformName.c_str())->AsVector()->SetFloatVector(value.vector4_f32);
	}

	void Effect::setUniformFloat(const std::string& uniformName, float value) const
	{
		m_effect->GetVariableByName(uniformName.c_str())->AsScalar()->SetFloat(value);
	}

	void Effect::sendCBufferToGPU(const std::string& cbuffName) const
	{
		ID3DX11EffectConstantBuffer* pCB = m_effect->GetConstantBufferByName(cbuffName.c_str());
		pCB->SetConstantBuffer(m_constantBuffers.at(cbuffName));
		DX_RELEASE(pCB);
	}
