#include "GameInputs.h"

std::unordered_map<std::string, char> GameInputs::m_mapping = {};
nlohmann::json GameInputs::m_json{};
std::filesystem::path GameInputs::m_filePath = "N/A";

void GameInputs::loadFile(const std::filesystem::path& path)
{
	m_filePath = path;
	std::ifstream is;
	is.open(path);
	is >> m_json;
	is.close();
}

void GameInputs::loadMapping()
{
	if (m_json.is_array())
	{
		int charTemp{};
		std::string strTemp{};
		for (auto& item : m_json)
		{
			charTemp = item["Touche"];
			strTemp = item["Fonction"];
			m_mapping[strTemp] = charTemp;
		}
	}
}

void GameInputs::EditMapping(const std::string& key, int value)
{
	for (auto& item : m_json)
	{
		if (item["Fonction"] == key)
		{
			item["Touche"] = value;
			m_mapping[key] = value;
			break;
		}
	}

	std::ofstream fileOut(m_filePath);
	fileOut << std::setw(4) << m_json << std::endl;
	fileOut.close();
}

void GameInputs::resetMapping()
{
	std::string pathResetstr = m_filePath.string();
	pathResetstr.erase(pathResetstr.find(".json"), 5);
	pathResetstr += "_Backup.json";

	std::filesystem::path pathReset = pathResetstr;

	nlohmann::json jsonTemp;

	std::ifstream is;
	is.open(pathResetstr);
	is >> jsonTemp;
	is.close();

	if (jsonTemp.is_array())
	{
		int charTemp{};
		std::string strTemp{};
		for (auto& item : jsonTemp)
		{
			charTemp = item["Touche"];
			strTemp = item["Fonction"];
			m_mapping[strTemp] = charTemp;
		}
	}

	std::ofstream fileOut(m_filePath);
	fileOut << std::setw(4) << jsonTemp << std::endl;
	fileOut.close();
}
