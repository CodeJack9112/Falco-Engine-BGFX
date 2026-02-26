#pragma once

#include <string>

#include "SComponent.h"

namespace GX
{
	class SCamera : public SComponent
	{
	public:
		SCamera() {}
		~SCamera() {}

		virtual void serialize(Serializer* s)
		{
			SComponent::serialize(s);
			data(fovy);
			data(znear);
			data(zfar);
			data(viewport_l);
			data(viewport_t);
			data(viewport_w);
			data(viewport_h);
			data(depth);
			data(clearColor);
			data(clearFlags);
			data(projectionType);
			data(orthographicSize);
			data(occlusionCulling);
			dataVector(cullingMask);
		}

	public:
		float fovy = 75;
		float znear = 0.1f;
		float zfar = 1000.0f;

		float viewport_l = 0.0f;	//0..1
		float viewport_t = 0.0f;	//0..1
		float viewport_w = 1.0f;	//0..1
		float viewport_h = 1.0f;	//0..1
		float orthographicSize = 5.0f;

		int depth = 0;
		int projectionType = 0;
		bool occlusionCulling = false;

		SColor clearColor = SColor(1, 1, 1, 1);
		int clearFlags = 0;
		std::vector<int> cullingMask; //bools represented as ints, because std::vector<bool> doesn't work with serialization template functions
	};
}