#pragma once

#include <vector>

#include "abstraction/Texture.h"
#include "../Platform/WindowsEngine.h"
#include "VFX.h"
#include "TiltShift.h"
#include "RadialBlur.h"
#include "Bloom.h"
#include "SpeedLines.h"
#include "abstraction/Camera.h"
#include "abstraction/FrameBuffer.h"

enum class EffectType
{
	Bloom,
	TiltShift,
	RadialBlur,
	SpeedLines
};

class VFXPipeline
{
private:

	d3d11_graphics::RenderingContext m_context;
	std::vector<VFX*> m_effects;
	FrameBuffer* m_gBuffer = nullptr;
	Camera m_camera;

public:

	VFXPipeline()
	{
		m_context = WindowsEngine::getInstance().getGraphics().getContext();
		m_effects.push_back(new Bloom{m_context.postProcessUAV});
		m_effects.push_back(new TiltShift);
		m_effects.push_back(new RadialBlur);
		m_effects.push_back(new SpeedLines);

		ID3D11ShaderResourceView* nullSRVs[1] = { nullptr };
		m_context.context->PSSetShaderResources(0, 1, nullSRVs);
	}

	~VFXPipeline()
	{
		std::ranges::for_each(m_effects, [&](const VFX* v) {delete v; });
	}

	void setGbuffer(FrameBuffer* gBuffer)
	{
		m_gBuffer = gBuffer;
	}

	void setCamera(const Camera& cam)
	{
		m_camera = cam;
	}

	template<class V>
	V* queryVisualEffect(EffectType type) const
	{
		return reinterpret_cast<V*>(m_effects[static_cast<int>(type)]);
	}

	template <class E> requires std::derived_from<E, VFX>
	void registerEffect()
	{
		m_effects.push_back(new E);
	}

	void renderPipeline() const
	{

		for (VFX* effect : m_effects) {
			if (!effect->isEnabled()) continue;

			effect->setGbuffer(m_gBuffer);
			effect->setCamera(m_camera);
			effect->apply(m_context.postProcessUAV);
		}
			


	}


	void update(float deltaTime) const 
	{
		for (VFX* effect : m_effects) {
			effect->update(deltaTime);
		}
	}

	void showImguiDebugWindow()
	{
		ImGui::Begin("VFX Pipeline");
		if (ImGui::Button("Recompile shaders"))
		{
			std::ranges::for_each(m_effects, [&](VFX* v)
			{
				delete v;

			});
			m_effects.clear();
			m_effects.push_back(new Bloom{ m_context.postProcessUAV });
			m_effects.push_back(new TiltShift);
			m_effects.push_back(new RadialBlur);
			m_effects.push_back(new SpeedLines);
		}
		std::ranges::for_each(m_effects, [&](VFX* v) {v->showImguiDebugWindow(); });
		ImGui::End();


	}



};
