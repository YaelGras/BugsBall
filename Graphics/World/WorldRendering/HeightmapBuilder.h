#pragma once
#ifndef HEIGHTMAPBUILDER_H
#define HEIGHTMAPBUILDER_H

#include "../Mesh.h"
#include "Heightmap.h"
#include <vector>
#include "../../../Utils/Vec2.h"

#define _XM_NO_INTRINSICS_
#include <directXmath.h>


#include <filesystem>

namespace fs = std::filesystem;

using Index = uint32_t;
using Vec = XMVECTOR;

/// <summary>
/// This is taken from the first rush with a bit of rewrite (changing types names, i hate dxmath)
/// </summary>

class HeightmapBuilder {

public:

	static Mesh buildChunk(const Heightmap& map, Vec2<int> chunkPos, Vec2<int> chunkSize, float xy_scale = 1, float height_factor = 10);

	static std::vector<Mesh> buildTerrainMesh(const Heightmap& map, Vec2<int> chunkCount,
		float xy_scale = 1, float height_factor = 1); 
};

class HeightmapReader {
public:
	/* Reads an image an produces a Heightmap */
	static Heightmap readMapFromFile(const fs::path& path);
};


#endif // !HEIGHTMAPBUILDER_H