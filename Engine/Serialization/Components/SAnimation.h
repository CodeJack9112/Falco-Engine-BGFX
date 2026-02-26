#pragma once

#include <string>

#include "SComponent.h"

#include "../Serializers/BinarySerializer.h"

namespace GX
{
	class SAnimationClipInfo : public Archive
	{
	public:
		SAnimationClipInfo() {}
		~SAnimationClipInfo() {}

		virtual void serialize(Serializer* s)
		{
			Archive::serialize(s);
			data(clipPath);
			data(name);
			data(startFrame);
			data(endFrame);
			data(loop);
			data(speed);
		}

	public:
		std::string clipPath = "";
		std::string name = "";
		float speed = 1.0f;
		int startFrame = 0;
		int endFrame = 0;
		bool loop = false;
	};

	class SAnimation : public SComponent
	{
	public:
		SAnimation() {}
		~SAnimation() {}

		virtual void serialize(Serializer* s)
		{
			SComponent::serialize(s);
			data(animationClips);
		}

	public:
		std::vector<SAnimationClipInfo> animationClips;
	};
}