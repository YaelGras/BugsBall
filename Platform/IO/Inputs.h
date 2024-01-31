#pragma once

#include <bitset>
#include <queue>
#include <optional>
#include <windows.h>
#include "imgui.h"


/*https://www.youtube.com/watch?v=h7HCdEyGRRw&list=PLqCJpWy5Fohd3S7ICFXwUomYW0Wv67pDD&index=9*/

static constexpr int BUFFER_SIZE = 32;


class Keyboard {

public:

	static char letterCodeFromChar(const char c) {
		return 0x41 + (c-'a');
	}

	class Event 
	{
	public:
		enum class Type {PRESS,RELEASE, INVALID};
		Event()									noexcept : m_type(Type::INVALID), m_code(0)  {}
		Event(Type type, unsigned char code)	noexcept : m_type(type), m_code(code) {}

		[[nodiscard]] bool isPress()			const noexcept	{ return m_type == Type::PRESS;		}
		[[nodiscard]] bool isRelease()			const noexcept	{ return m_type == Type::RELEASE;	}
		[[nodiscard]] bool isInvalide()			const noexcept	{ return m_type == Type::INVALID;	}
		[[nodiscard]] unsigned char getCode()	const noexcept	{ return m_code;					}
	private:

		Type m_type;
		unsigned char m_code;


	};

	Keyboard() = default;
	Keyboard(const Keyboard&) = delete;
	Keyboard& operator=(const Keyboard&) = delete;


	enum {KEY_COUNT=256u};
	bool isKeyPressed(const char keyCode) const noexcept { return m_keyStates[keyCode]; }
	Event readKey() {
		if (m_keyBuffer.size() == 0) return Keyboard::Event();
		Keyboard::Event e = m_keyBuffer.front();
		m_keyBuffer.pop();
		return e;
	}
	bool keyIsEmpty() const { return m_keyBuffer.empty(); }
	bool flushKey() { m_keyBuffer = std::queue<Event>(); }

	char readChar() {
		if (m_charBuffer.size() == 0) return 0;
		char c = m_charBuffer.front();
		m_charBuffer.pop();
		return c;
	}

	bool charIsEmpty() const { return m_charBuffer.empty(); }
	void flushChar() {
		m_charBuffer = std::queue<char>();
	}
	void flush() {
		flushKey();
		flushChar();
	}

	void enableAutorepeat()				noexcept { m_autorepeat = true; }
	void disableAutorepeat()			noexcept { m_autorepeat = false; }
	bool autorepeatIsEnable()	const	noexcept { return m_autorepeat == true; }

private:

	std::bitset<KEY_COUNT> m_keyStates;
	std::queue<Event> m_keyBuffer;
	std::queue<char> m_charBuffer;

	bool m_autorepeat = false;
public:
	void onKeyPressed(unsigned char code) noexcept {
		m_keyStates[code] = true;
		m_keyBuffer.push(Keyboard::Event{ Event::Type::PRESS, code });
		trimBuffer(m_keyBuffer);
	}

	void onKeyReleased(unsigned char code) noexcept {
		m_keyStates[code] = false;
		m_keyBuffer.push(Keyboard::Event{ Event::Type::RELEASE, code });
		trimBuffer(m_keyBuffer);
	}

	void onChar(char c) noexcept {
		m_charBuffer.push(c);
		trimBuffer(m_charBuffer);
	}

	void clearStates() { m_keyStates.reset(); }

	template<typename _T> 
	void trimBuffer(std::queue<_T>& buffer)
	{
		while (buffer.size() > 16) buffer.pop();
	}


};

class Mouse {

public:
	struct RawDelta { int x; int y; };

	class Event
	{
	public:
		enum class Type { 
			LDOWN, LPRESS, LRELEASE,
			RDOWN, RPRESS, RRELEASE,
			WHEELUP, WHEELDOWN,
			MOVE, INVALID };

		Event()									noexcept : m_type(Type::INVALID) {}
		Event(Type type, const Mouse& parent)	noexcept 
			: m_type(type), x(parent.m_x), y(parent.m_y),
			  isLeftDown(parent.m_isLeftDown), isRightDown(parent.m_isRightDown)
		{}

		[[nodiscard]] Type getType()			const noexcept { return m_type ; }
		[[nodiscard]] bool isValid()			const noexcept { return m_type != Type::INVALID; }
		[[nodiscard]] std::pair<int, int>	getPos()			const noexcept { return { x,y }; }
		[[nodiscard]] int					getPosX()			const noexcept { return x; }
		[[nodiscard]] int					getPosY()			const noexcept { return y; }
		[[nodiscard]] bool					leftIsDown()		const noexcept { return isLeftDown; }
		[[nodiscard]] bool					rightIsDown()		const noexcept { return isRightDown; }
	private:

