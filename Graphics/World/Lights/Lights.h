#pragma once


#define _XM_NO_INTRINSICS_
#include <directXmath.h>
#include <algorithm>

using namespace DirectX;

using Vec = XMVECTOR;


inline Vec computeRangeFromDistance(unsigned int distance);

enum class LightType : int32_t
{
	DIRECTIONAL,
	POINT,
	SPOTLIGHT,
};

struct Light
{
	bool isOn=false;
	LightType type;
};

struct hlsl_GenericLight
{

	Vec		direction; // For directional lights and spotlight
	Vec		range;
	Vec		position;
	Vec		ambiant;
	Vec		diffuse;

	float specularFactor;
	float fallOff;
	float strength;
	float isOn;

	uint32_t type;

	float padding[3];

};

struct DirectionalLight : public Light {

	Vec direction;
	Vec ambiant;
	Vec diffuse;
	float strength;
	LightType type = LightType::DIRECTIONAL;
private:

	float padding[2] = {};
};


struct PointLight : public Light {

	int distance;
	Vec range;
	Vec position;
	Vec ambiant;
	Vec diffuse;
	float specularFactor;
	LightType type = LightType::POINT;
	PointLight() = default;
	PointLight(unsigned int d, Vec pos, Vec Ka, Vec Kd, float f, bool on)
	{
		distance = d;
		range = computeRangeFromDistance(distance);
		position=pos;
		ambiant = Ka;
		diffuse = Kd;
		specularFactor = f;
		isOn = on;
	}


	float padding[2];

};

struct SpotLight : public  Light{

	Vec direction;
	Vec ambiant;
	Vec diffuse;
	Vec position;
	LightType type = LightType::SPOTLIGHT;

	float outsideAngle;
	float insideAngle;
	float strength;
	float specularFactor;
};


Vec computeRangeFromDistance(unsigned int distance)
{
	static Vec lookup[] =
	{
		// dist  const  linear   quad
		{7		,1.0f	,0.7f	,1.8f},
		{13		,1.0f	,0.35f	,0.44f},
		{20		,1.0f	,0.22f	,0.20f},
		{32		,1.0f	,0.14f	,0.07f},
		{50		,1.0f	,0.09f	,0.032f},
		{65		,1.0f	,0.07f	,0.017f},
		{100	,1.0f	,0.045f	,0.0075f},
		{160	,1.0f	,0.027f	,0.0028f},
		{200	,1.0f	,0.022f	,0.0019f},
		{325	,1.0f	,0.014f	,0.0007f},
		{600	,1.0f	,0.007f	,0.0002f},
		{3250	,1.0f	,0.0014f,0.000007f}
	};


	return lookup[std::clamp(distance, 0u, 11u)];

}