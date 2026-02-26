#pragma once

#include "../../glm/vec2.hpp"
#include "../../glm/vec3.hpp"
#include "../../glm/vec4.hpp"

#include "../Serializers/BinarySerializer.h"

namespace GX
{
	struct SVector2 : public Archive
	{
	public:
		virtual void serialize(Serializer* s)
		{
			Archive::serialize(s);
			data(x);
			data(y);
		}

		SVector2() {}
		SVector2(float _x, float _y)
		{
			x = _x;
			y = _y;
		}
		SVector2(glm::vec2 vec)
		{
			x = vec.x;
			y = vec.y;
		}
		~SVector2() {}

		float x = 0;
		float y = 0;

		SVector2& operator=(const glm::vec2& other)
		{
			x = other.x;
			y = other.y;

			return *this;
		}

		glm::vec2 getValue()
		{
			return glm::vec2(x, y);
		}
	};

	struct SVector3 : public Archive
	{
	public:
		virtual void serialize(Serializer* s)
		{
			Archive::serialize(s);
			data(x);
			data(y);
			data(z);
		}

		SVector3() {}
		SVector3(float _x, float _y, float _z)
		{
			x = _x;
			y = _y;
			z = _z;
		}
		SVector3(glm::vec3 vec)
		{
			x = vec.x;
			y = vec.y;
			z = vec.z;
		}
		~SVector3() {}

		float x = 0;
		float y = 0;
		float z = 0;

		SVector3& operator=(const glm::vec3& other)
		{
			x = other.x;
			y = other.y;
			z = other.z;

			return *this;
		}

		glm::vec3 getValue()
		{
			return glm::vec3(x, y, z);
		}
	};

	struct SVector4 : public Archive
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

		SVector4() {}
		SVector4(float _x, float _y, float _z, float _w) {
			x = _x;
			y = _y;
			z = _z;
			w = _w;
		}
		~SVector4() {}

		float x = 0;
		float y = 0;
		float z = 0;
		float w = 0;

		SVector4& operator=(const glm::vec4& other)
		{
			x = other.x;
			y = other.y;
			z = other.z;
			w = other.w;

			return *this;
		}

		glm::vec4 getValue()
		{
			return glm::vec4(x, y, z, w);
		}
	};
}