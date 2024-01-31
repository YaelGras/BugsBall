#include "ComputeShader.h"


#include <codecvt>
#include <d3d11.h>

#include "../Platform/WindowsEngine.h"


inline std::string narrow(const std::wstring& string)
{
	using convert_type = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_type, wchar_t> converter;
	return converter.to_bytes(string);
}


inline std::wstring widen(const std::string& string)
{
	using convert_type = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_type, wchar_t> converter;
	return converter.from_bytes(string);
}


ComputeShader::ComputeShader(const std::string& path)
	{
		m_context = WindowsEngine::getInstance().getGraphics().getContext();

		DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( _DEBUG )
		dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

		LPCSTR pProfile = (m_context.device->GetFeatureLevel() >= D3D_FEATURE_LEVEL_11_0) ? "cs_5_0" : "cs_4_0";

		ID3DBlob* pErrorBlob = NULL;
		ID3DBlob* pBlob = NULL;
		HRESULT hr = D3DCompileFromFile(widen(path).c_str(), nullptr,
			D3D_COMPILE_STANDARD_FILE_INCLUDE, "CSMain", pProfile, dwShaderFlags, 0, &pBlob, &pErrorBlob);

		if (FAILED(hr))
		{

			const void* errorBuffer = pErrorBlob->GetBufferPointer();
			const char* errorMessage = static_cast<const char*>(errorBuffer);
			throw std::runtime_error(errorMessage);

		}


		m_context.device->CreateComputeShader(
			pBlob->GetBufferPointer(),
			pBlob->GetBufferSize(),
			nullptr,
			&m_computeShader);

		if (pErrorBlob)
			pErrorBlob->Release();
		if (pBlob)
			pBlob->Release();


	}

ComputeShader::~ComputeShader()
	{
		DX_RELEASE(nullUAV)
			DX_RELEASE(m_computeShader)
	}

	// uavs need to be bound to a texture that derives from an rtv
	void ComputeShader::bind(const std::vector<ID3D11UnorderedAccessView*>& uav) const
	{
		m_context.context->CSSetShader(m_computeShader, nullptr, 0);
		m_context.context->CSSetUnorderedAccessViews(0, static_cast<UINT>(uav.size()), uav.data(), nullptr);
	}


	void ComputeShader::dispatch(int x, int y, int z) const
	{
		m_context.context->Dispatch(x, y, z);
	}



	void ComputeShader::unbind() const
	{
		static ID3D11ComputeShader* nullCS = nullptr;
		m_context.context->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);
		m_context.context->CSSetShader(nullCS, nullptr, 0);
	}