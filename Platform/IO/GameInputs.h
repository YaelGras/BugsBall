#pragma once

#include <iostream>
#include <nlohmann/json.hpp>
#include <fstream>
#include <string>
#include <algorithm>
#include <unordered_map>

enum class Direction { FORWARD, BACKWARD, LEFT, RIGHT };

enum class Command { POV, RESET, RESPAWN, PAUSE, NEXT_CP, LAST_CP };

class GameInputs
{
private:
	static nlohmann::json m_json;
	static std::unordered_map<std::string, char> m_mapping;
	static std::filesystem::path m_filePath;

public:
	static void Init(const std::filesystem::path& path)
	{
		loadFile(path);
		loadMapping();
	}

	static void EditMapping(const std::string& key, int value);
	static void resetMapping();

	// TODO make this a template
	static char queryDirectionInput(Direction direction)
	{
		switch (direction)
		{
		case Direction::FORWARD:
			return m_mapping["Forward"];
		case Direction::BACKWARD:
			return m_mapping["Backward"];
		case Direction::LEFT:
			return m_mapping["Left"];
		case Direction::RIGHT:
			return m_mapping["Right"];
		default:
			throw std::runtime_error("Direction not found");
		}
	};

	static char queryCommandInput(Command command)
	{
		switch (command)
		{
		case Command::POV:
			return m_mapping["POV"];
		case Command::RESET:
			return m_mapping["Reset"];
		case Command::RESPAWN:
			return m_mapping["Respawn"];
		case Command::PAUSE:
			return m_mapping["Pause"];
		case Command::NEXT_CP:
			return m_mapping["NextCP"];
		case Command::LAST_CP:
			return m_mapping["LastCP"];
		default:
			throw std::runtime_error("Command not found");
		}
	}

private:
	static void loadFile(const std::filesystem::path& path);
	static void loadMapping();

public:
	[[nodiscard]] static bool isBound(char key)
	{
		return std::ranges::find_if(m_mapping, [key](const auto& item) { return item.second == key; }) != m_mapping.
			end();
	}

	[[nodiscard]] static char getBoundKey(const std::string& key) noexcept { return m_mapping[key]; }
	[[nodiscard]] static size_t getBoundKeysCount() noexcept { return m_mapping.size(); }

	[[nodiscard]] static std::string displayKey(const std::string& key) { return key + " : " + m_mapping[key]; }

	[[nodiscard]] static std::vector<std::string> getKeys() noexcept
	{
		auto k = (m_mapping | std::views::keys);
		return {k.begin(), k.end()};
	}

};
