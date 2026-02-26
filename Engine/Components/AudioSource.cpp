#include "AudioSource.h"

#include "../Classes/IO.h"
#include "../Core/Engine.h"
#include "../Core/GameObject.h"
#include "../Components/Transform.h"
#include "../Core/SoundManager.h"
#include "../Core/APIManager.h"
#include "../Core/Time.h"
#include "../Classes/ZipHelper.h"
#include "../Assets/AudioClip.h"

#include "../glm/vec3.hpp"

#define DR_WAV_IMPLEMENTATION
#include "../Classes/dr_wav.h"
#undef DR_WAV_IMPLEMENTATION

#define DR_MP3_IMPLEMENTATION
#include "../Classes/dr_mp3.h"
#undef DR_MP3_IMPLEMENTATION

#define MIN_STREAMING_LENGTH 5

namespace GX
{
	std::string AudioSource::COMPONENT_TYPE = "AudioSource";

	AudioSource::AudioSource() : Component(APIManager::getSingleton()->audiosource_class)
	{
		for (int i = 0; i < 3; i++)
		{
			mPos[i] = mVel[i] = 0.0f;
		}

		mInfo = NULL;
		mVF = NULL;
		mComment = NULL;

		SoundManager * mgr = SoundManager::getSingleton();
		if (mgr != nullptr)
		{
			mgr->addSource(this);
		}

		//Gen audio source
		alGenSources(1, &mSourceID);

		//Default parameters
		alSourcef(mSourceID, AL_PITCH, getTimeScale());
		alSourcef(mSourceID, AL_GAIN, volume);
		alSourcefv(mSourceID, AL_POSITION, mPos);
		alSourcefv(mSourceID, AL_VELOCITY, mVel);
		alSourcei(mSourceID, AL_LOOPING, mLooped);
		alSourcei(mSourceID, AL_SOURCE_RELATIVE, is2D);
		alSourcei(mSourceID, AL_REFERENCE_DISTANCE, (ALint)minDistance);
		alSourcei(mSourceID, AL_MAX_DISTANCE, (ALint)maxDistance);
		alSource3f(mSourceID, AL_DIRECTION, 0.0, 0.0, 0.0);
	}

	AudioSource::~AudioSource()
	{
		close();
		
		if (alIsSource(mSourceID)) alDeleteSources(1, &mSourceID);

		SoundManager * mgr = SoundManager::getSingleton();
		if (mgr != nullptr)
		{
			mgr->deleteSource(this);
		}
	}

	std::string AudioSource::getComponentType()
	{
		return COMPONENT_TYPE;
	}

	void AudioSource::reloadFile()
	{
		if (audioClip != nullptr && audioClip->isLoaded())
		{
			if (getEnabled() && (gameObject == nullptr || gameObject->getActive()))
			{
				close();
				open();
			}
		}
	}

	float AudioSource::getTimeScale()
	{
		return overrideTimeScale ? overrideTimeScaleValue : Time::getTimeScale();
	}

	void AudioSource::setAudioClip(AudioClip* clip)
	{
		audioClip = clip;

		reloadFile();
	}

	bool AudioSource::open()
	{
		if (gameObject != nullptr)
		{
			if (!Engine::getSingleton()->getIsRuntimeMode())
				return false;
		}

		if (audioClip == nullptr || !audioClip->isLoaded())
			return false;

		std::string location = audioClip->getLocation();
		std::string fileName = audioClip->getName();

		// Check if file exists
		std::string fullPath = audioClip->getOrigin();

		bool read = false;

		if (IO::isDir(location))
		{
			if (IO::FileExists(fullPath))
			{
				read = true;
			}
		}
		else
		{
			zip_t* arch = Engine::getSingleton()->getZipArchive(location);
			if (ZipHelper::isFileInZip(arch, fileName))
			{
				fullPath = fileName;
				read = true;
			}
		}

		if (read)
		{
			fileValid = true;

			std::string Ext = IO::GetFileExtension(fileName);
			if (Ext == "ogg")
			{
				fileFormat = FileFormat::FF_OGG;
				return loadOggFile(fullPath);
			}

			if (Ext == "wav")
			{
				fileFormat = FileFormat::FF_WAV;
				return loadWavFile(fullPath);
			}

			if (Ext == "mp3")
			{
				fileFormat = FileFormat::FF_MP3;
				return loadMp3File(fullPath);
			}
		}

		return false;
	}

