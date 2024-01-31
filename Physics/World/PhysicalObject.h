#pragma once
#ifndef PHYSICALOBJECT_H
#define PHYSICALOBJECT_H
#include "../../Graphics/World/Mesh.h"
#include <memory>
#include <string>
#include "../physx_Impl/physx_shape.h"
#include <PxPhysicsAPI.h>
#include "../PhysicEngine.h"
#include <DirectXMath.h>




class PhysicalObject
{
public:
	using fVec3 = PhysicsEngine::fVec3;
	using group = PhysicsEngine::FilterGroup::Enum;
protected:
	static int count;
	std::string id;

protected:

	Transform m_transform; // Now holds it's own transform instead of using the mesh's one
	Mesh* m_mesh = nullptr;

	PhysicsEngine::Actor* m_actor = nullptr;
	group mFilterGroup
		= group::eOther;
	group mMaskGroup
		= group::eOther;

	PhysicalObject() : id{ std::string("PhysicalObject_") + std::to_string(++count) } {};
	PhysicalObject(group _filterGroup,	group _maskGroup)
		: id{ std::string("PhysicalObject_") + std::to_string(++count) }, 
			mFilterGroup{ _filterGroup }, mMaskGroup{ _maskGroup }
	{};
public:
	virtual ~PhysicalObject() {
		if (m_actor)
		{
			m_actor = nullptr;
		}
		if (m_mesh)
		{
			m_mesh = nullptr;
		}
	
	}
	bool operator==(const PhysicalObject& other) const { return id == other.id; }

	///////////////////////////////////////////////////////////////////////////////////////////////////////
	/// We assume getting the mesh is for rendering purpose only, so we update its transform when we query it.
	/// ....
	const Mesh* getMesh() const { 
		Transform& t = const_cast<Transform&>(m_mesh->getTransform());
		t = m_transform;
		return m_mesh;
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////
	virtual void setMesh(Mesh* mesh);

	virtual void updateTransform() = 0;
	virtual Transform& getTransform() { 
		//static Transform t{};  
		//return (m_mesh != nullptr) ? m_mesh->getTransform() : t;
		return m_transform;
	}
	
	std::string& getId() { return id; }

	virtual void addShape(PxShape* shape) {
		if (!shape) return;
		PxFilterData filterData;
		filterData.word0 = mFilterGroup;
		filterData.word1 = mMaskGroup;
		shape->setQueryFilterData(filterData);
		if(m_actor)
			m_actor->attachShape(*shape);
	}

	virtual void setName(const std::string name) {
		id = name;
		m_actor->setName(id.c_str());
		
	}
	virtual std::string getPhysxName() const  {
		return m_actor->getName();
	}

//	//Constructeur par mouvement
//PhysicalObject(PhysicalObject&& other)
//		: m_transform{ std::exchange(other.m_transform, {})},
//			m_mesh{ std::exchange(other.m_mesh, nullptr) }, 
//			m_actor{ std::exchange(other.m_actor, nullptr) },
//			mFilterGroup{ std::exchange(other.mFilterGroup, {})},
//			mMaskGroup{ std::exchange(other.mMaskGroup, {})},
//			id{ std::exchange(other.id, {}) }
//	{
//	}

};
#endif // !PHYSICALOBJECT_H
