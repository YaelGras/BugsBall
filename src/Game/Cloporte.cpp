#include "Cloporte.h"

#include "CameraController.h"
#include "Renderer.h"
#include "../../Graphics/abstraction/Camera.h"
#include "../../Platform/WindowsEngine.h"
#include "World/Managers/MeshManager.h"

#include "../../Physics/PhysicEngine.h"
#include "../../Physics/physx_impl/physx_shape.h"

extern std::unique_ptr<Mouse> wMouse;
extern std::unique_ptr<Keyboard> wKbd;

CameraController controller;

Cloporte::Cloporte()
	: m_object{ PhysicsEngine::FilterGroup::ePlayer, PhysicsEngine::FilterGroup::eAll }
	, maxVelocity(200.0f)
	, currentVelocity{0}
	, accelerationFactor(0.1f)
	, m_position{0, 0, 0, 0}
	, m_forward {0, 0, 1 , 0}
	, m_groundDir{0, 1, 0, 0}

{
	m_currentCam = &m_thirdPerson;

	m_thirdPerson.setProjection<PerspectiveProjection>(PerspectiveProjection{});
	m_firstPerson.setProjection<PerspectiveProjection>(PerspectiveProjection{});


	MeshManager::loadMeshFromFile("Cloporte", "res/mesh/Game/boule.obj");
	m_object.setMesh(MeshManager::getMeshReference("Cloporte").get());
	m_object.addShape(physx_shape::getBall(XMVectorGetX(m_object.getTransform().getScale()),
		0.2f, 0.2f, 1.f));
	m_object.setName("Cloporte");
	m_object.setUserData(new int(0));

	m_boundingSphere = BoundingSphere(m_position, XMVectorGetX(m_object.getTransform().getScale()));

	//Set max velocity
	m_object.setMaxLinearVelocity(maxVelocity);
	m_object.setMaxAngularVelocity(100.0f);

	//Set Velocity
	m_object.setLinearVelocity({ 0.5f,0.5f,0.5f });
	m_object.setAngularVelocity({ 0.5f,0.5f,0.5f });
	m_object.setMass(3.f);
	m_object.enableCCDSpeculative();
}

void Cloporte::switchView() noexcept
{
	m_isThirdPerson = false;
	if (m_currentCam == &m_thirdPerson) m_currentCam = &m_firstPerson;
	else
	{
		m_currentCam = &m_thirdPerson;
		m_isThirdPerson = true;
	}
}

void Cloporte::update(float deltaTime)
{

	if( m_handleInputs ) handleKeyboardInputs(deltaTime);
	updatePosition(deltaTime);
	m_boundingSphere.origin = m_position;
	if(!m_camsAreLocked) CameraController::computeThirdPersonPosition(*this, m_thirdPerson);
	if(!m_camsAreLocked) CameraController::computeFirstPersonPosition(*this, m_firstPerson);

	getCurrentCamera().updateCam();

}

void Cloporte::updatePosition(float deltaTime)
{
	m_position = m_object.getTransform().getPosition();
	m_object.updateTransform();

}

void Cloporte::handleKeyboardInputs(float deltaTime)
{
	DirectX::XMVECTOR m_positionDelta{};
	if (wKbd->isKeyPressed(GameInputs::queryDirectionInput(Direction::FORWARD)))
	{

		PhysicsEngine::fVec3 forward { XMVectorGetX(m_forward), XMVectorGetY(m_forward), XMVectorGetZ(m_forward) };
		PhysicalObject::fVec3 rotationAxis = forward.cross(m_object.getPosition());

		m_object.addTorque(-rotationAxis * 10);
		m_object.addForce(forward * 10);

	}

	if (wKbd->isKeyPressed(GameInputs::queryDirectionInput(Direction::BACKWARD)))
	{

		PhysicsEngine::fVec3 forward{ XMVectorGetX(m_forward), XMVectorGetY(m_forward), XMVectorGetZ(m_forward) };
		physx::PxVec3 rotationAxis = forward.cross(m_object.getPosition());

		m_object.addTorque(rotationAxis * 20);
		//m_object.addForce(-forward * 10);
	}

	if (wKbd->isKeyPressed(GameInputs::queryDirectionInput(Direction::LEFT)))
	{

		m_forward = XMVector3Rotate(m_forward, XMQuaternionRotationAxis({ 0,1,0 }, -0.05f));

	}

	if (wKbd->isKeyPressed(GameInputs::queryDirectionInput(Direction::RIGHT)))
	{

		m_forward = XMVector3Rotate(m_forward, XMQuaternionRotationAxis({ 0.f,1.f,0.f }, 0.05f));

	}
	// stupid workaround
	static bool tmp = false;
	if (wKbd->isKeyPressed(GameInputs::queryCommandInput(Command::POV)))
	{
		if (!tmp)
		{			
			switchView();
			tmp = true;
		}
	}

	if (!wKbd->isKeyPressed(GameInputs::queryCommandInput(Command::POV)))
	{
		tmp = false;

	}

}

void Cloporte::setTranslation(float x, float y, float z)
{
	m_object.clearForce();
	m_object.clearTorque();
	m_object.setTranslation(x, y, z);
	m_object.getTransform().setTranslation(x, y, z);
	
	m_groundDir = { 0, 1, 0, 0 };
	currentVelocity = { 0 };
	m_object.setLinearVelocity({ 0,0,0 });
	m_object.setAngularVelocity({ 0,0,0 });
	m_position = DirectX::XMVECTOR{ x,y,z, 1 };


}

void Cloporte::addBoost(float coeffBoost)
{
	auto v = m_object.getLinearValocity();
	v = v.multiply(PhysicsEngine::fVec3(coeffBoost, 1.f, coeffBoost*1.05));
	v.z = std::clamp(v.z, -120.f, -80.f);
	std::cout << "Boost en z " << v.z << std::endl;
	m_object.setLinearVelocity(v);
}

void Cloporte::setPosition(float x, float y, float z) {
	m_object.clearForce();
	m_object.clearTorque();
	m_object.getTransform().setTranslation(x, y, z);
	m_object.majTransformPhysics();
	m_position = DirectX::XMVECTOR{ x,y,z, 1 };

}