	bool AudioSource::isStreamed()
	{
		return mStreamed;
	}

	void AudioSource::play()
	{
		if (isPausedGlobal)
			return;

		if (audioClip == nullptr || !audioClip->isLoaded()) return;
		if (!fileValid) return;
		if (!getEnabled() || (gameObject != nullptr && !gameObject->getActive())) return;

		stop();

		prepareBuffers();

		alSourcePlay(mSourceID);

		isPlaying = true;
		isPaused = false;
	}

	void AudioSource::resume()
	{
		if (isPlaying)
		{
			if (isPaused)
			{
				if (!isPausedGlobal)
					alSourcePlay(mSourceID);

				isPaused = false;
			}
		}
	}

	void AudioSource::resumeGlobal()
	{
		if (isPlaying)
		{
			if (!isPaused)
				alSourcePlay(mSourceID);
		}

		isPausedGlobal = false;
	}

	void AudioSource::pause()
	{
		if (isPlaying)
		{
			if (!isPausedGlobal)
				alSourcePause(mSourceID);

			isPaused = true;
		}
	}

	void AudioSource::pauseGlobal()
	{
		if (isPlaying)
		{
			if (!isPaused)
				alSourcePause(mSourceID);
		}

		isPausedGlobal = true;
	}

	void AudioSource::stop()
	{
		if (fileValid)
		{
			alSourceStop(mSourceID);

			int Processed = 0;
			alGetSourcei(mSourceID, AL_BUFFERS_PROCESSED, &Processed);
			if (Processed > 0)
			{
				while (Processed--)
				{
					ALuint BufID;
					alSourceUnqueueBuffers(mSourceID, 1, &BufID);
				}
			}

			seekPos(0);
		}

		isPlaying = false;
		isPaused = false;
		currentPos = 0;
	}

	void AudioSource::setPosition(float X, float Y, float Z)
	{
		ALfloat Pos[3] = { X, Y, Z };
		alSourcefv(mSourceID, AL_POSITION, Pos);
	}

	void AudioSource::setLoop(bool loop)
	{
		mLooped = loop;
		if (mSourceID > -1)
			alSourcei(mSourceID, AL_LOOPING, mLooped);
	}

	void AudioSource::setVolume(float value)
	{
		volume = value;
		alSourcef(mSourceID, AL_GAIN, volume);
	}

	void AudioSource::setMinDistance(float value)
	{
		minDistance = value;
		alSourcei(mSourceID, AL_REFERENCE_DISTANCE, (ALint)minDistance);
	}

	void AudioSource::setMaxDistance(float value)
	{
		maxDistance = value;
		alSourcei(mSourceID, AL_MAX_DISTANCE, (ALint)maxDistance);
	}

	void AudioSource::setPitch(float value)
	{
		pitch = value;
	}

	void AudioSource::setIs2D(bool value)
	{
		is2D = value;
		alSourcei(mSourceID, AL_SOURCE_RELATIVE, is2D);

		if (is2D)
		{
			setPosition(0, 0, 0);
		}
		else
		{
			//if (gameObject == nullptr)
			//	return;

			if (gameObject != nullptr)
			{
				glm::vec3 pos = gameObject->getTransform()->getPosition();
				setPosition(pos.x, pos.y, pos.z);
			}
		}
	}

	void AudioSource::onSceneLoaded()
	{
		if (Engine::getSingleton()->getIsRuntimeMode())
		{
			reloadFile();

			if (playOnStart)
			{
				play();
			}
		}
	}

	void AudioSource::onStateChanged()
	{
		if (!getEnabled())
		{
			if (getIsPlaying())
				stop();
		}
	}

	Component* AudioSource::onClone()
	{
		AudioSource* newComponent = new AudioSource();
		newComponent->enabled = enabled;
		newComponent->playOnStart = playOnStart;
		newComponent->isPlaying = isPlaying;
		newComponent->volume = volume;
		newComponent->minDistance = minDistance;
		newComponent->maxDistance = maxDistance;
		newComponent->isPaused = isPaused;
		newComponent->isPausedGlobal = isPausedGlobal;
		newComponent->is2D = is2D;
		newComponent->mLooped = mLooped;
		newComponent->audioClip = audioClip;

		return newComponent;
	}

