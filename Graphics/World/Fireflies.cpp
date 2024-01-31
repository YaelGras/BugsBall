#include "Fireflies.h"

Fireflies::Fireflies()
{

	m_prng.seed(std::random_device()());
	m_fireflies.resize(200);
	std::ranges::for_each(m_fireflies,
		[&, i = 0](Firefly& f) mutable
		{
			//f.position = XMVECTOR{ 0,(float)i, 10 };
			f.color = XMVECTOR{ 50 + (zeroToOne() * 2 - 1) * 50,200 + (zeroToOne() * 2 - 1) * 50, 0, 1 };

			i++;
		}
	);
}

void Fireflies::update(float deltaTime, XMVECTOR centerPos /*= { 0,0,0 }*/)
{
	static float RADIUS_RANGE = 60;
	static XMVECTOR RADIUS = { RADIUS_RANGE,RADIUS_RANGE,RADIUS_RANGE };
	for (Firefly& f : m_fireflies)
	{

		f.color = XMVECTOR{ 50 + (zeroToOne() * 2 - 1) * 50,100 + (zeroToOne() * 2 - 1) * 50, 0, 1 };

		f.velocityVar = XMVECTOR{ zeroToOne() , zeroToOne(), zeroToOne() } *2 - XMVECTOR{ 1,1, 1 };
		f.velocityVar /= 30;
		f.velocity += f.velocityVar;
		f.velocity = XMVectorClamp(f.velocity, XMVECTOR{ -1,-1,-1 }, XMVECTOR{ 1,1,1 });
		f.position += f.velocity;
		f.position = centerPos + XMVector3ClampLength(f.position - centerPos, 0, RADIUS_RANGE);
	}

}

float Fireflies::zeroToOne()
{
	return (float)m_randomDistribution(m_prng) / (float)(std::numeric_limits<uint32_t>::max)();
}

std::vector<Billboard> Fireflies::getFireflies() const
{
	std::vector<Billboard> res;
	static Material m_fireflyMat{ {TextureType::ALBEDO, m_flyTexture} };
	std::ranges::for_each(m_fireflies,
		[&](const Firefly& f)
		{
			res.emplace_back(Billboard{
				f.position, f.color, {f.size,f.size,f.size,f.size},
				&m_fireflyMat
				}
			);
		}
	);
	std::ranges::reverse(res);
	return res;
}
