#include "PhysicalObject.h"

int PhysicalObject::count = 0;

void PhysicalObject::setMesh(Mesh* mesh)
{
	//m_mesh.reset(mesh);
	m_mesh = mesh;
	if (m_actor) {
		Transform t = m_mesh->getTransform();

		auto pos_Render = t.getTranslation();
		auto ang_Render = t.getAngles();
		fVec3 position{ DirectX::XMVectorGetX(pos_Render),  DirectX::XMVectorGetY(pos_Render),  DirectX::XMVectorGetZ(pos_Render) };
		fVec3 rotation{ DirectX::XMVectorGetX(ang_Render),  DirectX::XMVectorGetY(ang_Render),  DirectX::XMVectorGetZ(ang_Render) };
		

		// Convertir les angles en radians si ce n'est pas déjà le cas
		float angleX = DirectX::XMConvertToRadians(rotation.x);
		float angleY = DirectX::XMConvertToRadians(rotation.y);
		float angleZ = DirectX::XMConvertToRadians(rotation.z);

		// Calculer les demi-angles
		float halfAngleX = angleX * 0.5f;
		float halfAngleY = angleY * 0.5f;
		float halfAngleZ = angleZ * 0.5f;

		// Calculer les sin et cos des demi-angles
		float sinHalfX = sin(halfAngleX);
		float cosHalfX = cos(halfAngleX);
		float sinHalfY = sin(halfAngleY);
		float cosHalfY = cos(halfAngleY);
		float sinHalfZ = sin(halfAngleZ);
		float cosHalfZ = cos(halfAngleZ);

		// Calculer les composantes du quaternion
		float x = sinHalfX * cosHalfY * cosHalfZ - cosHalfX * sinHalfY * sinHalfZ;
		float y = cosHalfX * sinHalfY * cosHalfZ + sinHalfX * cosHalfY * sinHalfZ;
		float z = cosHalfX * cosHalfY * sinHalfZ - sinHalfX * sinHalfY * cosHalfZ;
		float w = cosHalfX * cosHalfY * cosHalfZ + sinHalfX * sinHalfY * sinHalfZ;

		// Construire le quaternion
		PxQuat quaternion(x, y, z, w);
		m_actor->setGlobalPose(PxTransform(position, quaternion));
		m_transform = t;
	}
}