		Type m_type;
		int x{}, y{};
		bool isLeftDown=false, isRightDown=false;


	};


	Mouse() = default;
	Mouse(const Mouse&) = delete;
	Mouse& operator=(const Mouse&) = delete;

	[[nodiscard]] std::pair<int, int>	getPos()			const noexcept { return { m_x,m_y };		}
	[[nodiscard]] int					getPosX()			const noexcept { return m_x;				}
	[[nodiscard]] int					getPosY()			const noexcept { return m_y;				}
	[[nodiscard]] bool					isLeftDown()		const noexcept { return m_isLeftDown;		}
	[[nodiscard]] bool					isRightDown()		const noexcept { return m_isRightDown;		}
	[[nodiscard]] bool					isRightPressed()	const noexcept { return m_isRightPressed;	}
	[[nodiscard]] bool					isLeftPressed()		const noexcept { return m_isLeftPressed;	}
	[[nodiscard]] bool					isEmpty()			const noexcept { return m_buffer.empty();	}

	[[nodiscard]] Event					read()					  noexcept { 
		if (isEmpty()) return Mouse::Event();
		Event& e = m_buffer.front();
		return (m_buffer.pop(), e);
	}

	[[nodiscard]] std::optional<RawDelta>	readRawdelta()	noexcept {
		if (m_rawDeltas.empty()) return std::nullopt;
		RawDelta e = m_rawDeltas.front();
		return (m_rawDeltas.pop(), e);
	}

	void flush() noexcept{ m_buffer = std::queue<Event>(); }

private:


	static constexpr unsigned short buffSize = BUFFER_SIZE;
	std::queue<Event> m_buffer;
	std::queue<RawDelta> m_rawDeltas;
	int m_x, m_y;
	bool m_isLeftDown, m_isRightDown;
	bool m_isLeftPressed, m_isRightPressed;
	bool m_isHidden = false;

public:

	void onMouseMoved	(int x, int y)	{ m_x = x; m_y = y; m_buffer.emplace(Event(Event::Type::MOVE, *this)); trimBuffer(); }
	void onLeftPress	()	{ m_isLeftDown  = true; m_buffer.emplace(Event(Event::Type::LDOWN, *this)); trimBuffer(); }
	void onRightPress	()	{ m_isRightDown	= true; m_buffer.emplace(Event(Event::Type::RDOWN, *this)); trimBuffer(); }


	void onLeftRelease	(int x, int y)
	{
		if (m_isLeftDown)
		{
			m_isLeftPressed = true;
			m_buffer.emplace(Event(Event::Type::LPRESS, *this));
		}
		m_isLeftDown  = false;
		m_buffer.emplace(Event(Event::Type::LRELEASE, *this)); trimBuffer();
	}
	void onRightRelease	(int x, int y)
	{
		if (m_isRightDown)
		{
			m_isRightPressed = true;
			m_buffer.emplace(Event(Event::Type::RPRESS, *this));
		}
		m_isRightDown	= false;
		m_buffer.emplace(Event(Event::Type::RRELEASE, *this)); trimBuffer();
	}


	void onWheelUp		(int x, int y)	{ m_buffer.emplace(Event(Event::Type::WHEELUP, *this)); trimBuffer();}
	void onWheelDown	(int x, int y)	{ m_buffer.emplace(Event(Event::Type::WHEELDOWN, *this)); trimBuffer();}
	void onRawDelta		(int x, int y)	{ m_rawDeltas.push(RawDelta{x,y}); trimBuffer(); }
	void trimBuffer		(){
	
		while (m_buffer.size() > buffSize)
			m_buffer.pop();

		while (m_rawDeltas.size() > buffSize)
			m_rawDeltas.pop();
	
	}

	void clearPress()
	{
		m_isLeftPressed = false;
		m_isRightPressed = false;
	}

	void enableCursor() { 
		m_isHidden = true; 
		while (::ShowCursor(TRUE) < 0);
		ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
	}
	void disableCursor() { 
		m_isHidden = false;
		while(::ShowCursor(FALSE) >= 0);
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
	}

	void confineCursor(HWND hWnd) {
		RECT rect;
		GetClientRect(hWnd, &rect);
		rect.left = rect.right / 2;
		rect.top = rect.bottom / 2;
		rect.right = rect.left + 1;
		rect.bottom = rect.top + 1;
		MapWindowPoints(hWnd, nullptr, reinterpret_cast<POINT*>(&rect), 2);
		ClipCursor(&rect);
	}

	void freeCursor() {
		ClipCursor(nullptr);
	}


	/* Confines and hides cursor on first call, frees on the second */
	void toggleCursorFocus(HWND hWnd) {


		if (m_isHidden) {
			freeCursor();
			enableCursor();
		}
		else {
			confineCursor(hWnd);
			disableCursor();
		}

	}

};

