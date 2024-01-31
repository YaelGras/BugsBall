#include "UIButton.h"


bool UIButton::isHovered(int mouseX, int mouseY) const
{
	return (mouseX > x && mouseX < x + width) &&
		(mouseY > y && mouseY < y + height);
}

bool UIButton::isClicked(int mouseX, int mouseY, bool pressed) const
{
	return (pressed) && isHovered(mouseX, mouseY);
}

void UIButton::render(Renderer2D& renderer)
{
	static Texture baseTexture{ "res/textures/template.dds" };
	static Texture hoveredTexture{ "res/textures/template_hovered.dds" };
	const Texture& currentTexture = (hovered) ? hoveredTexture : baseTexture;
	renderer.batchQuad({ (float)x,(float)y }, { (float)width, (float)height }, currentTexture);
}
