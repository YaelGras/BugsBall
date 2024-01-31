#pragma once
#ifndef TERRAIN_H
#define TERRAIN_H

#include "../Mesh.h"
#include "Heightmap.h"
#include "HeightmapBuilder.h"
#include <map>
#include <filesystem>

namespace fs = std::filesystem;

struct Chunk
{
	Mesh chunkMesh;
};

class Terrain {


public:

	struct TerrainParams {

		int		width = 0, height=0;
		float	xyScale=1;
		float	scaleFactor=1;
		int		chunkSize=0;
		Vec2<int> chunkCount{ 5,5 };
	};



private:


	TerrainParams m_params;

	Heightmap m_map;
	std::vector<Mesh> m_chunks;
	
	fs::path m_path; // used for hot reload

public:

	~Terrain() {
		std::cout << "Terrain has been destroyed !" << std::endl;
	}
	Terrain(const fs::path& pathToMap = "res/textures/heightmap.png")
	{
		m_path = pathToMap;
		m_map = HeightmapReader::readMapFromFile(pathToMap);

		const TerrainParams defaultParams{
			m_map.getWidth(), m_map.getHeigth(),
			/*xyScale*/ 1.F,
			/*height scale*/ 100.F,
			1
		};

		m_params = defaultParams;

		rebuildMesh();
	}

	void changeMap(const fs::path& pathToMap) {
		m_map = HeightmapReader::readMapFromFile(pathToMap);
		rebuildMesh();
	}

	void hotReloadMap()
	{
		changeMap(m_path);
	}

	void rebuildMesh() {
		m_chunks = HeightmapBuilder::buildTerrainMesh(
			m_map, m_params.chunkCount,
			m_params.xyScale, m_params.scaleFactor);
	}

	void setParams(const TerrainParams& params) {
		m_params = params;
		rebuildMesh();
	}

	// might be inaccurate
	XMVECTOR getNormalAt(XMVECTOR worldPos) const 
	{
		return getNormalAt(static_cast<int>(XMVectorGetX(worldPos)/ m_params.xyScale), static_cast<int>(XMVectorGetZ(worldPos)/ m_params.xyScale));

	}

	XMVECTOR getNormalAt(int x, int y) const
	{
		float a, b, c, d;
		float y_pos = m_map.getAt(x, y) * m_params.scaleFactor;
		a = ((y + 1) < m_map.getHeigth()) ? m_map.getAt(x, y + 1) : y_pos;
		b = ((y - 1) >= 0) ? m_map.getAt(x, y - 1) : y_pos;
		c = ((x + 1) < m_map.getWidth()) ? m_map.getAt(x + 1, y) : y_pos;
		d = ((x - 1) >= 0) ? m_map.getAt(x - 1, y) : y_pos;

		Vec normal = XMVector4Normalize(XMVector3Cross(
			Vec{ 0.f, (a - b) * m_params.scaleFactor, 2.f * m_params.xyScale },
			Vec{ 2.f * m_params.xyScale, (c - d) * m_params.xyScale, 0.f }
		));


		return normal;
	}

	float getWorldHeightAt(XMVECTOR worldPos) const
	{
		return m_map.getAt(
			static_cast<int>(XMVectorGetX(worldPos) / m_params.xyScale), 
			static_cast<int>(XMVectorGetZ(worldPos) / m_params.xyScale))

		* m_params.scaleFactor;
	}

	const TerrainParams& getParams() const { return m_params; }
	TerrainParams& getParams() { return m_params; }

	//const Mesh& getMesh() const noexcept { return m_mesh; }
	//Mesh& getMesh() noexcept { return m_mesh; }
	std::vector<Mesh>& getMesh() noexcept { return m_chunks; }


	void showDebugWindow() {

		ImGui::Begin("Terrain settings");

		if (
			ImGui::DragFloat("XY Scale", &m_params.xyScale) +
			ImGui::DragFloat("Height Factor", &m_params.scaleFactor) +
			ImGui::DragInt2("Chunk Count", &m_params.chunkCount[0], 2, 2, 5)
			) {
			rebuildMesh();
		}


		ImGui::End();


	}


	[[nodiscard]] Heightmap& getHeightmap() { return m_map; }
	[[nodiscard]] const Heightmap& getHeightmap() const { return m_map; }
};


#endif // !TERRAIN_H