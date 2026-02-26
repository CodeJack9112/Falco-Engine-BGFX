#pragma once

#include "../../glm/gtc/quaternion.hpp"

#include "../Serializers/BinarySerializer.h"

namespace GX
{
	struct SQuaternion : public Archive
	{
	public:
		virtual void serialize(Serializer* s)
		{
			Archive::serialize(s);
			data(x);
			data(y);
			data(z);
			data(w);
		}

		SQuaternion() {}
		SQuaternion(float _x, float _y, float _z, float _w)
		{
			x = _x;
			y = _y;
			z = _z;
			w = _w;
		}
		SQuaternion(glm::highp_quat quat)
		{
			x = quat.x;
			y = quat.y;
			z = quat.z;
			w = quat.w;
		}

		glm::highp_quat getValue()
		{
			return glm::highp_quat(w, x, y, z);
		}

		float x = 0;
		float y = 0;
		float z = 0;
		float w = 0;
	};
}