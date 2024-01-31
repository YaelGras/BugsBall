#pragma once
#include <vector>
#include <DirectXMath.h>

struct FormatJson;
class TriggerBox;
class Trace
{
	std::vector<DirectX::XMVECTOR> m_controle;
	//std::vector<DirectX::XMVECTOR> m_courbe;

#ifndef NDEBUG
	std::vector<TriggerBox*> m_trigger;
#endif

public:
	Trace() = default;
	Trace(std::vector<FormatJson>& _controle);;
	Trace(const Trace&) = default;
	Trace(Trace&&) = default;
	Trace& operator=(const Trace&) = default;
	Trace& operator=(Trace&&) = default;
#ifndef NDEBUG
	~Trace() = default;
#else
	~Trace() = default;
#endif
	void setControle(std::vector<FormatJson>& _controle);
	void addControle(DirectX::XMVECTOR position);
	void clearControle() noexcept { m_controle.clear(); }
	[[nodiscard]] const std::vector<DirectX::XMVECTOR>& getControle() const noexcept { return m_controle; }
	//[[nodiscard]] const std::vector<DirectX::XMVECTOR>& getCourbe() const noexcept { return m_controle; }

	/*void generateCourbe() noexcept;

private:
	std::pair<std::vector<float>, std::vector<float>> echantillonnageDistance(float pas);

	std::pair<std::vector<float>, std::vector<float>> echantillonnageRegulier(float pas);

    void Bezier(std::vector<float>& tToEval);

	DirectX::XMVECTOR DeCasteljau(float t);

	long KparmiN(int k, int n);

	void afficherControle() const noexcept;

	void interpolationLagrange(std::vector<float> T, std::vector<float> tToEval);

	float lagrange(float tToEval, std::vector<float> T, std::vector<float> controle);*/
};

