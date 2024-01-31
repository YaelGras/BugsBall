#pragma once

#include <PxPhysicsAPI.h>
#include "../PhysicEngine.h"
#include "../../Graphics/World/Mesh.h"
#define _XM_NO_INTRINSICS_
using namespace physx;
class physx_shape
{
public:
	static PxShape* getCube(const float staticFriction,
							const float dynamicFriction,
							const float restitution,
							const PhysicsEngine::fVec3 scale = PhysicsEngine::fVec3{ 1,1,1 });

	static PxShape* getCube(const PhysicsEngine::fVec3 scale = PhysicsEngine::fVec3{ 1,1,1 });
	
	static PxShape* getBall(const float scale = 1.0f,
							const float staticFriction = 0.5f,
							const float dynamicFriction = 0.5f,
							const float restitution = 0.6f);
	static PxShape* getHeightmap(	const PhysicsEngine::HeightMapData& data,
									const float staticFriction = 0.5f,
									const float dynamicFriction = 0.5f,
									const float restitution = 0.6f);
	static PxShape* getCapsule(	const float halfHeight,
								const float radius,	
								const float staticFriction = 0.5f,
								const float dynamicFriction = 0.5f,
								const float restitution = 0.6f);

	static PxShape* getTriangleMesh(const Mesh* data,
									const PhysicsEngine::fVec3& scale = PhysicsEngine::fVec3{ 1,1,1 },
									const float staticFriction = 0.5f,
									const float dynamicFriction = 0.5f,
									const float restitution = 0.6f);
};

