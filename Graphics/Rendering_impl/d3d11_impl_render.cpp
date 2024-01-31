#include "direct3D11_impl.h"

#include "../abstraction/Texture.h"

void direct3D11_impl::renderMesh(const Camera& camera, const Mesh& mesh) {

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetInputLayout(baseMesh.getVertexLayout());


	worldParams sp;

	sp.viewProj = XMMatrixTranspose(camera.getVPMatrix());
	sp.lightPos = XMVectorSet(-10.0f, 10.0f, -10.0f, 1.0f);
	sp.cameraPos = camera.getPosition();
	sp.ambiantLight = XMVectorSet(.2f, 0.2f, 0.2f, 1.0f);
	sp.diffuseLight = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
	sp.ambiantMat = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
	sp.diffuseMat = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);

	baseMesh.updateConstantBuffer(sp, "worldParams");
	baseMesh.updateConstantBuffer(XMMatrixTranspose(mesh.getTransform().getTransformationMatrix()), "meshParams"); // TODO make this more flexible

	baseMesh.apply();
	mesh.draw();
	baseMesh.unbindResources();

	triangleCount += mesh.getTriangleCount();

}

void direct3D11_impl::renderMesh(const Camera& camera, const Mesh& mesh, const Effect& effect) {

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetInputLayout(effect.getVertexLayout());

	effect.updateConstantBuffer(XMMatrixTranspose(mesh.getTransform().getTransformationMatrix()), "meshParams"); // TODO make this more flexible

	effect.apply();
	mesh.draw();
	effect.unbindResources();

	triangleCount += mesh.getTriangleCount();

}


void direct3D11_impl::renderCubemap(Camera& camera, const Mesh& mesh, const Effect& effect) {


	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetInputLayout(effect.getVertexLayout());
	effect.apply();
	mesh.draw();
	effect.unbindResources();


}

void direct3D11_impl::blitTexture(const Texture& tex, const DirectX::XMVECTOR& colorShift){

	//Renderer::setBackbufferToDefault();
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	blit.bindTexture("tex", tex);
	blit.setUniformVector("colorShift", colorShift);
	blit.apply();
	Renderer::draw(6);
	blit.unbindTexture("tex");
	blit.unbindResources();
}

void direct3D11_impl::writeTextOnScreen(const std::string& text, int screenX, int screenY, float scale, DirectX::XMVECTOR color)
{
	m_textRenderer.writeTextOnScreen(text, screenX, screenY, scale, color);
}

void direct3D11_impl::renderText()
{
	m_textRenderer.render();
}

void direct3D11_impl::renderQuadOnScreen(DirectX::XMVECTOR position, DirectX::XMVECTOR size, const Texture& texture, DirectX::XMVECTOR uvoffset)
{
	m_quadRenderer.batchQuad(position, size, texture, uvoffset);
}

void direct3D11_impl::renderBatch2D()
{
	m_quadRenderer.renderBatchedQuads();
}
