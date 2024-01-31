#include <rapidobj.hpp>

#include "MeshManager.h"
#include "../../abstraction/Vertex.h"
#include "../../World/Material.h"
#include "../Mesh.h"

#include <iostream>
#include <vector>
#define _XM_NO_INTRINSICS_
#include <directXmath.h>
#include <tuple>
#include <set>
#include <unordered_set>

std::vector<Mesh*> MeshManager::m_meshes = {};
std::unordered_map<Scene*, std::vector<std::shared_ptr<Mesh>>>  MeshManager::s_sceneLoadedMeshes = {};
std::unordered_map<std::string, std::shared_ptr<Mesh>> MeshManager::s_loadedMeshes = {};


void ReportError(const rapidobj::Error& error)
{
	std::cout << error.code.message() << "\n";
	if (!error.line.empty()) {
		std::cout << "On line " << error.line_num << ": \"" << error.line << "\"\n";
	}
}

void assertValid(const rapidobj::Result& res) {
	if (res.error)
	{
		ReportError(res.error);
		throw;
	}
}

inline Vertex generateVertexFromKey(const rapidobj::Index& key, const rapidobj::Result& loadedResult)
{

	XMVECTOR pos{}, normal{}, uv{};

	if (key.position_index != -1)
	{
		pos = {
			loadedResult.attributes.positions[key.position_index * 3],
			loadedResult.attributes.positions[key.position_index * 3 + 1],
			loadedResult.attributes.positions[key.position_index * 3 + 2],
		};
	}

	if (key.normal_index != -1)
	{
		normal =
		{
			loadedResult.attributes.normals[key.normal_index * 3],
			loadedResult.attributes.normals[key.normal_index * 3 + 1],
			loadedResult.attributes.normals[key.normal_index * 3 + 2],
		};
	}

	if (key.texcoord_index != -1)
	{
		uv = {
			loadedResult.attributes.texcoords[key.texcoord_index * 2],
			loadedResult.attributes.texcoords[key.texcoord_index * 2 + 1]
		};
	}

	return { pos, normal, uv };
}

static constexpr auto hash = [](const rapidobj::Index& index) -> size_t {
	size_t h1 = std::hash<int>()(index.position_index);
	size_t h2 = std::hash<int>()(index.normal_index);
	size_t h3 = std::hash<double>()(index.texcoord_index);
	return h1 ^ h2 << 1 ^ h3;
};

auto comp = [](const rapidobj::Index& a, const rapidobj::Index& b) -> bool {
	return a.position_index == b.position_index && a.normal_index == b.normal_index && a.texcoord_index == b.texcoord_index;
};

std::unordered_map<rapidobj::Index, IndexBuffer::size_type, decltype(hash), decltype(comp)> vertsIndex;

Mesh MeshManager::loadMeshFromFile(const fs::path& pathToFile) {

	bool pls = std::filesystem::exists(pathToFile);

	rapidobj::MaterialLibrary mtl_library = rapidobj::MaterialLibrary::Default();
	rapidobj::Result res = rapidobj::ParseFile(pathToFile);

	assertValid(res);
	rapidobj::Triangulate(res);
	assertValid(res);


	// -- Generate the mesh

	using key = std::tuple<int, int, int>;
	std::vector<key> cachedKeys; // todo use a haser for the rapidobj::Index
	std::set<key> allcachedKeys; // todo use a haser for the rapidobj::Index

	std::vector<Vertex> vertices;

	std::vector<IndexBuffer::size_type> indices;
	std::vector<IndexBuffer::size_type> submeshesIndices;
	std::vector<uint16_t> submeshesMat;
	IndexBuffer::size_type lastIndex = 0;


	for (const auto& shape : res.shapes) {
		
		submeshesIndices.push_back(static_cast<IndexBuffer::size_type>(indices.size()));
		submeshesMat.push_back(0); // trash

		for (size_t i = 0; i < shape.mesh.indices.size(); i += 3)
		{
			int materialIndex = 0;
			if (!res.materials.empty())
			{
				materialIndex = shape.mesh.material_ids[i / 3]; // Material index for this face
			}

			// Iterate through the vertices of the face
			// Use this one for left handed winding
			for (int j = 0; j < 3; ++j)
			{
				const auto& indexes = shape.mesh.indices[i + j];
				const auto& [position_index, texcoord_index, normal_index] = indexes;

				key currentKey =
				{
					position_index,
					texcoord_index,
					normal_index
				};

				IndexBuffer::size_type index = static_cast<IndexBuffer::size_type>(vertices.size());
				if (vertsIndex.contains(indexes)) {
					index = vertsIndex[indexes];
				}
				else {
					vertices.push_back(std::move(generateVertexFromKey(indexes, res)));
					vertsIndex[indexes] = index;
				}

				indices.push_back(index);
			}
		}
	}
	
	submeshesIndices.push_back(static_cast<IndexBuffer::size_type>(indices.size()));

	// -- Build Materials
	
	std::vector<Material> mats;

	for (const auto& mat : res.materials)
	{
		Material resMat;
		MaterialCoefs mc;

		mc.Ka[0] = mat.ambient[0];
		mc.Ka[1] = mat.ambient[1];
		mc.Ka[2] = mat.ambient[2];

		mc.Kd[0] = mat.diffuse[0];
		mc.Kd[1] = mat.diffuse[1];
		mc.Kd[2] = mat.diffuse[2];

		mc.Ks[0] = mat.specular[0];
		mc.Ks[1] = mat.specular[1];
		mc.Ks[2] = mat.specular[2];


		mc.Ke[0] = mat.emission[0];
		mc.Ke[1] = mat.emission[1];
		mc.Ke[2] = mat.emission[2];


		mc.Ns = mat.shininess;
		mc.transparency = 1.f - mat.dissolve;
		mc.Ni = mat.ior;

		resMat.setCoefficients(mc);
		resMat.setModel(mat.illum);

		resMat.loadTextures(
			{
			{mat.diffuse_texname, TextureType::ALBEDO},
			{mat.normal_texname, TextureType::NORMAL},
			{mat.roughness_texname, TextureType::ROUGHNESS},
			{mat.ambient_texname, TextureType::AO},
			{mat.specular_highlight_texname, TextureType::SPECULAR}
			}
		);
		


		mats.emplace_back(resMat);

	}
	vertsIndex.clear();
	return Mesh(vertices, indices, submeshesIndices, mats, submeshesMat);
}


void MeshManager::loadMeshFromFile(const std::string& name, const fs::path& pathToFile)
{
	if (s_loadedMeshes.contains(name)) return;
	s_loadedMeshes[name] = std::make_shared<Mesh>(loadMeshFromFile(pathToFile));
}