	void AudioSource::onRefresh()
	{
		reloadFile();
	}

	void AudioSource::prepareBuffers()
	{
		seekPos(0);

		if (mStreamed)
		{
			for (auto it = buffers.begin(); it != buffers.end(); ++it)
			{
				readDataBlock(it->ID, DYNBUF_SIZE);
				alSourceQueueBuffers(mSourceID, 1, &it->ID);
			}
		}
		else
		{
			uint64_t BlockSize = 0;
			if (fileFormat == FileFormat::FF_OGG)
				BlockSize = ov_pcm_total(mVF, -1) * 4;
			if (fileFormat == FileFormat::FF_WAV)
				BlockSize = wav.dataChunkDataSize;
			if (fileFormat == FileFormat::FF_MP3)
				BlockSize = mp3.dataSize;

			for (auto it = buffers.begin(); it != buffers.end(); ++it)
			{
				readDataBlock(it->ID, BlockSize);
				alSourcei(mSourceID, AL_BUFFER, it->ID);
			}
		}
	}

	void AudioSource::seekPos(int seconds)
	{
		currentPos = seconds;

		int bytePos = seconds * (sampleRate * 2 * 2);

		if (bytePos < totalSize)
		{
			if (fileFormat == FileFormat::FF_OGG)
				ov_pcm_seek(mVF, bytePos / 4);
			if (fileFormat == FileFormat::FF_WAV)
				drwav_seek_to_pcm_frame(&wav, bytePos / 4);
			if (fileFormat == FileFormat::FF_MP3)
				drmp3_seek_to_pcm_frame(&mp3, bytePos / 4);
		}
	}

