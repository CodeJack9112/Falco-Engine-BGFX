#pragma once

#include "SComponent.h"
#include <string>

namespace GX
{
	class SSpline : public SComponent
	{
	public:
		SSpline() {}
		~SSpline() {}

		virtual void serialize(Serializer* s)
		{
			SComponent::serialize(s);
			data(closed);
			data(points);
		}

	public:
		std::vector<SVector3> points;
		bool closed = false;
	};
}