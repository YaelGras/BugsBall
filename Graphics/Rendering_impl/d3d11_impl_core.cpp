#include "direct3D11_impl.h"

direct3D11_impl::direct3D11_impl()
{
	WindowsEngine& engine = WindowsEngine::getInstance();
	d3d11_graphics& gfx = engine.getGraphics();
	device = gfx.getDevice();
	context = gfx.getImmediateContext();
	swapChain = gfx.getSwapChain();
	rtv = gfx.getRenderTargetView();
	m_pp.uav = gfx.getContext().postProcessUAV;
	m_pp.srv = gfx.getContext().postProcessSRV;
	m_pp.rtv = gfx.getContext().postProcessRTV;
	depth = gfx.getDepthBuffer().getView();

	baseMesh.loadEffectFromFile("res/effects/baseMesh.fx");
	blit.loadEffectFromFile("res/effects/blit.fx");


	InputLayout layout, debuglayout;
	layout.pushBack<3>(InputLayout::Semantic::Position);
	layout.pushBack<3>(InputLayout::Semantic::Normal);
	layout.pushBack<2>(InputLayout::Semantic::Texcoord);
	baseMesh.bindInputLayout(layout);

	baseMesh.addNewCBuffer("worldParams", sizeof(worldParams));
	baseMesh.addNewCBuffer("meshParams", sizeof(XMMATRIX));

	debugLine.loadEffectFromFile("res/effects/debugLine.fx");
	debuglayout.pushBack<3>(InputLayout::Semantic::Position);
	debugLine.bindInputLayout(debuglayout);
	debugLine.addNewCBuffer("worldParams", sizeof(XMMATRIX));


}



 void direct3D11_impl::clearScreen()   {

	d3d11_graphics& gfx = WindowsEngine::getInstance().getGraphics();
	gfx.clearDepth();
	gfx.clearFramebuffer();
	m_textRenderer.clear();

}

 void direct3D11_impl::clearDepth() {

	 d3d11_graphics& gfx = WindowsEngine::getInstance().getGraphics();
	 gfx.clearDepth();

 }

 void direct3D11_impl::setBackbufferToDefault()   {

	d3d11_graphics& gfx = WindowsEngine::getInstance().getGraphics();
	context->OMSetRenderTargets(1, &gfx.getContext().rtv, gfx.getDepthBuffer().getView());
	gfx.bindBlendState();
}

 void direct3D11_impl::setDepthBuffer(ID3D11DepthStencilView* other)   {

	depth = other;
}


 void direct3D11_impl::clearScreen(float r, float g, float b, float a)   {
	const FLOAT rgba[4] = { r,g,b,a };
	d3d11_graphics& gfx = WindowsEngine::getInstance().getGraphics();
	context->ClearRenderTargetView(gfx.getContext().rtv, rgba);
	context->ClearRenderTargetView(gfx.getContext().postProcessRTV, rgba);
	clearScreen();
	m_allLines.clear();
}

 void direct3D11_impl::drawIndexed(size_t count, uint32_t startIndexLocation, uint32_t baseVertexLocation)  {

	context->DrawIndexed(static_cast<UINT>(count), startIndexLocation, baseVertexLocation);
}

 void direct3D11_impl::draw(size_t count)  {

	context->Draw(static_cast<UINT>(count), 0);
}

 void direct3D11_impl::clearText()
{
	 m_textRenderer.clear();
}

 void direct3D11_impl::cleanupAfterSceneDeletion()
 {
	 m_textRenderer.clear();
	 m_quadRenderer.clear();
 }