#pragma once

#include <filesystem>
#include <vector>
#include <unordered_map>
#include <memory>
#include <algorithm>
#include <string>
#include <ranges>

#include "../Mesh.h"
#include "../Frustum.h"
#include "../../abstraction/Camera.h"
namespace fs = std::filesystem;

class Scene;

class MeshManager
{
private:

	static std::vector<Mesh*> m_meshes;
	static std::unordered_map<std::string, std::shared_ptr<Mesh>> s_loadedMeshes;
	static std::unordered_map<Scene*, std::vector<std::shared_ptr<Mesh>>> s_sceneLoadedMeshes;


public:

	/// This loads a mesh reference into the MeshManager static list of meshes. You can query a reference with getReference()
	static Mesh loadMeshFromFile(const fs::path& pathToFile);
	static void loadMeshFromFile(const std::string& name, const fs::path& pathToFile);


public:

	// Brings a mesh reference to a scene; This should actually be a model instead of a mesh, and have it's own transform
	static void loadToCurrentScene(Scene* currentScene, const std::string& name)	{ 
		if (!currentScene) return;
		s_sceneLoadedMeshes[currentScene].push_back(getMeshReference(name));
	}
	static void clearCurrentSceneMesh(Scene* currentScene)	{ 
		if (!s_sceneLoadedMeshes.contains(currentScene)) return;
		std::ranges::for_each(s_sceneLoadedMeshes[currentScene], [&](const std::shared_ptr<Mesh>& m) {	MeshManager::unloadMesh(m);	});
		s_sceneLoadedMeshes[currentScene].clear();
	}


	static void renderCurrentSceneLoadedMeshes(Scene* scene, Camera& camera, const std::function<void(Camera& cam, const Mesh& m)>& renderFn)

	{
		Frustum frustum = Frustum::createFrustumFromPerspectiveCamera(camera);

		std::ranges::for_each(s_sceneLoadedMeshes[scene], [&](const std::shared_ptr<Mesh>& m)
			{
				if (frustum.isOnFrustum(m->getBoundingBox()))
				{
					renderFn(camera, *m);
				}
			}
		);
	}


	static void unloadMesh(const std::string& name)	{	s_loadedMeshes.erase(name); }
	static void unloadMesh(const std::shared_ptr<Mesh>& meshRef)
	{
		auto it = std::ranges::find_if(s_loadedMeshes, [&meshRef](auto m) {return m.second == meshRef; });
		if (it != s_loadedMeshes.end()) {
			s_loadedMeshes.erase(it->first);
		}
	}

	[[nodiscard]] static const std::vector<std::shared_ptr<Mesh>>& getSceneMeshes(Scene* currentScene) noexcept { return s_sceneLoadedMeshes[currentScene]; }
	[[nodiscard]] static std::shared_ptr<Mesh> getMeshReference(const std::string& name)
	{
		if (!s_loadedMeshes.contains(name)) return nullptr;
		return s_loadedMeshes[name];
	}

	static void clear()
	{
		std::ranges::for_each(m_meshes, [&](auto* m) {delete m; });
		m_meshes.clear();
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

	static void onImGuiRender(Scene* currentScene) {

		std::ranges::for_each(s_sceneLoadedMeshes[currentScene], [&](const std::shared_ptr<Mesh>& m) {
			std::string name = [&m]()-> std::string
			{
				for (const auto& p : s_loadedMeshes)
				{
					if (p.second == m) return p.first;
				}
				return "NA";
			}();
			m->getTransform().showControlWindow(name);
		});

	}
};

