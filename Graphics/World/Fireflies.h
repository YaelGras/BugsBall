#pragma once
#include <random>
#include "Billboard.h"



struct Firefly
{
	XMVECTOR position;
	XMVECTOR velocity, velocityVar;
	XMVECTOR color;
	float size = 1;
};

class Fireflies
{

private:


	BillboardRenderer m_renderer;
	std::vector<Firefly> m_fireflies;
	Texture m_flyTexture{ "res/textures/breadbug_alpha.dds" }; // :)

	std::uniform_int_distribution<std::mt19937::result_type> m_randomDistribution;
	std::mt19937 m_prng;


public:


	Fireflies();

	void update(float deltaTime, XMVECTOR centerPos = { 0,0,0 });

	float zeroToOne();

	std::vector<Billboard> getFireflies() const;

};
