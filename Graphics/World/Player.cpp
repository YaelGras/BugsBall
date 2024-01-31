#include "Player.h"


#include <optional>
#include "../../Platform/WindowsEngine.h"
#include "../../Utils/Vec2.h"


using namespace DirectX;
// This is the default FPS player.
// Windows specific

static Vec2<int> winSize;
static HWND hWnd;

Player::Player()
{
	m_position = { 0,0,0 };
	m_speed = 0.5f;
	m_camera.setProjection<PerspectiveProjection>(PerspectiveProjection());
	//m_camera.setProjection<OrthographicProjection>({-10,10,-10,10,0.1,20});

	hWnd = WindowsEngine::getInstance().getHwnd();
	winSize = Vec2<int>(WindowsEngine::getInstance().getGraphics().getWinSize());
}


void  Player::lockInputs(bool state)
{
	m_inputsAreLocked = state;
}

void Player::step(float deltaTime)
{
	if (!m_inputsAreLocked)
	{
		handleMouseEvents();
		handleKeyboardEvents();
	}

	m_camera.updateCam();
}

void Player::onImGuiRender()
{
	ImGui::Begin("Player debug");
	ImGui::DragFloat("Player speed", &m_speed, 0.05f, 0, 1.F);
	ImGui::End();
}


void Player::handleKeyboardEvents() {

	const XMVECTOR camForward = m_camera.getForwardDir();
	const XMVECTOR camHorizontal = m_camera.getHorizontalDir();

	const XMVECTOR forwardDir = XMVector3Normalize(	XMVECTOR{
		XMVectorGetX(camForward),
		0,
		XMVectorGetZ(camForward) });

	const XMVECTOR horizontalDir = XMVector3Normalize(XMVECTOR{
		XMVectorGetX(camHorizontal),
		0,
		XMVectorGetZ(camHorizontal) });


	Keyboard::Event e = wKbd->readKey();

	if (wKbd->isKeyPressed(VK_SPACE)) {
		m_positionDelta = XMVectorSetY(m_positionDelta, XMVectorGetY(m_positionDelta) + m_speed);
	}
	else if (wKbd->isKeyPressed(VK_SHIFT)) {
		m_positionDelta = XMVectorSetY(m_positionDelta, XMVectorGetY(m_positionDelta) - m_speed);
	}

	if (wKbd->isKeyPressed(Keyboard::letterCodeFromChar('q')) || wKbd->isKeyPressed(Keyboard::letterCodeFromChar('a'))) {
		m_positionDelta = XMVectorSetZ(m_positionDelta, XMVectorGetZ(m_positionDelta) - XMVectorGetZ(horizontalDir)*m_speed);
		m_positionDelta = XMVectorSetX(m_positionDelta, XMVectorGetX(m_positionDelta) - XMVectorGetX(horizontalDir)*m_speed);
	}

	else if (wKbd->isKeyPressed(Keyboard::letterCodeFromChar('d'))) {
		m_positionDelta = XMVectorSetZ(m_positionDelta, XMVectorGetZ(m_positionDelta) + XMVectorGetZ(horizontalDir)*m_speed);
		m_positionDelta = XMVectorSetX(m_positionDelta, XMVectorGetX(m_positionDelta) + XMVectorGetX(horizontalDir)*m_speed);
	}

	if (wKbd->isKeyPressed(Keyboard::letterCodeFromChar('z')) || wKbd->isKeyPressed(Keyboard::letterCodeFromChar('w')))
	{
		m_positionDelta = XMVectorSetZ(m_positionDelta, XMVectorGetZ(m_positionDelta) - XMVectorGetZ(forwardDir)*m_speed);
		m_positionDelta = XMVectorSetX(m_positionDelta, XMVectorGetX(m_positionDelta) - XMVectorGetX(forwardDir)*m_speed);
	}

	else if (wKbd->isKeyPressed(Keyboard::letterCodeFromChar('s'))) {
		m_positionDelta = XMVectorSetZ(m_positionDelta, XMVectorGetZ(m_positionDelta) + XMVectorGetZ(forwardDir)*m_speed);
		m_positionDelta = XMVectorSetX(m_positionDelta, XMVectorGetX(m_positionDelta) + XMVectorGetX(forwardDir)*m_speed);
	}

	m_camera.setPosition(m_positionDelta);
}

void Player::handleMouseEvents() {
	Mouse::Event me;

	while ((me = wMouse->read()).isValid())
	{
		if (me.getType() == Mouse::Event::Type::RDOWN) {

			if (m_cursorIsConfined) {
				wMouse->freeCursor();
				wMouse->enableCursor();

			}
			else {
				wMouse->confineCursor(hWnd);
				wMouse->disableCursor();
			}

			m_cursorIsConfined = !m_cursorIsConfined;
		}
	}

	wMouse->flush();
	float dx = 0, dy = 0;
	while (const auto rawDelta = wMouse->readRawdelta()) {

		if (m_cursorIsConfined) {
			dx += static_cast<float>(rawDelta->x);
			dy += static_cast<float>(rawDelta->y);
		}
	}

	float mouseDx = dx / static_cast<float>(winSize.x) * DirectX::XM_PI;
	float mouseDy = dy / static_cast<float>(winSize.y) * DirectX::XM_PI;
	m_camera.rotate(mouseDx, mouseDy);

}
