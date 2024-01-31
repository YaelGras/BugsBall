#pragma once

#define _XM_NO_INTRINSICS_
#include <directXmath.h>
#include "../../Graphics/abstraction/Camera.h"
#include "World/Mesh.h"
#include "../../Physics/World/DynamicObject.h"
#include "../../Platform/IO/GameInputs.h"

class Cloporte
{

private:


	DynamicObject m_object;
	BoundingSphere m_boundingSphere; // use this for collision check, and mesh aabb for culling

	Camera m_firstPerson;
	Camera m_thirdPerson;
	Camera* m_currentCam;

	float maxVelocity;
	float currentVelocity;
	float accelerationFactor;

	DirectX::XMVECTOR m_position	;
	DirectX::XMVECTOR m_forward		;
	DirectX::XMVECTOR m_groundDir	;

	bool m_handleInputs = true;
	bool m_isThirdPerson = true;
	bool m_camsAreLocked = true;

public:

	Cloporte();
	Cloporte(const Cloporte&) = delete;
	Cloporte& operator=(const Cloporte&) = delete;
	Cloporte(Cloporte&&) = delete;
	Cloporte& operator=(Cloporte&&) = delete;
	~Cloporte() = default;

private:

	void updatePosition(float deltaTime);

public:
	void switchView() noexcept;
	void update(float deltaTime);

public:

	[[nodiscard]] bool currentViewIsThirdPerson()		const noexcept	{ return m_isThirdPerson;		}
	[[nodiscard]] Camera& getCurrentCamera()				  noexcept	{ return *m_currentCam;			}
	[[nodiscard]] Camera& getThirdPersonCam()				  noexcept	{ return m_thirdPerson;			}
	[[nodiscard]] Camera& getFirstPersonCam()				  noexcept	{ return m_firstPerson;			}
	[[nodiscard]] const DynamicObject& getObject()		const noexcept	{ return m_object;				}
	[[nodiscard]] BoundingSphere getBoundingSphere()	const noexcept	{ return m_boundingSphere;		}
	[[nodiscard]] DirectX::XMVECTOR getForward()		const noexcept	{ return m_forward;				}
	[[nodiscard]] DirectX::XMVECTOR getPosition()		const noexcept	{ return m_position;			}
	[[nodiscard]] DirectX::XMVECTOR getGroundDir()		const noexcept	{ return m_groundDir;			}
	[[nodiscard]] float getMaxVelocity()				const noexcept	{ return maxVelocity;			}
	              const Mesh& getMesh()						  noexcept	{ return *(m_object.getMesh()); }


	void setPlayable(bool canPlay) { m_handleInputs = canPlay; }
	void setGroundVector(const DirectX::XMVECTOR& val) {	m_groundDir = val;	}
	void setPosition(float x, float y, float z);
	void setTranslation(float x, float y, float z);
	void setForward(DirectX::XMVECTOR forward) { m_forward = forward; }
	void addBoost(float boost);
	void lockCameras(bool lock = false) { m_camsAreLocked = lock; }
private:
	void handleKeyboardInputs(float deltaTime);

};

