#include "Sound.h"

std::unordered_map<SoundType, std::pair<ALuint, ALuint>> Sound::m_Sounds{};

ALCdevice* Sound::device{};
ALCcontext* Sound::context{};

const std::vector<std::pair<SoundType, std::string>> Sound::m_path = {
	{SoundType::Background, "res/sounds/bgm.wav"},
	{SoundType::Effect, "res/sounds/CP.wav"},
	{SoundType::Start, "res/sounds/Start2.wav" },
	{SoundType::End, "res/sounds/Victory.wav"},
	{SoundType::MainScreen, "res/sounds/nujabes.wav"}
};

std::unordered_map<SoundType, bool> Sound::m_isPlaying = {
	{SoundType::Background, false},
	{SoundType::Effect, false},
	{SoundType::Start, false},
	{SoundType::End, false},
	{SoundType::MainScreen, false}
};

std::unordered_map<SoundType, bool> Sound::m_isPaused = {
	{SoundType::Background, false},
	{SoundType::Effect, false},
	{SoundType::Start, false },
	{SoundType::End, false},
	{SoundType::MainScreen, false}
};

std::unordered_map<SoundType, float> Sound::m_gain = {
	{SoundType::Background, 0.5f},
	{SoundType::Effect, 0.5f},
	{SoundType::Start, 0.5f},
	{SoundType::End, 0.5f},
	{SoundType::MainScreen, 0.5f}
};

const int Sound::m_nbSounds = static_cast<int>(m_path.size());

ALuint* Sound::buffers;
ALuint* Sound::sources;

void Sound::Init()
{
	device = alcOpenDevice(nullptr);
	if (!device)
	{
		throw std::runtime_error("Failed to open OpenAL device.");
	}

	context = alcCreateContext(device, nullptr);
	if (!context)
	{
		throw std::runtime_error("Failed to create OpenAL context.");
	}

	if (!alcMakeContextCurrent(context))
	{
		throw std::runtime_error("Failed to make OpenAL context current.");
	}

	buffers = new ALuint[m_nbSounds];
	alGenBuffers(m_nbSounds, buffers);

	sources = new ALuint[m_nbSounds];
	alGenSources(m_nbSounds, sources);

	for (int i = 0; i < m_nbSounds; i++)
	{
		m_Sounds[static_cast<SoundType>(i)] = std::make_pair(sources[i], buffers[i]);
	}

	std::ranges::for_each(m_path, [](auto& sound)
	{
		loadWavFileToBuffer(sound.second, sound.first);
	});
}

bool Sound::loadWavFileToBuffer(std::string filePath, SoundType type)
{
	const char* path = filePath.c_str();

	std::ifstream file(path, std::ios::binary | std::ios::ate);
	if (!file.is_open())
	{
		std::cerr << "Failed to open file: " << path << std::endl;
		return false;
	}

	const std::streamsize fileSize = file.tellg();
	file.seekg(0, std::ios::beg);

	std::vector<char> bufferData(fileSize);
	if (!file.read(bufferData.data(), fileSize))
	{
		std::cerr << "Failed to read file: " << path << std::endl;
		return false;
	}

	file.close();

	const ALvoid* data = static_cast<ALvoid*>(bufferData.data());

	// Assuming a standard uncompressed PCM WAV format
	constexpr ALenum format = AL_FORMAT_STEREO16;
	constexpr ALsizei frequency = 44100;
	//frequency = 48000; // Change this according to your file's frequency
	const auto data_size = static_cast<ALuint>(fileSize);

	// Generate buffer
	alBufferData(m_Sounds[type].second, format, data, data_size, frequency);

	return true;
}

void Sound::playSound(SoundType type)
{
	alSourcei(m_Sounds[type].first, AL_BUFFER, m_Sounds[type].second);
	alSourcef(m_Sounds[type].first, AL_GAIN, m_gain[type]);
	alSourcePlay(m_Sounds[type].first);

	m_isPlaying[type] = true;
}

void Sound::playSoundLoop(SoundType type)
{
	alSourcei(m_Sounds[type].first, AL_BUFFER, m_Sounds[type].second);
	alSourcef(m_Sounds[type].first, AL_GAIN, m_gain[type]);
	alSourcei(m_Sounds[type].first, AL_LOOPING, AL_TRUE);
	alSourcePlay(m_Sounds[type].first);

	m_isPlaying[type] = true;
}

bool Sound::checkSourceState(SoundType type)
{
	const ALuint source = m_Sounds[type].first;

	ALint state;
	alGetSourcei(source, AL_SOURCE_STATE, &state);

	if (state == AL_STOPPED)
	{
		//std::cout << "Le son a fini de jouer." << std::endl;
		return true;
	}
	return false;
}

void Sound::stopSound(SoundType type)
{
	if (m_isPlaying[type])
	{
		alSourceStop(m_Sounds[type].first);
		m_isPlaying[type] = false;
		m_isPaused[type] = false;
	}
}

void Sound::stopAllSounds()
{
	std::ranges::for_each(m_Sounds, [](auto& sound)
	{
		if (m_isPlaying[sound.first])
		{
			alSourceStop(sound.second.first);
			m_isPlaying[sound.first] = false;
			m_isPaused[sound.first] = false;
		}
	});
}

void Sound::pauseSound(SoundType type)
{
	if (!m_isPaused[type])
	{
		alSourcePause(m_Sounds[type].first);
		m_isPaused[type] = true;
	}
}

void Sound::pauseAllSounds()
{
	std::ranges::for_each(m_Sounds, [](auto& sound)
	{
		if (!checkSourceState(sound.first))
		{
			if(m_isPaused[sound.first] == false)
			{
				alSourcePause(sound.second.first);
				m_isPaused[sound.first] = true;
			}
		}
	});
}

void Sound::resumeSound(SoundType type)
{
	if (m_isPlaying[type] && m_isPaused[type])
	{
		alSourcePlay(m_Sounds[type].first);
		m_isPaused[type] = false;
	}
}

void Sound::resumeAllSounds()
{
	std::ranges::for_each(m_Sounds, [](auto& sound)
	{
		if (m_isPlaying[sound.first] && m_isPaused[sound.first])
		{
			alSourcePlay(sound.second.first);
			m_isPaused[sound.first] = false;
		}
	});
}

void Sound::setSoundVolumeBGM(float newGain)
{
	if (newGain > 0 && newGain < 1.1)
	{
		alSourcef(m_Sounds[SoundType::Background].first, AL_GAIN, newGain);
		alSourcef(m_Sounds[SoundType::MainScreen].first, AL_GAIN, newGain);

		m_gain[SoundType::Background] = newGain;
		m_gain[SoundType::MainScreen] = newGain;

		//std::cout << newGain << std::endl;
	}
}

void Sound::setSoundVolumeEffect(float newGain)
{
	if (newGain > 0 && newGain < 1.1)
	{
		alSourcef(m_Sounds[SoundType::Effect].first, AL_GAIN, newGain);
		alSourcef(m_Sounds[SoundType::End].first, AL_GAIN, newGain);
		alSourcef(m_Sounds[SoundType::Start].first, AL_GAIN, newGain);

		m_gain[SoundType::Effect] = newGain;
		m_gain[SoundType::End] = newGain;
		m_gain[SoundType::Start] = newGain;
	}
}

void Sound::close()
{
	stopAllSounds();

	alDeleteSources(m_nbSounds, sources);
	alDeleteBuffers(m_nbSounds, buffers);

	m_Sounds.clear();

	alcMakeContextCurrent(nullptr);

	alcDestroyContext(context);
	alcCloseDevice(device);
}
