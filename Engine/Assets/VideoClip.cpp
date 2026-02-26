#include "VideoClip.h"

#include "../Core/Engine.h"
#include "../Core/APIManager.h"
#include "../Core/Debug.h"
#include "../Classes/IO.h"
#include "../Classes/ZipHelper.h"
#include "../Classes/wave.h"

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/opt.h>
#include <libswresample/swresample.h>
}

namespace GX
{
	std::string VideoClip::ASSET_TYPE = "VideoClip";

	VideoClip::VideoClip() : Asset(APIManager::getSingleton()->videoclip_class)
	{

	}

	VideoClip::~VideoClip()
	{
	}

	void VideoClip::unload()
	{
		if (isLoaded())
		{
			Asset::unload();
		}
	}

	void VideoClip::unloadAll()
	{
		std::vector<VideoClip*> deleteAssets;

		for (auto it = loadedInstances.begin(); it != loadedInstances.end(); ++it)
		{
			if (it->second->getAssetType() == VideoClip::ASSET_TYPE)
			{
				if (!it->second->getPersistent())
					deleteAssets.push_back((VideoClip*)it->second);
			}
		}

		for (auto it = deleteAssets.begin(); it != deleteAssets.end(); ++it)
			delete (*it);

		deleteAssets.clear();
	}

	void VideoClip::load()
	{
		if (!isLoaded())
		{
			Asset::load();
		}
	}

	void VideoClip::reload()
	{
		if (getOrigin().empty())
			return;

		if (isLoaded())
			unload();

		load(location, name);
	}

	VideoClip* VideoClip::load(std::string location, std::string name, bool warn)
	{
		std::string fullPath = location + name;

		Asset* cachedAsset = getLoadedInstance(location, name);

		if (cachedAsset != nullptr && cachedAsset->isLoaded())
		{
			return (VideoClip*)cachedAsset;
		}
		else
		{
			VideoClip* clip = nullptr;
			if (cachedAsset == nullptr)
			{
				clip = new VideoClip();
				clip->setLocation(location);
				clip->setName(name);
			}
			else
				clip = (VideoClip*)cachedAsset;

			if (IO::isDir(location))
			{
				if (!IO::FileExists(fullPath))
				{
					if (warn)
						Debug::log("[" + fullPath + "] Error loading video clip: file does not exists", Debug::DbgColorYellow);
					return nullptr;
				}
			}
			else
			{
				zip_t* arch = Engine::getSingleton()->getZipArchive(location);
				if (!ZipHelper::isFileInZip(arch, name))
				{
					if (warn)
						Debug::log("[" + fullPath + "] Error loading video clip: file does not exists", Debug::DbgColorYellow);
					return nullptr;
				}
			}

			clip->load();

			return clip;
		}
	}

#pragma warning( push )
#pragma warning( disable : 4996 )
	void VideoClip::extractAudio()
	{
		std::string filename = getOrigin();

		AVFormatContext* format_context = nullptr;
		AVCodecContext* codec_context = nullptr;
		AVCodec* codec = nullptr;
		int audio_stream = 0;

		int err = avformat_open_input(&format_context, filename.c_str(), NULL, NULL);
		if (err < 0)
		{
			Debug::log("ffmpeg: Unable to open input file", Debug::DbgColorRed);
			return;
		}

		err = avformat_find_stream_info(format_context, NULL);
		if (err < 0)
		{
			Debug::log("ffmpeg: Unable to find stream info", Debug::DbgColorRed);
			return;
		}

		// Find the first video stream
		for (audio_stream = 0; audio_stream < format_context->nb_streams; ++audio_stream)
		{
			if (format_context->streams[audio_stream]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
				break;
		}

		if (audio_stream == format_context->nb_streams)
		{
			Debug::log("ffmpeg: Unable to find audio stream", Debug::DbgColorRed);
			return;
		}

		codec_context = format_context->streams[audio_stream]->codec;
		codec = avcodec_find_decoder(codec_context->codec_id);
		err = avcodec_open2(codec_context, codec, NULL);
		if (err < 0)
		{
			Debug::log("ffmpeg: Unable to open codec", Debug::DbgColorRed);
			return;
		}

		////////

		std::string fileName = IO::GetFilePath(getOrigin()) + IO::GetFileName(getOrigin()) + ".wav";

		// prepare resampler
		struct SwrContext* swr = swr_alloc();
		av_opt_set_int(swr, "in_channel_count", codec_context->channels, 0);
		av_opt_set_int(swr, "out_channel_count", 1, 0);
		av_opt_set_int(swr, "in_channel_layout", codec_context->channel_layout, 0);
		av_opt_set_int(swr, "out_channel_layout", AV_CH_LAYOUT_MONO, 0);
		av_opt_set_int(swr, "in_sample_rate", codec_context->sample_rate, 0);
		av_opt_set_int(swr, "out_sample_rate", 44100, 0);
		av_opt_set_sample_fmt(swr, "in_sample_fmt", codec_context->sample_fmt, 0);
		av_opt_set_sample_fmt(swr, "out_sample_fmt", AV_SAMPLE_FMT_FLT, 0);
		swr_init(swr);
		if (!swr_is_initialized(swr)) {
			Debug::log("ffmpeg:: Resampler has not been properly initialized", Debug::DbgColorRed);
		}

		// prepare to read data
		AVPacket packet;
		av_init_packet(&packet);

		// iterate through frames
		std::vector<float> data;

		while (av_read_frame(format_context, &packet) >= 0)
		{
			AVFrame* frame = av_frame_alloc();

			AVStream* st = format_context->streams[audio_stream];
				
			if (packet.stream_index == audio_stream)
			{
				// decode one frame
				int gotFrame = 0;
				if (avcodec_decode_audio4(codec_context, frame, &gotFrame, &packet) < 0)
					break;

				if (!gotFrame)
					continue;

				// resample frames
				float* buffer = nullptr;
				av_samples_alloc((uint8_t**)&buffer, NULL, 1, frame->nb_samples, AV_SAMPLE_FMT_FLT, 0);
				int frame_count = swr_convert(swr, (uint8_t**)&buffer, frame->nb_samples, (const uint8_t**)frame->data, frame->nb_samples);
				
				int cur = data.size();
				data.resize(cur + frame->nb_samples);
				memcpy(data.data() + cur, buffer, frame_count * sizeof(float));

				av_free(buffer);
			}

			av_free(frame);
			av_free_packet(&packet);
		}

		// clean up
		swr_free(&swr);

		pcm_to_wav((char*)data.data(), data.size() * sizeof(float), 1, 44100, 32, fileName.c_str());
		data.clear();
		
		// Close the codec
		if (codec_context != nullptr)
			avcodec_close(codec_context);

		// Close the video file
		if (format_context != nullptr)
			avformat_close_input(&format_context);

		codec_context = nullptr;
		format_context = nullptr;
	}
#pragma warning( pop )
}