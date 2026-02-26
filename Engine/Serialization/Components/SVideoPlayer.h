#pragma once

#include "SComponent.h"
#include <string>

namespace GX
{
	class SVideoPlayer : public SComponent
	{
	public:
		SVideoPlayer() {}
		~SVideoPlayer() {}

		virtual void serialize(Serializer* s)
		{
			SComponent::serialize(s);
			data(videoClip);
			data(playOnStart);
			data(loop);
		}

	public:
		std::string videoClip = "";
		bool playOnStart = false;
		bool loop = false;
	};
}