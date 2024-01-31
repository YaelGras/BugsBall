#include "LightManager.h"

hlsl_GenericLight LightManager::spotToGeneric(const SpotLight& sl)
{
	return hlsl_GenericLight{

		sl.direction,
		{sl.insideAngle,sl.insideAngle,sl.insideAngle,sl.insideAngle},
		sl.position,
		sl.ambiant,
		sl.diffuse,

		sl.specularFactor,
		sl.outsideAngle,
		sl.strength,
		static_cast<float>(sl.isOn),
		static_cast<uint32_t>(sl.type)
	};
}

hlsl_GenericLight LightManager::pointToGeneric(const PointLight& pl)
{
	return hlsl_GenericLight{

		{},
		pl.range,
		pl.position,
		pl.ambiant,
		pl.diffuse,
		pl.specularFactor,
		{},
		{},
		static_cast<float>(pl.isOn),
		static_cast<uint32_t>(pl.type)

	};
}

hlsl_GenericLight LightManager::dirToGeneric(const DirectionalLight& dl)
{
	return hlsl_GenericLight{

		dl.direction,
		{},
		{},
		dl.ambiant,
		dl.diffuse,
		{},
		{},
		dl.strength,
		static_cast<float>(dl.isOn),
		static_cast<uint32_t>(dl.type)
	};
}

std::vector<hlsl_GenericLight> LightManager::convertLights() const noexcept
{
	std::vector<hlsl_GenericLight> res;
	for (const SpotLight& sl : m_spots)
		res.push_back(spotToGeneric(sl));
		
	for (const PointLight& pl : m_point)
		res.push_back(pointToGeneric(pl));
		

	for (const DirectionalLight& dl : m_dirs)
		res.push_back(dirToGeneric(dl));

	for (int i = 0; i < 32 - static_cast<int>(res.size());++i)
		res.push_back(hlsl_GenericLight{});
	return res;
		
}

std::vector<std::string> LightManager::getAllLightsDebugName() const
{

	std::vector<std::string> res;

	for (int i = 0; i < m_point.size(); ++i)
	{
		res.push_back("Pointlight " + std::to_string(i));
	}

	for (int i = 0; i < m_dirs.size(); ++i)
	{
		res.push_back("Directionallight " + std::to_string(i));
	}

	for (int i = 0; i < m_spots.size(); ++i)
	{
		res.push_back("Spotlight " + std::to_string(i));
	}
	return res;
}
