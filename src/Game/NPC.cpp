#include "NPC.h"
#include <../../Graphics/World/Managers/MeshManager.h>
#include <string>
#include <algorithm>
int NPC::nextId = 1;

NPC::NPC() :
	m_object{},
	maxVelocity{ 200.f },
	id{nextId++}
{
	MeshManager::loadMeshFromFile("NPC", "res/mesh/Game/boule.obj");
	auto m = MeshManager::getMeshReference("NPC").get();
	m_object.setMesh(m);

	m_object.addShape(physx_shape::getBall(XMVectorGetX(m_object.getTransform().getScale()),
				0.2f, 0.2f, 1.f));
	name = std::string("NPC_") + std::to_string(id);
	m_object.setName(name);
	m_object.setUserData(new int(id));
	m_object.setMass(3.f);
	m_object.setMaxLinearVelocity(maxVelocity);
	m_object.setMaxAngularVelocity(50.0f);
	m_object.setLinearVelocity({ 0.5f,0.5f,0.5f });
	m_object.setAngularVelocity({ 0.5f,0.5f,0.5f });
}


void NPC::swap(NPC& other) noexcept
{
	using std::swap;
	swap(m_mesh, other.m_mesh);
	swap(m_object, other.m_object);
	swap(name, other.name);
	swap(m_trace, other.m_trace);
	swap(id, other.id);
}

NPC::NPC(NPC&& other) noexcept :
	m_mesh(std::exchange(other.m_mesh, {})),
	m_object(std::exchange(other.m_object, {})),
	name(std::exchange(other.name, {})),
	m_trace{std::exchange(other.m_trace,  {})},
	id {std::exchange(other.id, {})}
{	
	m_object.setName(name);
	//std::cout << "NPC moved, id is " << id <<"position : " << m_object.getPosition().x << " " << m_object.getPosition().z << "\n";
}

NPC& NPC::operator=(NPC&& other) noexcept
{
	NPC{ std::move(other) }.swap(*this);
	return *this;
}

void NPC::update()
{
	
	if(isRespawn)
	{
		isRespawn = false; 
		m_object.clearForce();
		m_object.clearTorque();
		m_object.setLinearVelocity(PhysicalObject::fVec3(0.f, 0.f, 0.f));
		std::cout << "NPC : " << id << " respawned\n";
		updateBeforeTP = 750;
		return;
	}
	else if (isUpdatable) {
		nbUpdate++;
		auto pos = m_object.getTransform().getPosition();
		const auto& courbe = m_trace.getControle();

		if (m_nextPoint >= courbe.size()) return;
		auto point = courbe[m_nextPoint];
		auto distance = XMVectorGetX(XMVector3Length(point - pos));
		if (distance < 50) {
			m_nextPoint = (m_nextPoint + 1);
			if (m_nextPoint >= courbe.size()) return;
			point = courbe[m_nextPoint];
			updateBeforeTP = 750;
			std::cerr << "NPC : "  << id << " next point is " << m_nextPoint << "\n";
		}

		if (updateBeforeTP < 10) {
			setPosition(XMVectorGetX(point), XMVectorGetY(point), XMVectorGetZ(point));
			updateBeforeTP = 750;
		}
		else {
			auto direction = point - pos;
			auto forward = PhysicalObject::fVec3(DirectX::XMVectorGetX(direction),
				0,
				DirectX::XMVectorGetZ(direction)).getNormalized();
			PhysicalObject::fVec3 rotationAxis = forward.cross(m_object.getPosition());

			m_object.addTorque(-rotationAxis * 10);
			m_object.addForce(forward * 10);

		}
		updateBeforeTP--;
	}
	m_object.updateTransform();
}

void NPC::setPosition(float x, float y, float z)
{
	isRespawn = true;

	m_object.setTranslation(x, y, z);
	m_object.updateTransform();

	auto pos = DirectX::XMVECTOR{ x, y, z };
	float dist = 1000.f;
	for (const auto& point : m_trace.getControle()) {
		auto trydist = XMVectorGetX(DirectX::XMVector3Length(point - pos));
		if (std::abs(trydist) < dist) {
			dist = trydist;
			m_nextPoint = static_cast<int>(std::distance(&m_trace.getControle()[0], &point));
		}
	}
	
}

void NPC::addBoost(float coeffBoost)
{
	auto v = m_object.getLinearValocity();
	v = v.multiply(PhysicsEngine::fVec3(0, 2, coeffBoost*1.15f));
	v.y = std::abs(v.y);
	m_object.setLinearVelocity(v);
}

void NPC::enableUpdate()
{
	isUpdatable = true;
}

void NPC::disableUpdate()
{
	isUpdatable = false;
}