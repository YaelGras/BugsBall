#include "Material.h"

#include <unordered_map>
#include <string>
#include <directXmath.h>


#include <codecvt>
#include <locale>

using namespace DirectX;

inline std::wstring widestring2string2(const std::string& string)
{
	using convert_type = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_type, wchar_t> converter;
	return converter.from_bytes(string);
}


/*
ILLUMINATION MODEL
0. Color on and Ambient off
1. Color on and Ambient on
2. Highlight on
3. Reflection on and Ray trace on
4. Transparency: Glass on, Reflection: Ray trace on
5. Reflection: Fresnel on and Ray trace on
6. Transparency: Refraction on, Reflection: Fresnel off and Ray trace on
7. Transparency: Refraction on, Reflection: Fresnel on and Ray trace on
8. Reflection on and Ray trace off
9. Transparency: Glass on, Reflection: Ray trace off
10. Casts shadows onto invisible surfaces
*/


void Material::loadTextures(const std::initializer_list<std::pair<std::string, TextureType>>& textures)
{
	for (auto& t : textures)
		m_textures[t.second] = std::move(Texture(widestring2string2(t.first)));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Material::Material(std::initializer_list<Material::mat_key>&& init)
	: m_textures(std::unordered_map<TextureType, Texture>{init.begin(), init.end()})
	, m_coefs{}
	, m_illuminationModel(0)
{}

Material::Material()
	: m_textures(std::unordered_map<TextureType, Texture>{})
	, m_coefs{}
	, m_illuminationModel(0)
{}

void Material::swap(Material& other) noexcept
{
	static_assert(sizeof(float[4]) == sizeof(XMVECTOR));
	std::swap(other.m_textures, m_textures);
	std::swap(other.m_coefs, m_coefs);
	std::swap(other.m_illuminationModel, m_illuminationModel);
}


Material& Material::operator=(const Material& other)
{
	Material{ other }.swap(*this);
	return *this;
}

Material::Material(Material&& other) noexcept
	: m_textures(std::exchange(other.m_textures, {}))
	, m_coefs(std::exchange(other.m_coefs, {}))
	, m_illuminationModel(std::exchange(other.m_illuminationModel, 0))
{}

Material& Material::operator=(Material&& other) noexcept {
	Material{ std::move(other) }.swap(*this);
	return *this;
}
