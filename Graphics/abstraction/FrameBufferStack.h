#pragma once

#include <stack>


class FrameBuffer;

class FrameBufferStack
{
private:
	std::stack<const FrameBuffer*> m_stack;

	// singleton stuff
	FrameBufferStack(const FrameBufferStack&) = delete;
	void operator=(const FrameBufferStack&) = delete;
	FrameBufferStack() {}

public:
	static FrameBufferStack& getInstance() {
		static FrameBufferStack singleton;
		return singleton;
	}

	// to be called when binding an fbo
	void pushFBO(const FrameBuffer* fbo);
	// to be called when unbinding an fbo
	void popFBO();
	/// Call this to rebind current stack top fbo
	void rebind() const ; 
	size_t getCount() const { return m_stack.size(); }
	void resetStack();

};
