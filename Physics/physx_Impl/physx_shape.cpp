#include "physx_shape.h"
#include "physx_impl.h"
#include <cassert>
#include <PxPhysicsAPI.h>

PxShape* physx_shape::getCube(const float staticFriction,
							  const float dynamicFriction,
							  const float restitution,
							  const PhysicsEngine::fVec3 scale)
{
	Physx_Impl::ModulePhysics& mp = Physx_Impl::getModulePhysics();
	PxPhysics* physics = mp.gPhysics;
	PxMaterial* material = physics->createMaterial(staticFriction, dynamicFriction, restitution);
    PxShape* shape = physics->createShape(PxBoxGeometry(scale.x, scale.y, scale.z), *material);
    return shape;
}

PxShape* physx_shape::getCube(const PhysicsEngine::fVec3 scale)
{
	Physx_Impl::ModulePhysics& mp = Physx_Impl::getModulePhysics();
	PxPhysics* physics = mp.gPhysics;
	PxMaterial* material = mp.gMaterial;
	PxShape* shape = physics->createShape(PxBoxGeometry(scale.x, scale.y, scale.z), *material);
	return shape;
}

PxShape* physx_shape::getBall(const float scale,
								float staticFriction,
								float dynamicFriction,
								float restitution)
{
	Physx_Impl::ModulePhysics& mp = Physx_Impl::getModulePhysics();
	PxPhysics* physics = mp.gPhysics;
	PxMaterial* material = physics->createMaterial(staticFriction, dynamicFriction, restitution);
    PxShape* shape = physics->createShape(PxSphereGeometry(scale), *material);
    return shape;
}

PxShape* physx_shape::getHeightmap(const PhysicsEngine::HeightMapData& data,
	float staticFriction,
	float dynamicFriction,
	float restitution)
{
	Physx_Impl::ModulePhysics& mp = Physx_Impl::getModulePhysics();
	PxPhysics* physics = mp.gPhysics;

	assert(data.value.size() == data.width * data.length);
	
	PxHeightFieldSample* hfSamples = new PxHeightFieldSample[data.length * data.width];
	
	PxU32 index = 0;
	for (PxU32 col = 0; col < data.length; col++)
	{
		for (PxU32 row = 0; row < data.width; row++)
		{
	
			PxHeightFieldSample& smp = hfSamples[(row * data.width) + col];
			smp.height = PxI16(data.value[(col * data.width) + row] * 255);
		}
	}
	
	// Build PxHeightFieldDesc from samples
	PxHeightFieldDesc terrainDesc;
	terrainDesc.format = PxHeightFieldFormat::eS16_TM;
	terrainDesc.nbColumns = data.length;
	terrainDesc.nbRows = data.width;
	terrainDesc.samples.data = hfSamples;
	terrainDesc.samples.stride = sizeof(PxHeightFieldSample);
	terrainDesc.flags = PxHeightFieldFlags();
	PxHeightField* aHeightField = PxCreateHeightField(terrainDesc,	physics->getPhysicsInsertionCallback());
	
	
	PxHeightFieldGeometry hfGeom;
	hfGeom.columnScale = data.scaleXZ;
	hfGeom.rowScale = data.scaleXZ;
	hfGeom.heightScale = data.scaleY / 255.f;
	hfGeom.heightField = aHeightField;
	hfGeom.heightFieldFlags = PxMeshGeometryFlag::eDOUBLE_SIDED;
	
	delete[] hfSamples;
	PxMaterial* material = physics->createMaterial(staticFriction, dynamicFriction, restitution);
	PxShape* shape = physics->createShape(hfGeom, *material);
	return shape;
}

PxShape* physx_shape::getCapsule(const float halfHeight,
	const float radius,
	const float staticFriction,
	const float dynamicFriction,
	const float restitution)
{
	Physx_Impl::ModulePhysics& mp = Physx_Impl::getModulePhysics();
	PxPhysics* physics = mp.gPhysics;
	PxMaterial* material = physics->createMaterial(staticFriction, dynamicFriction, restitution);
	PxShape* shape = physics->createShape(PxCapsuleGeometry(radius, halfHeight), *material);
	shape->setLocalPose(PxTransform(PxQuat(PxHalfPi, PxVec3(0, 0, 1))));
	return shape;
}

PxShape* physx_shape::getTriangleMesh(const Mesh* data, const PhysicsEngine::fVec3& scale, const float staticFriction, const float dynamicFriction, const float restitution)
{
	Physx_Impl::ModulePhysics& mp = Physx_Impl::getModulePhysics();
	auto* physics = mp.gPhysics;
	// Remplissage des structures de données PhysX à partir de m_vbo et m_ibo
	const VertexBuffer& m_vbo = data->getVertexBuffer();
	const IndexBuffer& m_ibo = data->getIndexBuffer();

	PxTriangleMeshDesc meshDesc;
	meshDesc.points.count = static_cast<PxU32>(m_vbo.getVerticesCount());
	meshDesc.points.stride = sizeof(PhysicsEngine::fVec3); // Chaque vertex a 3 coordonnées
	using fVec3 = PhysicsEngine::fVec3;
	std::vector<fVec3> points;
	std::ranges::for_each(m_vbo.getVertices(), [&points](const Vertex& v) {
			points.push_back(fVec3(DirectX::XMVectorGetX(v.position), 
			DirectX::XMVectorGetY(v.position),
			DirectX::XMVectorGetZ(v.position)));
		});
	meshDesc.points.data = points.data();

	meshDesc.triangles.count = static_cast<PxU32>(m_ibo.getIndicesCount() / 3); // Chaque triangle a 3 indices
	meshDesc.triangles.stride = 3 * sizeof(PxU32);

	std::vector<PxU32> index;
	std::ranges::for_each(m_ibo.getIndices(), [&index](const IndexBuffer::size_type& v) {
		index.push_back(static_cast<PxU32>(v));
		});

	meshDesc.triangles.data = index.data(); 

	PxTolerancesScale scaleTol;
	PxCookingParams params(scaleTol);
	// disable mesh cleaning - perform mesh validation on development configurations
	params.meshPreprocessParams |= PxMeshPreprocessingFlag::eDISABLE_CLEAN_MESH;
	// disable edge precompute, edges are set for each triangle, slows contact generation
	params.meshPreprocessParams |= PxMeshPreprocessingFlag::eDISABLE_ACTIVE_EDGES_PRECOMPUTE;
	// lower hierarchy for internal mesh
	

#ifndef NDEBUG
	// mesh should be validated before cooked without the mesh cleaning
	bool res = PxValidateTriangleMesh(params, meshDesc);
	PX_ASSERT(res);
#endif

	PxTriangleMesh* triangleMesh = PxCreateTriangleMesh(params, meshDesc,
		physics->getPhysicsInsertionCallback());

	PxMaterial* material = physics->createMaterial(staticFriction, dynamicFriction, restitution);
	PxShape* s =  physics->createShape(PxTriangleMeshGeometry(triangleMesh, PxMeshScale(scale)), *material);
	return s;
}
