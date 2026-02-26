#pragma once

#include "../../glm/mat4x4.hpp"
#include "SVector.h"

#include "../Serializers/BinarySerializer.h"

namespace GX
{
	struct SMatrix4 : public Archive
	{
	public:
		virtual void serialize(Serializer* s)
		{
			Archive::serialize(s);
			data(a);
			data(b);
			data(c);
			data(d);
		}

		SMatrix4() {}
		SMatrix4(glm::vec4 _a, glm::vec4 _b, glm::vec4 _c, glm::vec4 _d)
		{
			a = _a;
			b = _b;
			c = _c;
			d = _d;
		}
		SMatrix4(glm::mat4x4 mat)
		{
			a = mat[0];
			b = mat[1];
			c = mat[2];
			d = mat[3];
		}

		glm::mat4x4 getValue()
		{
			glm::mat4x4 mt;
			mt[0] = a.getValue();
			mt[1] = b.getValue();
			mt[2] = c.getValue();
			mt[3] = d.getValue();

			return mt;
		}

		SVector4 a = SVector4(0, 0, 0, 0);
		SVector4 b = SVector4(0, 0, 0, 0);
		SVector4 c = SVector4(0, 0, 0, 0);
		SVector4 d = SVector4(0, 0, 0, 0);
	};
}