	bool AudioSource::loadWavFile(const std::string& Filename)
	{
		std::string location = Engine::getSingleton()->getAssetsPath();
		
		audioData = new AudioData();

		if (IO::isDir(location))
		{
			AudioFile.open(Filename.c_str(), std::ios_base::in | std::ios_base::binary);
			audioData->isZip = false;
			audioData->data = &AudioFile;

			if (!drwav_init(&wav, readWav, seekWav, audioData, NULL))
			{
				AudioFile.close();
				return false;
			}
		}
		else
		{
			zip_t* arch = Engine::getSingleton()->getZipArchive(location);
			int sz = 0;
			AudioZipData = ZipHelper::readFileFromZip(arch, Filename, sz);
			AudioFileZip = new boost::iostreams::stream<boost::iostreams::array_source>(AudioZipData, sz);
			audioData->isZip = true;
			audioData->data = AudioFileZip;

			if (!drwav_init(&wav, readWav, seekWav, audioData, NULL))
			{
				AudioFile.close();
				return false;
			}
		}

		totalSize = wav.dataChunkDataSize;
		sampleRate = wav.sampleRate;

		mStreamed = getTotalLength() > MIN_STREAMING_LENGTH;

		if (!mStreamed)
		{
			AudioInfo buffer;

			buffer.Rate = wav.sampleRate;
			buffer.Format = (wav.channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;

			alGenBuffers(1, &buffer.ID);
			buffers.push_back(buffer);
		}
		else
		{
			alSourcei(mSourceID, AL_LOOPING, AL_FALSE);

			for (int i = 0; i < NUM_OF_DYNBUF; i++)
			{
				AudioInfo buffer;

				buffer.Rate = wav.sampleRate;
				buffer.Format = (wav.channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;

				alGenBuffers(1, &buffer.ID);
				buffers.push_back(buffer);
			}
		}

		return true;
	}

	bool AudioSource::loadMp3File(const std::string& Filename)
	{
		std::string location = Engine::getSingleton()->getAssetsPath();

		audioData = new AudioData();

		if (IO::isDir(location))
		{
			AudioFile.open(Filename.c_str(), std::ios_base::in | std::ios_base::binary | std::ios_base::ate);
			audioData->isZip = false;
			audioData->data = &AudioFile;

	#ifdef _WIN32
			AudioFile.seekg(0, std::ios_base::_Seekdir::_Seekbeg);
	#else
			AudioFile.seekg(0, std::ifstream::beg);
	#endif

			if (!drmp3_init(&mp3, readMp3, seekMp3, audioData, nullptr))
			{
				AudioFile.close();
				return false;
			}
		}
		else
		{
			zip_t* arch = Engine::getSingleton()->getZipArchive(location);
			int sz = 0;
			AudioZipData = ZipHelper::readFileFromZip(arch, Filename, sz);
			AudioFileZip = new boost::iostreams::stream<boost::iostreams::array_source>(AudioZipData, sz);
			audioData->isZip = true;
			audioData->data = AudioFileZip;

			if (!drmp3_init(&mp3, readMp3, seekMp3, audioData, nullptr))
			{
				AudioFile.close();
				return false;
			}
		}

		sampleRate = mp3.sampleRate;
		uint64_t numFrames = drmp3_get_pcm_frame_count(&mp3);
		totalSize = (int)numFrames * 4;

		mStreamed = getTotalLength() > MIN_STREAMING_LENGTH;

		if (!mStreamed)
		{
			AudioInfo buffer;

			buffer.Rate = mp3.sampleRate;
			buffer.Format = (mp3.channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;

			alGenBuffers(1, &buffer.ID);
			buffers.push_back(buffer);
		}
		else
		{
			alSourcei(mSourceID, AL_LOOPING, AL_FALSE);

			for (int i = 0; i < NUM_OF_DYNBUF; i++)
			{
				AudioInfo buffer;

				buffer.Rate = mp3.sampleRate;
				buffer.Format = (mp3.channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;

				alGenBuffers(1, &buffer.ID);
				buffers.push_back(buffer);
			}
		}

		return true;
	}

	bool AudioSource::loadOggFile(const std::string & Filename)
	{
		std::string location = Engine::getSingleton()->getAssetsPath();

		audioData = new AudioData();

		ov_callbacks cb;

		cb.close_func = closeOgg;
		cb.read_func = readOgg;
		cb.seek_func = seekOgg;
		cb.tell_func = tellOgg;

		mVF = new OggVorbis_File;

		if (IO::isDir(location))
		{
			AudioFile.open(Filename.c_str(), std::ios_base::in | std::ios_base::binary);
			audioData->isZip = false;
			audioData->data = &AudioFile;

			if (ov_open_callbacks(audioData, mVF, NULL, -1, cb) < 0)
			{
				AudioFile.close();
				return false;
			}
		}
		else
		{
			zip_t* arch = Engine::getSingleton()->getZipArchive(location);
			int sz = 0;
			AudioZipData = ZipHelper::readFileFromZip(arch, Filename, sz);
			AudioFileZip = new boost::iostreams::stream<boost::iostreams::array_source>(AudioZipData, sz);
			audioData->isZip = true;
			audioData->data = AudioFileZip;

			if (ov_open_callbacks(audioData, mVF, NULL, -1, cb) < 0)
			{
				AudioFile.close();
				return false;
			}
		}

		totalSize = (int)ov_pcm_total(mVF, -1) * 4;

		mComment = ov_comment(mVF, -1);
		mInfo = ov_info(mVF, -1);

		sampleRate = mInfo->rate;

		mStreamed = getTotalLength() > MIN_STREAMING_LENGTH;

		if (!mStreamed)
		{
			AudioInfo buffer;

			buffer.Rate = mInfo->rate;
			buffer.Format = (mInfo->channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;

			alGenBuffers(1, &buffer.ID);
			buffers.push_back(buffer);
		}
		else
		{
			alSourcei(mSourceID, AL_LOOPING, AL_FALSE);

			for (int i = 0; i < NUM_OF_DYNBUF; i++)
			{
				AudioInfo buffer;

				buffer.Rate = mInfo->rate;
				buffer.Format = (mInfo->channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;

				alGenBuffers(1, &buffer.ID);
				buffers.push_back(buffer);
			}
		}

		return true;
	}

	size_t AudioSource::readDataBlock(ALuint BufID, size_t Size)
	{
		if (Size < 1) return -20;

		int eof = 0;
		ALsizei ret = 0;
		int current_section = 0;

		std::vector<char> PCM;
		PCM.resize(Size);

		if (fileFormat == FileFormat::FF_OGG)
		{
			while (ret < Size && eof == 0)
			{
				long result = ov_read(mVF, &PCM[0] + ret, (int)Size - ret, 0, 2, 1, &current_section);

				if (result == 0)
				{
					eof = 1;
				}
				else if (result < 0)
				{
				}
				else
				{
					ret += result;
				}
			}
		}
		if (fileFormat == FileFormat::FF_WAV)
		{
			drwav_uint32 frameSize = drwav_get_bytes_per_pcm_frame(&wav);
			ret = drwav_read_pcm_frames_s16(&wav, Size / frameSize, (drwav_int16*)&PCM[0]) * frameSize;
			if (ret == 0) eof = 1;
		}
		if (fileFormat == FileFormat::FF_MP3)
		{
			uint64_t s = drmp3_read_pcm_frames_raw(&mp3, Size / mp3.channels / 2, &PCM[0]);
			ret = s * mp3.channels * 2;
			if (ret == 0) eof = 1;
		}

		auto it = find_if(buffers.begin(), buffers.end(), [=](const AudioInfo& a) -> bool { return a.ID == BufID; });
		if (it != buffers.end() && !eof)
		{
			AudioInfo inf = *it;

			if (inf.Format == AL_FORMAT_STEREO16)
			{
				if (!is2D)
				{
					size_t sz = ret / 2;

					//If the buffer has 2 channels, convert it to a single channel to support 3d sound.
					std::vector<char> pcm;
					pcm.resize(sz);

					uint32_t j = 0;
					for (int i = 0; i < ret; i += 4, j += 2)
					{
						int16_t l = 0;
						int16_t r = 0;

						memcpy(&l, &PCM[0] + i, 2);
						memcpy(&r, &PCM[0] + i + 2, 2);

						int16_t c = (int16_t)std::round((l + r) / 2);

						memcpy(&pcm[0] + j, &c, 2);
					}

					alBufferData(BufID, AL_FORMAT_MONO16, (void*)&pcm[0], sz, inf.Rate);
					pcm.clear();
				}
				else
				{
					alBufferData(BufID, inf.Format, (void*)&PCM[0], ret, inf.Rate);
				}
			}
			else
			{
				alBufferData(BufID, inf.Format, (void*)&PCM[0], ret, inf.Rate);
			}
		}

		if (SoundManager::checkALError())
			ret = -10; // Error

		if (eof == 1)
			ret = -20; // Stop

		PCM.clear();

		return ret;
	}

	size_t AudioSource::readWav(void* pUserData, void* pBufferOut, size_t bytesToRead)
	{
		AudioData* data = reinterpret_cast<AudioData*>(pUserData);

		if (!data->isZip)
		{
			std::istream* File = reinterpret_cast<std::istream*>(data->data);
			File->read((char*)pBufferOut, bytesToRead);
			size_t gc = File->gcount();
			return gc;
		}
		else
		{
			boost::iostreams::stream<boost::iostreams::array_source>* File = reinterpret_cast<boost::iostreams::stream<boost::iostreams::array_source>*>(data->data);
			File->read((char*)pBufferOut, bytesToRead);
			size_t gc = File->gcount();
			return gc;
		}
	}

	drwav_bool32 AudioSource::seekWav(void* pUserData, int offset, drwav_seek_origin origin)
	{
		AudioData* data = reinterpret_cast<AudioData*>(pUserData);

		if (!data->isZip)
		{
			std::istream* File = reinterpret_cast<std::istream*>(data->data);
			std::ios_base::seekdir Dir;
			File->clear();
			switch (origin)
			{
				case drwav_seek_origin_start: Dir = std::ios::beg; break;
				case drwav_seek_origin_current: Dir = std::ios::cur; break;
				default: Dir = std::ios::end; break;
			}
			File->seekg((std::streamoff)offset, Dir);
			return (File->fail() ? false : true);
		}
		else
		{
			boost::iostreams::stream<boost::iostreams::array_source>* File = reinterpret_cast<boost::iostreams::stream<boost::iostreams::array_source>*>(data->data);
			std::ios_base::seekdir Dir;
			File->clear();
			switch (origin)
			{
				case drwav_seek_origin_start: Dir = std::ios::beg; break;
				case drwav_seek_origin_current: Dir = std::ios::cur; break;
				default: Dir = std::ios::end; break;
			}
			File->seekg((std::streamoff)offset, Dir);
			return (File->fail() ? false : true);
		}
	}

	size_t AudioSource::readMp3(void* pUserData, void* pBufferOut, size_t bytesToRead)
	{
		AudioData* data = reinterpret_cast<AudioData*>(pUserData);

		if (!data->isZip)
		{
			std::istream* File = reinterpret_cast<std::istream*>(data->data);
			File->read((char*)pBufferOut, bytesToRead);
			int gc = File->gcount();
			return gc;
		}
		else
		{
			boost::iostreams::stream<boost::iostreams::array_source>* File = reinterpret_cast<boost::iostreams::stream<boost::iostreams::array_source>*>(data->data);
			File->read((char*)pBufferOut, bytesToRead);
			int gc = File->gcount();
			return gc;
		}
	}

	drmp3_bool32 AudioSource::seekMp3(void* pUserData, int offset, drmp3_seek_origin origin)
	{
		AudioData* data = reinterpret_cast<AudioData*>(pUserData);

		if (!data->isZip)
		{
			std::istream* File = reinterpret_cast<std::istream*>(data->data);
			std::ios_base::seekdir Dir;
			File->clear();
			switch (origin)
			{
			case drmp3_seek_origin_start: Dir = std::ios::beg;  break;
			case drmp3_seek_origin_current: Dir = std::ios::cur;  break;
			default: Dir = std::ios::end; break;
			}
			File->seekg((std::streamoff)offset, Dir);
			return (File->fail() ? false : true);
		}
		else
		{
			boost::iostreams::stream<boost::iostreams::array_source>* File = reinterpret_cast<boost::iostreams::stream<boost::iostreams::array_source>*>(data->data);
			std::ios_base::seekdir Dir;
			File->clear();
			switch (origin)
			{
			case drmp3_seek_origin_start: Dir = std::ios::beg;  break;
			case drmp3_seek_origin_current: Dir = std::ios::cur;  break;
			default: Dir = std::ios::end; break;
			}
			File->seekg((std::streamoff)offset, Dir);
			return (File->fail() ? false : true);
		}
	}

	size_t AudioSource::readOgg(void *ptr, size_t size, size_t nmemb, void *datasource)
	{
		AudioData* data = reinterpret_cast<AudioData*>(datasource);

		if (!data->isZip)
		{
			std::istream* File = reinterpret_cast<std::istream*>(data->data);
			File->read((char*)ptr, size * nmemb);
			int gc = File->gcount();
			return gc;
		}
		else
		{
			boost::iostreams::stream<boost::iostreams::array_source> * File = reinterpret_cast<boost::iostreams::stream<boost::iostreams::array_source>*>(data->data);
			File->read((char*)ptr, size * nmemb);
			int gc = File->gcount();
			return gc;
		}
	}

	int AudioSource::seekOgg(void *datasource, ogg_int64_t offset, int whence)
	{
		AudioData* data = reinterpret_cast<AudioData*>(datasource);

		if (!data->isZip)
		{
			std::istream* File = reinterpret_cast<std::istream*>(data->data);
			std::ios_base::seekdir Dir;
			File->clear();
			switch (whence)
			{
				case SEEK_SET: Dir = std::ios::beg; break;
				case SEEK_CUR: Dir = std::ios::cur; break;
				case SEEK_END: Dir = std::ios::end; break;
				default: return -1;
			}
			File->seekg((std::streamoff)offset, Dir);
			return (File->fail() ? -1 : 0);
		}
		else
		{
			boost::iostreams::stream<boost::iostreams::array_source>* File = reinterpret_cast<boost::iostreams::stream<boost::iostreams::array_source>*>(data->data);
			std::ios_base::seekdir Dir;
			File->clear();
			switch (whence)
			{
				case SEEK_SET: Dir = std::ios::beg; break;
				case SEEK_CUR: Dir = std::ios::cur; break;
				case SEEK_END: Dir = std::ios::end; break;
				default: return -1;
			}
			File->seekg((std::streamoff)offset, Dir);
			return (File->fail() ? -1 : 0);
		}
	}

	long AudioSource::tellOgg(void *datasource)
	{
		AudioData* data = reinterpret_cast<AudioData*>(datasource);

		if (!data->isZip)
		{
			std::istream* File = reinterpret_cast<std::istream*>(data->data);
			return (long)File->tellg();
		}
		else
		{
			boost::iostreams::stream<boost::iostreams::array_source>* File = reinterpret_cast<boost::iostreams::stream<boost::iostreams::array_source>*>(data->data);
			return (long)File->tellg();
		}
	}

	int AudioSource::closeOgg(void *datasource)
	{
		return 0;
	}

	void AudioSource::update()
	{
		if (!fileValid)
			return;

		if (mSourceID == -1)
			return;

		if (!is2D)
		{
			if (gameObject != nullptr)
			{
				glm::vec3 pos = gameObject->getTransform()->getPosition();
				setPosition(pos.x, pos.y, pos.z);
			}
		}

		alSourcef(mSourceID, AL_PITCH, getTimeScale() * pitch);

		if (!isPlaying) return;

		if (isPaused || isPausedGlobal)
			return;

		ALenum state;
		alGetSourcei(mSourceID, AL_SOURCE_STATE, &state);

		bool shouldStop = false;

		if (mStreamed)
		{
			ALint Processed = 0;
			alGetSourcei(mSourceID, AL_BUFFERS_PROCESSED, &Processed);

			while (Processed--)
			{
				ALuint BufID;

				alSourceUnqueueBuffers(mSourceID, 1, &BufID);
				int ret = readDataBlock(BufID, DYNBUF_SIZE);
				alSourceQueueBuffers(mSourceID, 1, &BufID);

				if (ret == -20)
					shouldStop = true;
			}
		}

		currentPos += Time::getDeltaTime() * getTimeScale();

		if (state == AL_STOPPED || shouldStop)
		{
			if (!mLooped)
			{
				stop();
			}
			else
			{
				stop();
				play();
			}
		}
	}

	int AudioSource::getTotalLength()
	{
		size_t totalSeconds = totalSize / (sampleRate * 2 * 2);
		return (int)totalSeconds;
	}

	int AudioSource::getPlaybackPosition()
	{
		return currentPos;
	}

	void AudioSource::setPlaybackPosition(int seconds)
	{
		if (!isPlaying)
			return;

		if (mStreamed)
		{
			alSourceStop(mSourceID);

			int Processed = 0;
			alGetSourcei(mSourceID, AL_BUFFERS_PROCESSED, &Processed);
			if (Processed > 0)
			{
				while (Processed--)
				{
					ALuint BufID;
					alSourceUnqueueBuffers(mSourceID, 1, &BufID);
				}
			}

			seekPos(seconds);

			for (auto it = buffers.begin(); it != buffers.end(); ++it)
			{
				readDataBlock(it->ID, DYNBUF_SIZE);
				alSourceQueueBuffers(mSourceID, 1, &it->ID);
			}

			alSourcePlay(mSourceID);
		}
		else
		{
			alSourcef(mSourceID, AL_SEC_OFFSET, seconds);
			currentPos = seconds;
		}
	}

	void AudioSource::close()
	{
		stop();
		freeResources();
	}

	void AudioSource::freeResources()
	{
		fileValid = false;

		for (auto i = buffers.begin(); i != buffers.end(); i++)
			alDeleteBuffers(1, &i->ID);

		buffers.clear();

		if (AudioFileZip != nullptr)
		{
			delete AudioFileZip;
		}

		if (AudioZipData != nullptr)
		{
			delete[] AudioZipData;
		}

		if (fileFormat == FileFormat::FF_OGG)
		{
			if (!mVF)
			{
				ov_clear(mVF);
				delete mVF;
				mVF = nullptr;
			}
		}

		if (fileFormat == FileFormat::FF_WAV)
			drwav_uninit(&wav);

		if (fileFormat == FileFormat::FF_MP3)
		{
			drmp3_uninit(&mp3);
			mp3 = drmp3();
		}

		if (AudioFile.is_open())
			AudioFile.close();

		if (audioData != nullptr)
			delete audioData;

		audioData = nullptr;
	}
}