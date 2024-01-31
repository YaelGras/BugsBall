#include "TextureCube.h"

#include <filesystem>
#include <d3d11.h>
#include <array>

#include "../../Platform/IO/DDSTextureLoader11.h"
#include "../../Platform/WindowsEngine.h"

#include <locale>
#include <codecvt>

inline std::wstring widestring2string(const std::string& string)
{
	using convert_type = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_type, wchar_t> converter;
	return converter.from_bytes(string);
}



TextureCube::TextureCube(const std::string& path)
		: m_path(path)
		, m_srv(nullptr)
		, m_tex(nullptr)
		, resource(nullptr)
	{
	bool exists = std::filesystem::exists(path);
#ifdef D3D11_IMPL
		m_renderContext = WindowsEngine::getInstance().getGraphics().getContext();

		if (auto hr = DirectX::CreateDDSTextureFromFileEx(
			m_renderContext.device,
			m_renderContext.context,
			widestring2string(path).c_str(),
			0,
			D3D11_USAGE_DEFAULT,
			D3D11_BIND_SHADER_RESOURCE,
			0,
			D3D11_RESOURCE_MISC_TEXTURECUBE,
			DirectX::DDS_LOADER_DEFAULT,
			&resource,
			&m_srv
		); hr != S_OK) {
		}

#endif
	}

