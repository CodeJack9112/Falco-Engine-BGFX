#pragma once

#include "../Mono/include/mono/metadata/object.h"

namespace GX
{
	namespace API
	{
		struct Vector4
		{
		public:
			float x, y, z, w;
		};

		struct Vector3
		{
		public:
			float x, y, z;
		};

		struct Vector2
		{
		public:
			float x, y;
		};

		struct Quaternion
		{
		public:
			float x, y, z, w;
		};

		struct Color
		{
		public:
			float r, g, b, a;
		};

		struct Matrix4
		{
		public:
			float m00;
			float m10;
			float m20;
			float m30;
			float m01;
			float m11;
			float m21;
			float m31;
			float m02;
			float m12;
			float m22;
			float m32;
			float m03;
			float m13;
			float m23;
			float m33;
		};

		struct Matrix3
		{
		public:
			float m00;
			float m10;
			float m20;
			float m01;
			float m11;
			float m21;
			float m02;
			float m12;
			float m22;
		};

		struct Rect
		{
		public:
			float m_XMin;
			float m_YMin;
			float m_Width;
			float m_Height;
		};
	}
}