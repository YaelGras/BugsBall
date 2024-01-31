#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <chrono>
#include <unordered_map>
#include <vector>
#include <algorithm>

#include "../../Utils/vendor/openAL/AL/al.h"
#include "../../Utils/vendor/openAL/AL/alc.h"

enum class SoundType
{
	MainScreen,
	Background,
	Start,
	Effect,
	End
};

class Sound
{
private:
	// La pair de ALuint est composée de la source et du buffer
	static std::unordered_map<SoundType, std::pair<ALuint, ALuint>> m_Sounds;

	static ALCdevice* device;
	static ALCcontext* context;

	static const std::vector<std::pair<SoundType, std::string>> m_path;
	static std::unordered_map<SoundType, bool> m_isPlaying;
	static std::unordered_map<SoundType, bool> m_isPaused;
	static std::unordered_map<SoundType, float> m_gain;

	static const int m_nbSounds;

	static ALuint* buffers;
	static ALuint* sources;

public:
	static void Init();

	static bool loadWavFileToBuffer(std::string filePath, SoundType type);

	static void playSound(SoundType type);

	static void playSoundLoop(SoundType type);

	static bool checkSourceState(SoundType type);

	static std::pair<ALuint, ALuint> getSound(SoundType type) { return m_Sounds[type]; }

	static void stopSound(SoundType type);

	static void stopAllSounds();

	static void pauseSound(SoundType type);

	static void pauseAllSounds();

	static void resumeSound(SoundType type);

	static void resumeAllSounds();

	static void setSoundVolumeBGM(float newGain);

	static void setSoundVolumeEffect(float newGain);

	static float getGain(const SoundType type) { return m_gain[type]; }

	static bool isPlaying(const SoundType type) { return m_isPlaying[type]; }

	static bool isPaused(const SoundType type) { return m_isPaused[type]; }

	static void close();
};
