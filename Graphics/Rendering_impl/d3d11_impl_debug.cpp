#include "direct3D11_impl.h"

#include "../abstraction/Camera.h"



void direct3D11_impl::renderAABB(Camera& cam, const AABB& box) {
	Vec o = box.origin;
	Vec x = { XMVectorGetX(box.size), 0, 0 };
	Vec y = { 0, XMVectorGetY(box.size), 0 };
	Vec z = { 0, 0, XMVectorGetZ(box.size) };

	addDebugLine(o, o + x);
	addDebugLine(o + y, o + x + y);
	addDebugLine(o + z, o + x + z);
	addDebugLine(o + y + z, o + x + y + z);
	addDebugLine(o, o + y);
	addDebugLine(o + x, o + y + x);
	addDebugLine(o + z, o + y + z);
	addDebugLine(o + x + z, o + y + x + z);
	addDebugLine(o, o + z);
	addDebugLine(o + x, o + z + x);
	addDebugLine(o + y, o + z + y);
	addDebugLine(o + x + y, o + z + x + y);
}

void direct3D11_impl::renderDebugLine(Camera& cam) {

	setBackbufferToDefault();
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	context->IASetInputLayout(debugLine.getVertexLayout());


	debugLine.updateConstantBuffer(cam.getVPMatrix(), "worldParams");

	debugLine.apply();


	VertexBuffer vbo(m_allLines);
	vbo.bind();

	context->Draw(static_cast<UINT>(m_allLines.size()), 0);
	m_allLines.clear();

}

void direct3D11_impl::addDebugLine(Vec from, Vec to) {
	m_allLines.push_back(Vertex{ XMVectorSetW(from,1) });
	m_allLines.push_back(Vertex{ XMVectorSetW(to,1) });
}


void direct3D11_impl::renderDebugPerspectiveCameraOutline(Camera& viewCamera, const Camera& outlinedCamera) {

	auto proj = viewCamera.getProjection();
	float zfar = 1000.F;
	float znear = 0.10F;
	float fovy = 3.1415f / 4.F;
	float aspect = proj.aspectRatio;

	Vec pos = outlinedCamera.getPosition();
	Vec I = outlinedCamera.getHorizontalDir();
	Vec J = outlinedCamera.getUp();
	Vec F = outlinedCamera.getForward();
	float dh = sin(fovy * .5f);
	float dw = dh * aspect;
	Vec U1 = F + dh * J + dw * I;
	Vec U2 = F - dh * J + dw * I;
	Vec U3 = F - dh * J - dw * I;
	Vec U4 = F + dh * J - dw * I;
	float zNear = -znear;
	float zFar = -zfar;

	addDebugLine(pos, pos + F * zFar); // dir
	addDebugLine(pos, pos + Vec{ 0,1,0,1 }); // world up
	addDebugLine(pos, pos + I); // right
	addDebugLine(pos, pos + J); // up
	addDebugLine(pos, pos + U1 * zFar);
	addDebugLine(pos, pos + U2 * zFar);
	addDebugLine(pos, pos + U3 * zFar);
	addDebugLine(pos, pos + U4 * zFar);
	addDebugLine(pos + U1 * zFar, pos + U2 * zFar);
	addDebugLine(pos + U2 * zFar, pos + U3 * zFar);
	addDebugLine(pos + U3 * zFar, pos + U4 * zFar);
	addDebugLine(pos + U4 * zFar, pos + U1 * zFar);

}


void direct3D11_impl::renderDebugOrthographicCameraOutline(const Camera& viewCamera, const Camera& outlinedCamera) 
{
	const OrthographicProjection& proj = outlinedCamera.getProjection<OrthographicProjection>();
	const XMVECTOR  pos = outlinedCamera.getPosition();
	XMVECTOR I = outlinedCamera.getHorizontalDir();
	XMVECTOR J = outlinedCamera.getUp();
	XMVECTOR F = -outlinedCamera.getForwardDir();
	float zNear = proj.znear;
	float zFar = proj.zfar;
	XMVECTOR p1 = pos + I * proj.right + J * proj.top;
	XMVECTOR p2 = pos + I * proj.right - J * proj.top;
	XMVECTOR p3 = pos - I * proj.right - J * proj.top;
	XMVECTOR p4 = pos - I * proj.right + J * proj.top;

	addDebugLine(pos, pos + F * zFar); // dir
	addDebugLine(pos, pos + XMVECTOR {0, 1, 0}); // world up
	addDebugLine(pos, pos + I		); // right
	addDebugLine(pos, pos + J		); // up
	addDebugLine(p1, p1 + F * zFar	);
	addDebugLine(p2, p2 + F * zFar	);
	addDebugLine(p3, p3 + F * zFar	);
	addDebugLine(p4, p4 + F * zFar	);
	for (float z = zNear; z <= zNear; z += 5) {
		addDebugLine(p1 + z * F, p2 + z * F);
		addDebugLine(p2 + z * F, p3 + z * F);
		addDebugLine(p3 + z * F, p4 + z * F);
		addDebugLine(p4 + z * F, p1 + z * F);
	}
}


void direct3D11_impl::showImGuiDebugData() {
	ImGui::Begin("D3D11 Rendering debug data:");
	ImGui::Text("Current drawn triangle count : %d", triangleCount);
	ImGui::End();

	triangleCount = 0;

}

