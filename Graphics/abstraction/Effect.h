///////////////////////////////////////
/// THIS FILE NEED LOTS OF CLEANING	///
///	BE PATIENT PLS					///
///////////////////////////////////////

#pragma once

#include <filesystem>
#include <vector>
#include <d3d11.h>
#include <string>

#include "../../d3dx11effect.h"
#include <map>
#include "../../Platform/WindowsEngine.h"

#define _XM_NO_INTRINSICS_
#include <DirectXMath.h>
#include "InputLayout.h"

class Texture;
using namespace DirectX;
namespace fs = std::filesystem;



class Effect 
{
private:

	d3d11_graphics::RenderingContext m_renderContext;
	std::map < std::string, ID3D11Buffer* > m_constantBuffers;

	ID3DX11EffectTechnique* m_technique; 
	ID3DX11EffectPass* m_pass;
	ID3D11InputLayout* m_vertexLayout = nullptr;
	ID3DX11Effect* m_effect;

	// Recompile stuff
	InputLayout m_layout;
	fs::path m_path;

public:

	Effect();
	~Effect();

	ID3D11InputLayout* getVertexLayout() const { return m_vertexLayout; }

	void recompile();
	void loadEffectFromFile(const fs::path& pathToEffect);
	void bindInputLayout(const InputLayout& layout);


	void addNewCBuffer(const std::string& name, uint32_t structSize);
	void apply() const;
	void bindTexture(const std::string& uniformName,const ID3D11ShaderResourceView* tex) const;

	void bindTextureArray(const std::string& uniformName, const std::vector<const Texture*>& tex) const;
	void bindTextureArray(const std::string& uniformName, const std::vector<ID3D11ShaderResourceView*>& tex) const;
	void bindTexture(const std::string& uniformName, const Texture& tex) const;

	void setUniformVector(const std::string& uniformName, const DirectX::XMVECTOR& value) const;
	void setUniformFloat(const std::string& uniformName, float value) const;


	void unbindResources() const 
	{
		ID3D11ShaderResourceView* nullSRV =  nullptr ;
		ID3D11SamplerState* nullSampler =  nullptr ;
		m_renderContext.context->PSSetShaderResources(0, 1, &nullSRV);
		m_renderContext.context->PSSetSamplers(0, 1, &nullSampler);
		m_technique->GetPassByIndex(0)->Apply(0, m_renderContext.context);
	}

	void unbindTexture(const std::string& uniformName) const
	{
		static ID3D11Texture2D* nullTexture = nullptr;
		m_effect->GetVariableByName(uniformName.c_str())->AsShaderResource()->SetResource(reinterpret_cast<ID3D11ShaderResourceView*>(nullTexture));
	}

public:

	template<class ShaderParam>
	void updateConstantBuffer(const ShaderParam& sp, const std::string& cbuffName) const
	{
		if (!m_constantBuffers.contains(cbuffName)) throw std::runtime_error("Couldn't find cbuffer name " + cbuffName);
		m_renderContext.context->UpdateSubresource(m_constantBuffers.at(cbuffName), 0, nullptr, &sp, 0, 0);
		sendCBufferToGPU(cbuffName);
	}

	template<class ShaderParam>
	void updateConstantBuffer(const ShaderParam& sp, const std::string& cbuffName, size_t size) const
	{
		if (!m_constantBuffers.contains(cbuffName)) throw std::runtime_error("Couldn't find cbuffer name " + cbuffName);
		m_renderContext.context->UpdateSubresource(m_constantBuffers.at(cbuffName), 0, nullptr, &sp, size, 0);
		sendCBufferToGPU(cbuffName);
	}

private:

	void sendCBufferToGPU(const std::string& cbuffName) const;
};