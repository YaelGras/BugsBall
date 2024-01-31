#include "Trace.h"
#include <cmath>
#include "../../Platform/IO/JsonParser.h"
#include "../../Physics/World/TriggerBox.h"
Trace::Trace(std::vector<FormatJson>& _controle)
{
    for (auto& item : _controle)
    {
		addControle(item.positionObj);
	}
}

void Trace::setControle(std::vector<FormatJson>& _controle)
{
    for (auto& item : _controle)
    {
        addControle(item.positionObj);
    }
}

void Trace::addControle(DirectX::XMVECTOR position)
{
	m_controle.emplace_back(position);
#ifndef NDEBUG
    m_trigger.push_back(new TriggerBox{
            {DirectX::XMVectorGetX(position), DirectX::XMVectorGetY(position), DirectX::XMVectorGetZ(position)},
            { 3.f, 3.f, 3.f } });
    m_trigger.back()->setName("Controle");
#endif
}
/*
void Trace::generateCourbe() noexcept
{

    auto [T, tToEval]= echantillonnageDistance(20.0f);
    interpolationLagrange(T, tToEval);
}

void Trace::afficherControle() const noexcept
{
    for (const auto& c : m_controle) {
        std::cout << "Controle : " << DirectX::XMVectorGetX(c) << " " << DirectX::XMVectorGetY(c) << " " << DirectX::XMVectorGetZ(c) << std::endl;
    }
}

std::pair<std::vector<float>, std::vector<float>> Trace::echantillonnageDistance(float pas)
{
    afficherControle();
    // Vecteur des pas temporels
    std::vector<float> T{};
    // Echantillonage des pas temporels
    std::vector<float> tToEval{};

    // Construction des pas temporels
    T.emplace_back(0.f);
    float distTot = 0;
    for (std::size_t i = 1; i < m_controle.size(); i++)
    {
        auto distanceX = std::pow(DirectX::XMVectorGetX(m_controle[i - 1]) - DirectX::XMVectorGetX(m_controle[i]), 2);
        auto distanceY = std::pow(DirectX::XMVectorGetY(m_controle[i - 1]) - DirectX::XMVectorGetY(m_controle[i]), 2);
        auto distanceZ = std::pow(DirectX::XMVectorGetZ(m_controle[i - 1]) - DirectX::XMVectorGetZ(m_controle[i]), 2);


        float value = static_cast<float>(std::sqrt(distanceX + distanceY + distanceZ));
        distTot += value;

        T.emplace_back(distTot);

    }
    // Construction des échantillons
    for (int i = 0; i < static_cast<int>(distTot / pas); i++)
    {
        tToEval.emplace_back(pas * static_cast<float>(i));
    }

   
    std::cout << "Echantillonnage distance de la courbe : " << tToEval.size() << " points pour une distance de " << distTot << std::endl;
    return std::make_pair(T, tToEval);
}


std::pair<std::vector<float>, std::vector<float>> Trace::echantillonnageRegulier(float pas) {
    afficherControle();
    // Vecteur des pas temporels
    std::vector<float> T{};
    // Echantillonage des pas temporels
    std::vector<float> tToEval{};

    for (std::size_t i = 0; i < m_controle.size(); i++)
    {
        T.emplace_back(i);

    }

    for (int i = 0; i < static_cast<float>(m_controle.size() - 1) / pas; i++)
    {
        tToEval.emplace_back(pas * i);
    }
    std::cout << "Echantillonnage regulier de la courbe : " << tToEval.size() << " points" << std::endl;
    return std::make_pair(T, tToEval);
}

// --------------------------------------------- BEZIER -------------------------------------------------

void Trace::Bezier(std::vector<float>& tToEval)
{    
    m_courbe.clear();
    for (int i = 0; i < tToEval.size(); ++i)
    {

        float t = tToEval[i] / (m_controle.size() - 1);

        m_courbe.emplace_back(DeCasteljau(t));

#ifndef NDEBUG
        m_trigger.push_back(new TriggerBox{ 
                {DirectX::XMVectorGetX(m_courbe.back()), DirectX::XMVectorGetY(m_courbe.back()), DirectX::XMVectorGetZ(m_courbe.back())},
                { 0.5f, 0.5f, 0.5f } });
        m_trigger.back()->setName("Courbe");
#endif

    }    
}

DirectX::XMVECTOR Trace::DeCasteljau(float t)
{
    float Px = 0;
    float Py = 0;
    float Pz = 0;
    int n = m_controle.size() - 1;
    for (int k = 0; k <= n; k++)
    {
        float bern = KparmiN(k, n) * std::pow(1 - t, n - k) * std::pow(t, k);
        Px += DirectX::XMVectorGetX(m_controle[k]) * bern;
        Py += DirectX::XMVectorGetY(m_controle[k]) * bern;
        Pz += DirectX::XMVectorGetZ(m_controle[k]) * bern;
    }
    return DirectX::XMVECTOR{ Px, Py, Pz };
}

long Trace::KparmiN(int k, int n)
{
    long result = 1;
    for (int i = 1; i <= k; i++)
    {
        result *= n - (k - i);
        result /= i;
    }
    return (long)result;
}



// ----------------------------------------LAGRANGE------------------------------------------------------

void Trace::interpolationLagrange(std::vector<float> T, std::vector<float> tToEval)
{
    std::vector<float> controleX{};
    std::vector<float> controleY{};
    std::vector<float> controleZ{};
    for (int i = 0; i < m_controle.size(); i++)
    {
		controleX.emplace_back(DirectX::XMVectorGetX(m_controle[i]));
		controleY.emplace_back(DirectX::XMVectorGetY(m_controle[i]));
		controleZ.emplace_back(DirectX::XMVectorGetZ(m_controle[i]));
	}


    for (int i = 0; i < tToEval.size(); ++i)
    {
        // Calcul de xpoint et ypoint
        float t = tToEval[i];

        float xpoint = lagrange(t, T, controleX);
        float ypoint = lagrange(t, T, controleY);
        float zpoint = lagrange(t, T, controleZ);

        m_courbe.emplace_back(DirectX::XMVECTOR{ xpoint, ypoint, zpoint });

#ifndef NDEBUG
        m_trigger.push_back(new TriggerBox{
                {DirectX::XMVectorGetX(m_courbe.back()), DirectX::XMVectorGetY(m_courbe.back()), DirectX::XMVectorGetZ(m_courbe.back())},
                { 0.5f, 0.5f, 0.5f } });
        m_trigger.back()->setName("Courbe");
#endif
    }
}

float Trace::lagrange(float tToEval, std::vector<float> T, std::vector<float> controle)
{
    float result = 0.f;
    for (int i = 0; i < T.size(); i++)
    {
        float xi = T[i];
        float Li = 1.f;
        for (int j = 0; j < T.size(); j++)
        {
            if (i != j)
            {
                float xj = T[j];
                Li = Li * (tToEval - xj) / (xi - xj);
            }
        }
        result += Li * controle[i];
    }

    return result;
}
*/
/*
{
        "j_position": {
            "x": 180,
            "y": 55,
            "z": 720,
            "n": 20
          }
    },
    {
        "j_position": {
            "x": 180,
            "y": 60,
            "z": 540,
            "n": 25
          }
    },
    {
      "j_position": {
          "x": 347,
          "y": 60,
          "z": 436,
          "n": 25
        }
  },
  {
      "j_position": {
          "x": 338,
          "y": 50,
          "z": 124,
          "n": 25
        }
    },
    {
        "j_position": {
            "x": 876,
            "y": 50,
            "z": 473,
            "n": 25
          }
    }


*/