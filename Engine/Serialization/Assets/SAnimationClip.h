#pragma once

#include <boost/serialization/access.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/vector.hpp>

#include <string>
#include <vector>

#include "../Data/SVector.h"
#include "../Data/SQuaternion.h"

#include "../Serializers/BinarySerializer.h"

namespace GX
{
	class STimeVector3 : public Archive
	{
	public:
		virtual void serialize(Serializer* s)
		{
			Archive::serialize(s);
			data(time);
			data(value);
		}

		STimeVector3() = default;
		STimeVector3(double _time, glm::vec3 _value)
		{
			time = _time;
			value = SVector3(_value);
		}

		~STimeVector3() = default;

		double time = 0;
		SVector3 value = SVector3(0, 0, 0);
	};

	class STimeQuaternion : public Archive
	{
	public:
		virtual void serialize(Serializer* s)
		{
			Archive::serialize(s);
			data(time);
			data(value);
		}

		STimeQuaternion() = default;
		STimeQuaternion(double _time, glm::highp_quat _value)
		{
			time = _time;
			value = SQuaternion(_value);
		}

		~STimeQuaternion() = default;

		double time = 0;
		SQuaternion value = SQuaternion(0, 0, 0, 1);
	};

	class SAnimationClipNode : public Archive
	{
	public:
		virtual void serialize(Serializer* s)
		{
			Archive::serialize(s);
			data(name);
			data(positionKeys);
			data(scalingKeys);
			data(rotationKeys);
		}

		SAnimationClipNode() {}
		~SAnimationClipNode()
		{
			positionKeys.clear();
			scalingKeys.clear();
			rotationKeys.clear();
		}

		std::string name = "";
		std::vector<STimeVector3> positionKeys;
		std::vector<STimeVector3> scalingKeys;
		std::vector<STimeQuaternion> rotationKeys;
	};

	class SAnimationClip : public Archive
	{
	public:
		virtual void serialize(Serializer* s)
		{
			Archive::serialize(s);
			data(duration);
			data(framesPerSecond);
			data(animationNodes);
		}

		SAnimationClip() {}
		~SAnimationClip() {}

		float duration = 0;
		float framesPerSecond = 0;
		std::vector<SAnimationClipNode> animationNodes;
	};
}