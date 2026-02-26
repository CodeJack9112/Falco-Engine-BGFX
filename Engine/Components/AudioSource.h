#pragma once

#include <string>
#include <fstream>

#include "../Core/SoundManager.h"
#include "Component.h"

#include <boost/iostreams/stream.hpp>

//Codecs
#include "../codecs/audio/ogg/vorbis/codec.h"
#include "../codecs/audio/ogg/vorbis/vorbisfile.h"
#include "../codecs/audio/ogg/ogg/os_types.h"
#include "../Classes/dr_wav.h"
#include "../Classes/dr_mp3.h"

namespace GX
{
	class AudioClip;

	class AudioSource : public Component
	{
	private:
		enum class FileFormat { FF_NONE, FF_OGG, FF_WAV, FF_MP3 };

		struct AudioData
		{
			bool isZip = false;
			void* data = nullptr;
		};

		//Main
		ALfloat mVel[3];
		ALfloat mPos[3];
		bool  mLooped = false;
		ALuint mSourceID = -1;
		AudioClip* audioClip = nullptr;
		bool fileValid = false;
		FileFormat fileFormat = FileFormat::FF_NONE;

		std::vector<AudioInfo> buffers;

		//Wav
		drwav wav;

		//Mp3
		drmp3 mp3;

		//Ogg
		OggVorbis_File *mVF = nullptr;
		vorbis_comment *mComment = nullptr;
		vorbis_info *mInfo = nullptr;
		std::ifstream AudioFile;
		boost::iostreams::stream<boost::iostreams::array_source>* AudioFileZip = nullptr;
		char* AudioZipData = nullptr;
		AudioData* audioData = nullptr;

		bool mStreamed = true;
		size_t readDataBlock(ALuint BufID, size_t Size);

		//Wav callbacks
		static size_t readWav(void* pUserData, void* pBufferOut, size_t bytesToRead);
		static drwav_bool32 seekWav(void* pUserData, int offset, drwav_seek_origin origin);

		//Mp3 callbacks
		static size_t readMp3(void* pUserData, void* pBufferOut, size_t bytesToRead);
		static drmp3_bool32 seekMp3(void* pUserData, int offset, drmp3_seek_origin origin);

		//Ogg callbacks
		static size_t readOgg(void *ptr, size_t size, size_t nmemb, void *datasource);
		static int seekOgg(void *datasource, ogg_int64_t offset, int whence);
		static long tellOgg(void *datasource);
		static int closeOgg(void *datasource);

		//Loading files functions
		bool loadWavFile(const std::string &Filename);
		bool loadMp3File(const std::string &Filename);
		bool loadOggFile(const std::string &Filename);

		void setPosition(float X, float Y, float Z);

		bool playOnStart = false;
		bool isPlaying = false;
		float volume = 1.0f;
		float minDistance = 10.0f;
		float maxDistance = 1000.0f;
		bool isPaused = false;
		bool isPausedGlobal = false;
		bool is2D = false;
		uint64_t totalSize = 0;
		float currentPos = 0.0f;
		uint32_t sampleRate = 1;
		float pitch = 1.0f;
		bool overrideTimeScale = false;
		float overrideTimeScaleValue = 1.0f;

		void prepareBuffers();
		void seekPos(int seconds);
		void reloadFile();

		float getTimeScale();

	public:
		AudioSource();
		virtual ~AudioSource();

		bool open();
		void setAudioClip(AudioClip* clip);
		AudioClip* getAudioClip() { return audioClip; }
		bool isStreamed();
		void play();
		void resume();
		void resumeGlobal();
		void pause();
		void pauseGlobal();
		void stop();
		void close();
		void freeResources();
		void update();

		int getTotalLength();
		int getPlaybackPosition();
		void setPlaybackPosition(int seconds);

		bool getIsPlaying() { return isPlaying; }
		bool getIsPaused() { return isPaused; }
		bool getIsPausedGlobal() { return isPausedGlobal; }
		
		void setPlayOnStart(bool play) { playOnStart = play; }
		void setLoop(bool loop);

		bool getPlayOnStart() { return playOnStart; }
		bool getLooped() { return mLooped; }

		float getVolume() { return volume; }
		void setVolume(float value);

		float getMinDistance() { return minDistance; }
		void setMinDistance(float value);

		float getMaxDistance() { return maxDistance; }
		void setMaxDistance(float value);

		float getPitch() { return pitch; }
		void setPitch(float value);

		bool getIs2D() { return is2D; }
		void setIs2D(bool value);

		bool getOverrideTimeScale() { return overrideTimeScale; }
		void setOverrideTimeScale(bool value) { overrideTimeScale = value; }

		bool getOverrideTimeScaleValue() { return overrideTimeScaleValue; }
		void setOverrideTimeScaleValue(bool value) { overrideTimeScaleValue = value; }

		static std::string COMPONENT_TYPE;
		virtual std::string getComponentType();
		virtual void onSceneLoaded();
		virtual void onStateChanged();
		virtual Component* onClone();
		virtual void onRefresh();
	};
}