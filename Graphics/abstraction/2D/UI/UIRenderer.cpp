#include "UIRenderer.h"


Mouse* UIRenderer::attachedMouse = nullptr;
std::vector<std::unique_ptr<UIElement>>UIRenderer::m_renderList {};

Renderer2D* UIRenderer::m_renderer = nullptr;