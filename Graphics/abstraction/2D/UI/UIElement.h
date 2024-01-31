#pragma once

class Renderer2D;

class UIElement
{
protected:

	long long uuid;

public:


	virtual void render(Renderer2D&) {}



};
