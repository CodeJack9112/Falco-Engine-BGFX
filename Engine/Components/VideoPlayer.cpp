#include "VideoPlayer.h"

#include "../Core/Engine.h"
#include "../Core/APIManager.h"
#include "../Core/Debug.h"
#include "../Core/Time.h"
#include "../Core/GameObject.h"
#include "../Classes/Helpers.h"
#include "../Classes/GUIDGenerator.h"
#include "../Components/Transform.h"
#include "../Components/Canvas.h"
#include "../Assets/Texture.h"
#include "../Assets/VideoClip.h"
#include "../Math/Mathf.h"
#include "../Classes/ZipHelper.h"
#include "../Classes/IO.h"

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

#include <boost/iostreams/stream.hpp>

#pragma warning( push )
#pragma warning( disable : 4996 )
namespace GX
{
	std::string VideoPlayer::COMPONENT_TYPE = "VideoPlayer";

	bool VideoPlayer::open()
	{
		if (opened)
			close();

		if (videoClip == nullptr || !videoClip->isLoaded())
			return false;

		std::string filename = videoClip->getOrigin();
		int err = 0;

		if (IO::isDir(Engine::getSingleton()->getAssetsPath()))
		{
			err = avformat_open_input(&format_context, filename.c_str(), NULL, NULL);
			if (err < 0)
			{
				Debug::log("ffmpeg: Unable to open input file", Debug::DbgColorRed);
				return false;
			}
		}
		else
		{
			std::string location = videoClip->getLocation();
			zip_t* arch = Engine::getSingleton()->getZipArchive(location);

			zip_stream = new StreamInfo();

			int sz = 0;
			zip_buffer = ZipHelper::readFileFromZip(arch, videoClip->getName(), sz);
			zip_stream->stream = new boost::iostreams::stream<boost::iostreams::array_source>(zip_buffer, sz);
			zip_stream->totalSize = sz;

			int buf_size = 32768;
			unsigned char* fileStreamBuffer = (unsigned char*)av_malloc(buf_size);

			// Get a AVContext stream
			ioContext = avio_alloc_context(
				fileStreamBuffer,
				buf_size,
				0,
				zip_stream,
				zip_read,
				nullptr,
				zip_seek
			);

			// Allocate a AVContext
			format_context = avformat_alloc_context();

			// Set up the Format Context
			format_context->pb = ioContext;
			//format_context->flags |= AVFMT_FLAG_CUSTOM_IO;

			if (avformat_open_input(&format_context, "", nullptr, nullptr) < 0)
			{
				delete[] zip_buffer;
				((boost::iostreams::stream<boost::iostreams::array_source>*)zip_stream->stream)->close();
				delete zip_stream;
				zip_buffer = nullptr;
				zip_stream = nullptr;

				Debug::log("ffmpeg: Unable to open input file", Debug::DbgColorRed);
				return false;
			}
		}

		err = avformat_find_stream_info(format_context, NULL);
		if (err < 0)
		{
			Debug::log("ffmpeg: Unable to find stream info", Debug::DbgColorRed);
			return false;
		}

		//Video
		for (video_stream = 0; video_stream < format_context->nb_streams; ++video_stream)
		{
			if (format_context->streams[video_stream]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
				break;
		}

		if (video_stream == format_context->nb_streams)
		{
			Debug::log("ffmpeg: Unable to find video stream", Debug::DbgColorRed);
			return false;
		}

		//Audio
		for (audio_stream = 0; audio_stream < format_context->nb_streams; ++audio_stream)
		{
			if (format_context->streams[audio_stream]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
				break;
		}

		codec_context = format_context->streams[video_stream]->codec;
		codec = avcodec_find_decoder(codec_context->codec_id);
		err = avcodec_open2(codec_context, codec, NULL);
		if (err < 0)
		{
			Debug::log("ffmpeg: Unable to open codec", Debug::DbgColorRed);
			return false;
		}

		opened = true;

		if (format_context != nullptr)
		{
			AVStream* videoStream = format_context->streams[video_stream];
			if (videoStream != nullptr)
			{
				double time_base = (double)videoStream->time_base.num / (double)videoStream->time_base.den;
				double duration = (double)videoStream->duration * time_base;

				total_time = (int)duration;
			}

			if (audio_stream != format_context->nb_streams)
			{
				AVStream* audioStream = format_context->streams[audio_stream];
				if (audioStream != nullptr)
				{
					double time_base = (double)audioStream->time_base.num / (double)audioStream->time_base.den;
					double duration = (double)audioStream->duration * time_base;

					total_time = std::max((int)duration, total_time);
				}
			}
		}

		return true;
	}

	void VideoPlayer::close()
	{
		if (opened)
		{
			opened = false;

			if (ioContext != nullptr)
			{
				av_free(ioContext->buffer);
				av_free(ioContext);
			}

			// Close the codec
			if (codec_context != nullptr)
				avcodec_close(codec_context);

			// Close the video file
			if (format_context != nullptr)
				avformat_close_input(&format_context);

			if (zip_stream != nullptr)
			{
				((boost::iostreams::stream<boost::iostreams::array_source>*)zip_stream->stream)->close();
				delete zip_stream;
			}

			if (zip_buffer != nullptr)
				delete[] zip_buffer;

			codec_context = nullptr;
			format_context = nullptr;
			zip_stream = nullptr;
			zip_buffer = nullptr;
		}
	}

	VideoPlayer::VideoPlayer() : Component(APIManager::getSingleton()->videoplayer_class)
	{
		
	}

	VideoPlayer::~VideoPlayer()
	{
		stop();
	}

	Component* VideoPlayer::onClone()
	{
		VideoPlayer* newComponent = new VideoPlayer();

		return newComponent;
	}

	void VideoPlayer::onSceneLoaded()
	{
		if (Engine::getSingleton()->getIsRuntimeMode())
		{
			if (playOnStart)
				play();
		}
	}

	void VideoPlayer::onStateChanged()
	{
		if (!getEnabled())
		{
			if (getIsPlaying())
				stop();
		}
	}

	void VideoPlayer::onRefresh()
	{
		if (videoClip != nullptr && videoClip->isLoaded())
		{
			if (getEnabled() && (gameObject == nullptr || gameObject->getActive()))
			{
				close();
				open();
			}
		}
	}

	void VideoPlayer::setVideoClip(VideoClip* value)
	{
		if (videoClip != nullptr)
		{
			if (opened)
			{
				stop();
				close();
			}
		}

		videoClip = value;

		if (!open())
			return;

		//Create texture
		int w = codec_context->width;
		int h = codec_context->height;

		texture = Texture::create("system/video_player/textures/", GUIDGenerator::genGuid() + "_texture", w, h, 1, Texture::TextureType::Texture2D, bgfx::TextureFormat::RGB8);

		int sz = w * h * 3;
		texture->allocData(sz);
		unsigned char* data = texture->getData();

		for (int i = 0; i < sz; ++i)
			data[i] = 0;

		texture->updateTexture();
		//

		close();

		play(false);
		stop();
	}

	int VideoPlayer::getTotalLength()
	{
		return total_time;
	}

	int VideoPlayer::getPlaybackPosition()
	{
		return currentTime;
	}

	void VideoPlayer::setPlaybackPosition(int value)
	{
		if (format_context == nullptr)
			return;

		clearFrameQueue();

		int64_t ts = av_rescale(
			value,
			format_context->streams[video_stream]->time_base.den,
			format_context->streams[video_stream]->time_base.num
		);

		if (value < getTotalLength())
		{
			time = 0;
			currentTime = value;
			av_seek_frame(format_context, video_stream, ts, AVSEEK_FLAG_FRAME);
		}
	}

	void VideoPlayer::play(bool startThread)
	{
		if (open())
		{
			time = 0;
			currentTime = 0;
			isPlaying = true;
			threadWork = true;

			setPlaybackPosition(0);

			if (startThread)
			{
				worker = std::thread([=]
					{
						while (threadWork)
						{
							frame();
						}
					}
				);

				worker.detach();
			}
		}
	}

	void VideoPlayer::stop()
	{
		bool threadWorked = threadWork;

		isPlaying = false;
		threadWork = false;
		time = 0;
		currentTime = 0;

		if (opened)
		{
			setPlaybackPosition(0);

			if (total_time > 0)
			{
				for (int i = 0; i < 3; ++i)
					frame();

				while (frameQueue.size() > 0)
					updateTexture();

				setPlaybackPosition(0);
			}

			close();
		}
	}

	void VideoPlayer::pause()
	{
		isPlaying = false;
	}

	void VideoPlayer::resume()
	{
		isPlaying = true;
	}

	Color VideoPlayer::getRGBPixel(const AVFrame* frame, int x, int y)
	{
		// Y component
		const unsigned char Y = frame->data[0][frame->linesize[0] * y + x];

		// U, V components 
		x /= 2;
		y /= 2;
		const unsigned char u = frame->data[1][frame->linesize[1] * y + x];
		const unsigned char v = frame->data[2][frame->linesize[2] * y + x];

		// RGB conversion
		const unsigned char r = Y + 1.402 * (v - 128);
		const unsigned char g = Y - 0.344 * (u - 128) - 0.714 * (v - 128);
		const unsigned char b = Y + 1.772 * (u - 128);

		return Color(
			Mathf::clamp(r, 0.0f, 255.0f),
			Mathf::clamp(g, 0.0f, 255.0f),
			Mathf::clamp(b, 0.0f, 255.0f),
			255.0f
		);
	}

	bool VideoPlayer::frame()
	{
		if (format_context == nullptr)
			return true;

		float fps = (float)av_q2d(format_context->streams[video_stream]->r_frame_rate);
		if (frameQueue.size() > (int)fps)
			return true;

		bool frameRead = true;

		AVFrame* frame = av_frame_alloc();
		AVPacket packet;
		if (av_read_frame(format_context, &packet) >= 0)
		{
			AVStream* st = format_context->streams[video_stream];
			
			if (packet.stream_index == video_stream)
			{
				int frame_finished = 0;
				avcodec_decode_video2(codec_context, frame, &frame_finished, &packet);

				if (frame_finished)
				{
					int w = codec_context->width;
					int h = codec_context->height;

					unsigned char* data = new unsigned char[w * h * 3];

					int x = 0;
					int y = -1;

					int pitch = w * 3;
					int length = w * h * 3;

					for (int i = 0; i < length; i += 3)
					{
						if (i % pitch == 0)
						{
							++y;
							x = 0;
						}

						Color color = getRGBPixel(frame, x, h - y - 1);

						data[i + 0] = (unsigned char)color.r();
						data[i + 1] = (unsigned char)color.g();
						data[i + 2] = (unsigned char)color.b();

						++x;
					}

					frameQueue.push_back(data);
				}
			}

			frameRead = true;
		}
		else
		{
			frameRead = false;
		}

		av_free_packet(&packet);
		av_free(frame);

		return frameRead;
	}

	void VideoPlayer::updateTexture()
	{
		if (frameQueue.size() > 0)
		{
			unsigned char* data = texture->getData();
			memcpy(data, frameQueue[0], texture->getSize());
			texture->updateTexture();
			delete[] frameQueue[0];
			frameQueue.erase(frameQueue.begin());
		}
	}

	void VideoPlayer::clearFrameQueue()
	{
		while (frameQueue.size() > 0)
		{
			delete[] frameQueue[0];
			frameQueue.erase(frameQueue.begin());
		}
	}

	int VideoPlayer::zip_read(void* opaque, unsigned char* buf, int buf_size)
	{
		StreamInfo* zip_stream = reinterpret_cast<StreamInfo*>(opaque);
		boost::iostreams::stream<boost::iostreams::array_source>* h = reinterpret_cast<boost::iostreams::stream<boost::iostreams::array_source>*>(zip_stream->stream);

		h->read(reinterpret_cast<char*>(buf), buf_size);

		return h->gcount();
	}

	int64_t VideoPlayer::zip_seek(void* opaque, int64_t offset, int whence)
	{
		StreamInfo* zip_stream = reinterpret_cast<StreamInfo*>(opaque);
		boost::iostreams::stream<boost::iostreams::array_source>* h = reinterpret_cast<boost::iostreams::stream<boost::iostreams::array_source>*>(zip_stream->stream);

		if (whence == AVSEEK_SIZE)
			return zip_stream->totalSize;

		h->clear();

		std::ios_base::seekdir dir = std::ios_base::beg;
		switch (whence)
		{
			case SEEK_SET: dir = std::ios::beg; break;
			case SEEK_CUR: dir = std::ios::cur; break;
			case SEEK_END: dir = std::ios::end; break;
			default: return -1;
		}

		h->seekg((std::streamoff)offset, dir);

		return h->tellg();
	}

	void VideoPlayer::onUpdate(float deltaTime)
	{
		if (!isPlaying)
			return;

		if (format_context == nullptr)
			return;

		float fps = (float)av_q2d(format_context->streams[video_stream]->r_frame_rate);
		float rate = 1.0f / fps;
		
		time += deltaTime * Time::getTimeScale();
		currentTime += deltaTime * Time::getTimeScale();

		while (time >= rate)
		{
			time -= rate;
				
			if (time < 0.0f)
				time = 0.0f;

			if (frameQueue.size() > 0)
				updateTexture();

			if (currentTime >= total_time)
			{
				if (loop)
					setPlaybackPosition(0);
				else
					stop();

				if (Engine::getSingleton()->getIsRuntimeMode())
				{
					void* args[1] = { managedObject };
					APIManager::getSingleton()->execute(managedObject, "CallOnEnded", args, "VideoPlayer");
				}
			}
		}
	}
}
#pragma warning( pop )