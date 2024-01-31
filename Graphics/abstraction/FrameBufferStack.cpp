#include "FrameBufferStack.h"
#include "Framebuffer.h"
#include "Renderer.h"


void FrameBufferStack::pushFBO(const FrameBuffer* fbo)
{
	m_stack.push(fbo);
}


void FrameBufferStack::rebind() const 
{
	if (!m_stack.empty()) m_stack.top()->bindCached();
}

void FrameBufferStack::popFBO()
{
	m_stack.pop();

	if (m_stack.empty()) {
		Renderer::setBackbufferToDefault();
		return;
	}

	m_stack.top()->bindCached(); // bind without pushing to the stack

}

void FrameBufferStack::resetStack()
{
	while (!m_stack.empty())
		m_stack.pop();
}