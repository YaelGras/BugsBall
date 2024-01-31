#include "CameraController.h"

const Terrain* CameraController::terrain = nullptr;

inline float smoothstep(float a, float b, float x)
{
	const float t = std::clamp((x - a) / (b - a), 0.F, 1.f);
	return t * t * (3.0f - (2.0f * t));
}

void CameraController::computeThirdPersonPosition(const Cloporte& player, Camera& cam)
{
	static constexpr float distanceFromPlayer = 25.F;
	const auto forward = player.getForward();
	const auto pos = player.getPosition();

	// Compute camera roll
	cam.setRoll(computeCameraRoll(player, cam));

	// Find camera's height
	const float computedHeight = std::clamp(computeCameraHeight(player), -30.f, 30.f);
	XMVECTOR camPos = pos - (distanceFromPlayer * forward) + XMVECTOR{0.F, -computedHeight, 0.F};

	// Lerp
	//static const float MAX_DISTANCE = 20.f;
	camPos = XMVectorLerp(
		cam.getPosition(),
		camPos, 0.05f
		//smoothstep(0, MAX_DISTANCE, XMVectorGetX(XMVector3Length(camPos - cam.getPosition())))
	);

	// Set
	cam.setPosition(camPos);
	cam.lookAt(pos);
	cam.updateCam();
}

float CameraController::computeCameraRoll(const Cloporte& player, const Camera& cam)
{
	const auto forwardDir = player.getForward();
	const auto groundNormal = player.getGroundDir();

	const auto u = XMVector3Cross(forwardDir, XMVECTOR{0.F, 1.F, 0.f});
	const auto v = XMVector3Cross(forwardDir, u);

	// le plan (u , v) a pour normale forward;
	// on projette groundDir sur le plan

	const auto projectedPlane_x = XMVectorGetX(XMVector3Dot(groundNormal, u));
	const auto projectedPlane_y = XMVectorGetX(XMVector3Dot(groundNormal, v));

	// On applique l'arctan pour avoir l'angle
	const float roll = std::atan2f(projectedPlane_y, projectedPlane_x);

	// Lerp
	const float old_roll = cam.getRoll();
	const float computedRoll = std::lerp(
		old_roll,
		roll + XM_PIDIV2,
		.05f
	);

	return computedRoll;
}

float CameraController::computeCameraHeight(const Cloporte& player)
{
	const auto forwardDir = player.getForward();
	const auto playerPosition = player.getPosition();

	// sample height in front and check height difference
	static constexpr float STEP_LENGTH = 5;
	const auto posInFront = playerPosition + forwardDir * STEP_LENGTH;
	const float trueHeight = terrain->getWorldHeightAt(posInFront);
	const float delta = trueHeight - XMVectorGetY(posInFront); // > 0 si le point est sous la map, 

	return delta;
}

void CameraController::computeFirstPersonPosition(const Cloporte& player, Camera& cam)
{
	cam.setPosition(player.getPosition());
	cam.lookAt(player.getForward() + player.getPosition());
	cam.updateCam();
}
