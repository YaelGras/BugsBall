#include "Camera.h"

#include <memory>
#include <algorithm>
#include <complex>
#include <iostream>


using namespace DirectX;

float PerspectiveProjection::s_baseAspectRatio = 16.f / 9.f;;


/////////////////////////////////////////////////////////////////////////////////////////////////////////
/// -- Basic operations

void Camera::updateCam() { computeVPMatrix(); }

void Camera::move(const DirectX::XMVECTOR& delta) { m_position += delta; }
void Camera::setPosition(const DirectX::XMVECTOR& pos) { m_position = pos; }

void Camera::rotate(const float dx /*= 0.0F*/, const float dy /*= 0.0f*/ , const float dz/*=0.0F*/ )
{
	m_angles.yaw += dx;
	m_angles.pitch = std::clamp(m_angles.pitch + dy, -DirectX::XM_PI * .499f, DirectX::XM_PI * .499f);
}

void Camera::lookAt(const DirectX::XMVECTOR& target)
{
	const DirectX::XMVECTOR delta = XMVector4Normalize(m_position-target);
	m_angles.pitch = std::asin(XMVectorGetY(delta));
	m_angles.yaw = std::atan2(-XMVectorGetX(delta), -XMVectorGetZ(delta));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/// -- Matrices and projection


void Camera::computeVPMatrix()
{
	computeViewMatrix();
	m_viewProjMatrix = m_viewMatrix * m_projection->getProjMatrix();
}

void Camera::computeViewMatrix() {
	m_viewMatrix = XMMatrixIdentity();
	m_viewMatrix = m_viewMatrix * XMMatrixTranslationFromVector(-m_position);
	m_viewMatrix = m_viewMatrix * XMMatrixRotationAxis({ 0,1,0,1 }, -m_angles.yaw);
	m_viewMatrix = m_viewMatrix * XMMatrixRotationAxis({ 1,0,0,1 }, -m_angles.pitch);
	m_viewMatrix = m_viewMatrix * XMMatrixRotationAxis({ 0,0,1,1 }, -m_angles.roll);
}

Mat Camera::getVPMatrix() { computeVPMatrix(); return m_viewProjMatrix; }
Mat Camera::getVPMatrix() const { return m_viewProjMatrix; }
Mat Camera::getProjMatrix() const { return m_projection->getProjMatrix(); }


/////////////////////////////////////////////////////////////////////////////////////////////////////////
/// -- Directions and position

[[nodiscard]] DirectX::XMVECTOR Camera::getPosition() const noexcept { return m_position; }
[[nodiscard]] DirectX::XMVECTOR Camera::getUp() const noexcept { return XMVector4Normalize(XMVector3Cross(getHorizontalDir(), getForward())); }
[[nodiscard]] DirectX::XMVECTOR Camera::getForward() const  noexcept {
	const double cy = cos(m_angles.yaw), sy = sin(m_angles.yaw);
	const double cp = cos(m_angles.pitch), sp = sin(m_angles.pitch);
	return { -static_cast<float>(sy * cp), static_cast<float>(sp), static_cast<float> (-cy * cp) };
}
[[nodiscard]] DirectX::XMVECTOR Camera::getHorizontalDir() const noexcept {
	return XMVector4Normalize(XMVector3Cross(getForwardDir(), m_up));
}
[[nodiscard]] DirectX::XMVECTOR Camera::getForwardDir() const noexcept {
	return XMVector4Normalize(getForward());
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////
/// -- Constructors, move and stuff

void Camera::swap(Camera& other) noexcept
{
	std::swap(m_position, other.m_position);
	std::swap(m_up, other.m_up);
	std::swap(m_left, other.m_left);
	std::swap(m_rotationSpeed, other.m_rotationSpeed);
	std::swap(m_angles, other.m_angles);
	std::swap(m_viewProjMatrix, other.m_viewProjMatrix);
	std::swap(m_viewMatrix, other.m_viewMatrix);
}
Camera::Camera(const Camera& other)
	: m_position(other.m_position)
	, m_up(other.m_up)
	, m_left(other.m_left)
	, m_angles(other.m_angles)
	, m_viewProjMatrix(other.m_viewProjMatrix)
	, m_viewMatrix(other.m_viewMatrix)
	, m_rotationSpeed(other.m_rotationSpeed)
{

}
Camera& Camera::operator=(const Camera& other)
{
	Camera{ other }.swap(*this);
	return *this;
}

Camera::Camera(Camera&& other) noexcept
	: m_position(std::exchange(other.m_position, {}))
	, m_up(std::exchange(other.m_up, {}))
	, m_left(std::exchange(other.m_left, {}))
	, m_angles(std::exchange(other.m_angles, {}))
	, m_viewProjMatrix(std::exchange(other.m_viewProjMatrix, {}))
	, m_viewMatrix(std::exchange(other.m_viewMatrix, {}))
	, m_rotationSpeed(std::exchange(other.m_rotationSpeed, 1.f))
	, m_projection(std::exchange(other.m_projection, nullptr))
{

}
Camera& Camera::operator=(Camera&& other) noexcept
{
	Camera{ std::move(other) }.swap(*this);
	return *this;
}

