#pragma once

//Std
#include <string>
#include <map>
#include <vector>

//OpenAL
#include "../OpenAL/include/al.h"
#include "../OpenAL/include/alc.h"
#include "../OpenAL/include/alu.h"
#include "../OpenAL/include/alut.h"

//Ogg-Vorbis
#include "../codecs/audio/ogg/vorbis/codec.h"
#include "../codecs/audio/ogg/vorbis/vorbisfile.h"

#define NUM_OF_DYNBUF	4		// Num buffers in queue
#define DYNBUF_SIZE		65536	// Buffer size

namespace GX
{
	struct AudioInfo
	{
		unsigned int  ID;
		unsigned int  Rate;
		unsigned int  Format;
	};

	//typedef std::map<ALuint, AudioInfo> BufferList;

	class AudioSource;
	class AudioListener;

	class SoundManager
	{
		friend class AudioSource;
		friend class AudioListener;

	private:
		static SoundManager singleton;

		ALCdevice* pDevice = nullptr;
		ALCcontext* pContext = nullptr;
		std::vector<AudioSource*> sources;
		AudioListener* listener = nullptr;

		bool paused = false;

		static ALboolean checkALCError(ALCdevice* pDevice);
		static ALboolean checkALError();

	public:
		SoundManager();
		~SoundManager();

		static SoundManager* getSingleton() { return &singleton; }

		//static std::map<ALuint, AudioInfo> Buffers;

		bool init();
		void update();
		void addSource(AudioSource * src) { sources.push_back(src); }
		void deleteSource(AudioSource* src);
		void destroy();
		void setMasterVolume(float volume);

		void setListener(AudioListener * listener) { this->listener = listener; }
		AudioListener * getListener() { return this->listener; }

		bool getPaused() { return paused; }
		void setPaused(bool value);
	};
}