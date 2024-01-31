#pragma once

#include <unordered_map>
#include <string>
#include "../abstraction/Texture.h"

enum class TextureType {
	ALBEDO,	NORMAL,	BUMP,
	HEIGHT,	ROUGHNESS,	METALNESS,
	SPECULAR,AO,
};

struct MaterialCoefs
{
	float Ka[4];	// ambiant	
	float Kd[4];	// diffuse
	float Ks[4];	// specular
	float Ke[4];	// emissive	
	float Ns;		// exponent specular	
	float Ni; // refraction, optical density	
	float transparency; // 1 - d

	float padding;
};

class Material {

private:

	std::unordered_map<TextureType, Texture> m_textures;
	MaterialCoefs m_coefs;
	int m_illuminationModel;

public:

	using mat_key = std::pair<TextureType, Texture>;

public:

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void setCoefficients(const MaterialCoefs& coefs)		noexcept { m_coefs = coefs; }
	void setModel(const int model)							noexcept { m_illuminationModel = model; }
	
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	template<TextureType T>
	[[nodiscard]] const Texture*		queryTexture()	  const noexcept
	{
		if (!this || !m_textures.contains(T)) return nullptr;
		return &m_textures.at(T);
	}


	[[nodiscard]] const MaterialCoefs&	getCoefficients() const noexcept	{ return m_coefs; }

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void loadTextures(const std::initializer_list<std::pair<std::string, TextureType>>& textures);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Material(std::initializer_list<mat_key>&&);
	Material();	
	void swap(Material&) noexcept;
	Material(const Material&) = default;
	Material& operator=(const Material&);
	Material(Material&&) noexcept;
	Material& operator=(Material&&) noexcept;	
	~Material() = default;
};