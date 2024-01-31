#include "HeightmapBuilder.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
Mesh HeightmapBuilder::buildChunk(const Heightmap& map, Vec2<int> chunkPos, Vec2<int> chunkSize, float xy_scale, float height_factor)
{

	int width = map.getWidth();
	int height = map.getHeigth();

	std::vector<Vertex> vertices;
	std::vector<Index> indices;

	float maxHeightValue = FLT_MIN; // can't use limits cause ::max() becomes C max... so annoying
	float minHeightValue = FLT_MAX;
	// -- Step 1 : Build the vertices

	for (int dy = 0; dy < chunkSize.y + 1; dy++)
		for (int dx = 0; dx < chunkSize.x + 1; dx++)
		{
			int x = chunkPos.x + dx + 1;
			int y = chunkPos.y + dy + 1;


			// 1.1 -> Compute the position
			Vertex v{};
			float h = map.getAt(x, y);
			maxHeightValue = max(maxHeightValue, h * height_factor);
			minHeightValue = min(minHeightValue, h * height_factor);
			v.position = Vec{ x * xy_scale, h * height_factor  , y * xy_scale, };

			// 1.2 -> Compute the normal
			float a, b, c, d;
			a = ((y + 1) < height) ? map.getAt(x, y + 1) :	XMVectorGetY(v.position);
			b = ((y - 1) >= 0) ? map.getAt(x, y - 1) :		XMVectorGetY(v.position);
			c = ((x + 1) < width) ? map.getAt(x + 1, y) :	XMVectorGetY(v.position);
			d = ((x - 1) >= 0) ? map.getAt(x - 1, y) :		XMVectorGetY(v.position);

			Vec normal = XMVector4Normalize(XMVector3Cross(
				Vec{ 0.f, (a - b) * height_factor, 2.f * xy_scale },
				Vec{ 2.f * xy_scale, (c - d) * height_factor, 0.f }
			));

			v.normal = Vec{ normal };

			// 1.3 -> Compute UVs

			Vec uv = { (float)x/30, (float)y/30 };
			v.uv = uv;
			vertices.push_back(v);
		}

	// -- Step 2 : Build the indicies
	for (int y = 0; y < chunkSize.y; y++)
		for (int x = 0; x < chunkSize.x; x++)
		{


			Index a1 = y * (chunkSize.x + 1) + x;
			Index a2 = y * (chunkSize.x + 1) + x + 1;
			Index a3 = (y + 1) * (chunkSize.x + 1) + x;
			Index a4 = (y + 1) * (chunkSize.x + 1) + x + 1;


			indices.push_back(a1);
			indices.push_back(a3);
			indices.push_back(a2);
			indices.push_back(a2);
			indices.push_back(a3);
			indices.push_back(a4);

		}
	//assert(indices.size() == 6 * (chunkSize.x * chunkSize.y));
	//assert(vertices.size() == ((chunkSize.x + 1) * (chunkSize.y + 1)));
	Mesh res{ vertices, indices };
	res.getBoundingBox() = AABB{
		Vec{static_cast<float>(chunkPos.x + 1) * xy_scale, minHeightValue, static_cast<float>(chunkPos.y + 1) * xy_scale},
		Vec{static_cast<float>(chunkSize.x) * xy_scale, maxHeightValue, static_cast<float>(chunkSize.y) * xy_scale} };

	return res;
}

std::vector<Mesh> HeightmapBuilder::buildTerrainMesh(const Heightmap& map, Vec2<int> chunkCount,
	float xy_scale, float height_factor) {

	int width = map.getWidth();
	int height = map.getHeigth();

	int chunkWidth = width / chunkCount.x;
	int chunkHeight = height / chunkCount.y;

	std::vector<Mesh> chunks;

	for (int y = 0; y < chunkCount.y; ++y)
		for (int x = 0; x < chunkCount.x; ++x)
		{
			chunks.emplace_back(buildChunk(map,
				Vec2<int>{ x* chunkWidth, y* chunkHeight },
				Vec2<int>{ chunkWidth, chunkHeight },
				xy_scale, height_factor));
		}


	return chunks;

}


/* Reads an image an produces a Heightmap */
Heightmap HeightmapReader::readMapFromFile(const fs::path& path)
{
	int x, y, channel;

	unsigned char* data = stbi_load(path.string().c_str(), &x, &y, &channel, 1);
	if (!data) throw std::exception();

	Heightmap resMap(x, y);

	// Store the values of the heightmap png into the object while normalizing them
	std::transform(resMap.begin(), resMap.end(), resMap.begin(),
		[ptr = data](float& v) mutable {return static_cast<float>((*ptr++) / 255.f); });

	stbi_image_free(data);

	return resMap;

}