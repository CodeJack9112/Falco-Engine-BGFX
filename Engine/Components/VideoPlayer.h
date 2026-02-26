#pragma once

#include <string>
#include <thread>
#include <vector>

#include "Component.h"
#include "../Renderer/Color.h"

class AVFormatContext;
class AVCodecContext;
class AVCodec;
struct SwsContext;
struct AVFrame;
struct AVPicture;
struct AVIOContext;

namespace GX
{
	class VideoClip;
	class Texture;

	class VideoPlayer : public Component
	{
	public:
		struct StreamInfo
		{
		public:
			StreamInfo() {}
			~StreamInfo() {}

			int totalSize = 0;
			void* stream = nullptr;
		};

	private:
		VideoClip* videoClip = nullptr;
		Texture* texture = nullptr;
		bool playOnStart = false;
		bool loop = false;
		bool isPlaying = false;
		bool opened = false;
		bool threadWork = false;
		float currentTime = 0;

		std::vector<unsigned char*> frameQueue;
		std::thread worker;

		AVFormatContext* format_context = nullptr;
		AVCodecContext* codec_context = nullptr;
		AVCodec* codec = nullptr;
		AVIOContext* ioContext = nullptr;
		int video_stream = -1;
		int audio_stream = -1;
		float time = 0.0f;
		int total_time = 0;

		bool open();
		void close();

		Color getRGBPixel(const AVFrame* frame, int x, int y);
		bool frame();
		void updateTexture();
		void clearFrameQueue();

		char* zip_buffer = nullptr;
		StreamInfo* zip_stream = nullptr;

		static int zip_read(void* opaque, unsigned char* buf, int buf_size);
		static int64_t zip_seek(void* opaque, int64_t offset, int whence);

	public:
		VideoPlayer();
		virtual ~VideoPlayer();

		static std::string COMPONENT_TYPE;
		virtual std::string getComponentType() { return COMPONENT_TYPE; }
		virtual Component* onClone();
		virtual void onUpdate(float deltaTime);
		virtual void onSceneLoaded();
		virtual void onStateChanged();
		virtual void onRefresh();

		VideoClip* getVideoClip() { return videoClip; }
		void setVideoClip(VideoClip* value);

		bool getPlayOnStart() { return playOnStart; }
		void setPlayOnStart(bool value) { playOnStart = value; }

		bool getLoop() { return loop; }
		void setLoop(bool value) { loop = value; }

		bool getIsPlaying() { return opened; }
		bool getIsPaused() { return opened && !isPlaying; }

		int getTotalLength();
		int getPlaybackPosition();
		void setPlaybackPosition(int value);

		Texture* getTexture() { return texture; }

		void play(bool startThread = true);
		void stop();
		void pause();
		void resume();
	};
}