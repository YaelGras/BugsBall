#pragma once

#include <stdexcept>
#include <vector>

#include "UIButton.h"

#include "../Platform/IO/Inputs.h"
#include "abstraction/2D/Renderer2D.h"



class UIRenderer
{
	static std::vector<std::unique_ptr<UIElement>> m_renderList;
	static Mouse* attachedMouse;

	static Renderer2D* m_renderer;


public:

	static void attachMouse(Mouse* mouse)
	{
		attachedMouse = mouse;
	}

	static bool Button(int x, int y, int w, int h)
	{
		if (!attachedMouse) throw std::runtime_error("You have not attached a mouse to the ui system");
		std::unique_ptr<UIButton> button = std::make_unique<UIButton>();
		button->x = x;
		button->y = y;
		button->width = w;
		button->height = h;
		button->hovered = button->isHovered(attachedMouse->getPosX(), attachedMouse->getPosY());
		bool res = button->isClicked(attachedMouse->getPosX(), attachedMouse->getPosY(), attachedMouse->isLeftPressed());
		m_renderList.push_back(std::move(button));
		return res;

	}


	static void clear()
	{
		m_renderList.clear();

	}
	static void renderUI()
	{
		if (m_renderer) {

			for (auto&& elem : m_renderList)
			{
				elem->render(*m_renderer);
			}
			m_renderer->renderBatchedQuads();
			m_renderer->clear();
			
		}
		
		clear();
		attachedMouse->clearPress();
	}

	static void init()
	{
		if (!m_renderer) m_renderer = new Renderer2D;
	}
	static void cleanUp()
	{
		delete m_renderer;
		m_renderer = nullptr;
	}
	
};
