#pragma once
#ifndef LANDSCAPEPHYSICS
#define LANDSCAPEPHYSICS

#include "StaticObject.h"

class Terrain;

class PhysicalHeightMap : public StaticObject
{
private:
	const Terrain* terrain = nullptr; // don't hold a unique pointer that steal ownership
public :
	PhysicalHeightMap() : StaticObject() { m_actor->setName("Landscape"); 

	//if (m_actor->is<PxRigidBody>())
	//	m_actor->is<PxRigidBody>()->setRigidBodyFlag(PxRigidBodyFlag::eENABLE_SPECULATIVE_CCD, true);
	};
	virtual void setMesh(Mesh*) override {};

	void setTerrain(const Terrain* _terrain);

	virtual ~PhysicalHeightMap() override final {
		std::cout << "PhysicalHeightMap has been destroyed !" << std::endl;
	}
};


#endif // !LANDSCAPEPHYSICS