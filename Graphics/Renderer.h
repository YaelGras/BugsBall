#pragma once

#pragma warning( disable : 4244 ) 

#include <concepts>
#include <type_traits>
#include <memory>
#include <filesystem>
#define _XM_NO_INTRINSICS_
#include <DirectXMath.h>



// Creates static function for the renderer 
// that calls the implementation function of the same name
#define CALL_IMPL(fnName)\
static auto fnName(auto&& ...args)\
{\
	return implementation->##fnName(std::forward<decltype(args)>(args)...);\
}

// Forward declars


class Mesh;
class Effect;
class Camera;
class Material;
class Texture;
struct AABB;
struct ID3D11DepthStencilView;
class DeferredRenderer;

/// Renderer class :
/// 
/// It simply is an interface. It has a pointer to some implementation. All methods are static and call the 
/// method of the same name of the implementation.
/// 
/// Add new pure virtual methods in the _Impl struct, and don't forget to add CALL_IMPL in the Renderer class.
/// Set an implementation in main.cpp, such as d3d11_impl before calling Renderer:: functions

class Renderer
{

public:

	struct _Impl
	{
		friend class Renderer;
	public:


	protected:

		// -- Core

		virtual void drawIndexed(size_t count, uint32_t startIndexLocation, uint32_t baseVertexLocation) = 0;
		virtual void draw(size_t count) = 0;
		virtual void clearScreen(float, float, float, float) = 0;
		virtual void clearScreen() = 0;
		virtual void clearDepth() = 0;
		virtual void clearText() = 0;
		virtual void setBackbufferToDefault() = 0;
		virtual void setDepthBuffer(ID3D11DepthStencilView* other) = 0;
		virtual void cleanupAfterSceneDeletion() = 0;


		// -- Render

		virtual void renderMesh(const Camera&, const Mesh&, const Effect&) = 0;
		virtual void renderMesh(const Camera&, const Mesh&) = 0;
		virtual void renderCubemap(Camera&, const Mesh&, const Effect&) = 0;
		virtual void blitTexture(const Texture&, const DirectX::XMVECTOR & = { 1,1,1,1 }) = 0;
		virtual void writeTextOnScreen(const std::string& text, int screenX, int screenY, float scale, DirectX::XMVECTOR color = { 1,1,1,1 }) = 0;
		virtual void renderText() = 0;

		virtual void renderQuadOnScreen(DirectX::XMVECTOR, DirectX::XMVECTOR, const Texture&, DirectX::XMVECTOR = { 0,0,0,0 }) = 0;
		virtual void renderBatch2D() = 0;


		// -- Debug

		virtual void renderAABB(Camera& camera, const AABB&) = 0;
		virtual void renderDebugPerspectiveCameraOutline(Camera& viewCamera, const Camera& outlinedCamera) = 0;
		virtual void renderDebugOrthographicCameraOutline(const Camera& viewCamera, const Camera& outlinedCamera) = 0;
		virtual void renderDebugLine(Camera& cam) = 0;
		virtual void showImGuiDebugData() = 0;
		virtual void addDebugLine(DirectX::XMVECTOR from, DirectX::XMVECTOR to) = 0;

	};

	//----------------------------------------------//

	static void draw(size_t count) { implementation->draw(count); }
	static void drawIndexed(size_t count, uint32_t startIndexLocation, uint32_t baseVertexLocation) { implementation->drawIndexed(count, startIndexLocation, baseVertexLocation); }
	static void clearScreen(float r, float g, float b, float a) { implementation->clearScreen(r,g,b,a); }
	static void clearScreen() { implementation->clearScreen(); }
	static void clearDepth() { implementation->clearDepth(); }
	static void clearText() { implementation->clearText(); }
	static void setBackbufferToDefault() { implementation->setBackbufferToDefault(); }
	static void setDepthBuffer(ID3D11DepthStencilView* other) { implementation->setDepthBuffer(other); }
	static void cleanupAfterSceneDeletion() { implementation->cleanupAfterSceneDeletion(); }

	//----------------------------------------------//

	static void renderMesh(const Camera& cam, const Mesh& mesh, const Effect& effect) { implementation->renderMesh(cam, mesh, effect); }
	static void renderMesh(const Camera& cam, const Mesh& mesh) { implementation->renderMesh(cam, mesh); }
	static void renderCubemap(Camera& cam, const Mesh& cubeMesh, const Effect& effect) { implementation->renderCubemap(cam, cubeMesh, effect); }
	static void blitTexture(const Texture& toBlit, const DirectX::XMVECTOR& colorShift = { 1,1,1,1 }) { implementation->blitTexture(toBlit, colorShift); }
	static void writeTextOnScreen(const std::string& text, int screenX, int screenY, float scale, DirectX::XMVECTOR color = { 1,1,1,1 }) { implementation->writeTextOnScreen(text, screenX, screenY,scale,  color); }
	static void renderText() { implementation->renderText(); }
	static void renderQuadOnScreen(DirectX::XMVECTOR pos , DirectX::XMVECTOR size, const Texture& tex, DirectX::XMVECTOR uvShift = { 0,0,0,0 }) { implementation->renderQuadOnScreen(pos, size, tex, uvShift); }
	static void renderBatch2D() { implementation->renderBatch2D(); }

	//----------------------------------------------//

	static void renderAABB(Camera& cam, const AABB& aabb) { implementation->renderAABB(cam, aabb); }
	static void renderDebugPerspectiveCameraOutline(Camera& viewCamera, const Camera& outlinedCamera) { implementation->renderDebugPerspectiveCameraOutline(viewCamera, outlinedCamera); }
	static void renderDebugOrthographicCameraOutline(const Camera& viewCamera, const Camera& outlinedCamera) { implementation->renderDebugOrthographicCameraOutline(viewCamera, outlinedCamera); }
	static void renderDebugLine(Camera& cam) { implementation->renderDebugLine(cam); }
	static void showImGuiDebugData() { implementation->showImGuiDebugData(); }
	static void addDebugLine(DirectX::XMVECTOR from, DirectX::XMVECTOR to) { implementation->addDebugLine(from, to); }


	/////////////////////////////////////////////////////////////////////////////
	template<typename _RenderingImplementation>
		requires (std::derived_from<_RenderingImplementation, _Impl>)
	static void setImplementation()
	{
		implementation = std::make_unique<_RenderingImplementation>();
	}


private:

	static std::unique_ptr<_Impl> implementation;

};




