#pragma once

#include <iostream>
#include <nlohmann/json.hpp>
#include <fstream>
#include <string>
#include <algorithm>
#include <vector>
#define _XM_NO_INTRINSICS_
#include <DirectXMath.h>


struct FormatJson
{
	std::string nameObj;
	std::string pathObj;
	DirectX::XMVECTOR positionObj;
	DirectX::XMVECTOR forwardObj;
	DirectX::XMVECTOR scaleObj;
	DirectX::XMVECTOR scaleShape;
	DirectX::XMVECTOR matShape;
};

enum class TypeLight : int32_t
{
	DIRECTIONAL,
	POINT,
	SPOTLIGHT
};

struct FormatJsonLight
{
	TypeLight type{};

	int distance{};

	float strength{};
	float falloff{};
	float radius{};
	float specular{};

	DirectX::XMVECTOR direction{};
	DirectX::XMVECTOR ambiant{};
	DirectX::XMVECTOR diffuse{};
	DirectX::XMVECTOR range{};
	DirectX::XMVECTOR position{};
};

class JsonParser
{
private:
	std::string m_fileName;
	nlohmann::json m_json;

	std::ifstream m_file;

	std::vector<FormatJson> objs;


public:

	JsonParser() = default;
	JsonParser(std::string fileName) : m_fileName(fileName)
	{};

	void openFile();

	void updateList();

	void getCheckpoints(std::vector<FormatJson>& chepointInfos);

	void getControleIA(std::vector<FormatJson>& controleIAInfos);

	[[nodiscard]] std::vector<FormatJson> getObjs();

	[[nodiscard]] static std::vector<FormatJsonLight> getLights();
};
