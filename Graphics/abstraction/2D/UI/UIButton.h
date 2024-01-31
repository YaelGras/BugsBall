#pragma once


// should be a x,y,size with behaviour on click
// should also highlight on hover
// should contain text


#include "UIElement.h"
#include "abstraction/2D/Renderer2D.h"

struct UIButton : public UIElement
{


	int x, y;
	int width, height;
	bool hovered = false;
	bool clicked = false;
	std::string pathTexture = "res/textures/template.dds";
	std::string pathTextureHovered = "res/textures/template_hovered.dds";

public:

	bool isHovered(int mouseX, int mouseY) const;

	bool isClicked(int mouseX, int mouseY, bool pressed) const;


	virtual void render(Renderer2D& renderer) override;
};
