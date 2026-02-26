#pragma once

#include "SComponent.h"
#include <string>

namespace GX
{
	class SFaceInfo : public Archive
	{
	public:
		SFaceInfo() {}
		~SFaceInfo() {}

		virtual int getVersion() { return 1; }

		virtual void serialize(Serializer* s)
		{
			Archive::serialize(s);
			dataVector(indices);
			data(texCoords);
			data(texCoordsScale);
			data(texCoordsOffset);
			data(texCoordsRotation);
			data(material);

			if (version > 0)
				data(smoothNormals);
		}

		std::vector<uint32_t> indices;
		std::vector<SVector2> texCoords;
		SVector2 texCoordsScale = SVector2(1.0f, 1.0f);
		SVector2 texCoordsOffset = SVector2(0.0f, 0.0f);
		float texCoordsRotation = 0.0f;
		bool smoothNormals = true;
		std::string material = "";
	};

	class SCSGBrush : public SComponent
	{
	public:
		SCSGBrush() {}
		~SCSGBrush() {}

		virtual int getVersion() { return 0; }

		virtual void serialize(Serializer* s)
		{
			SComponent::serialize(s);
			data(brushType);
			data(brushOperation);
			data(castShadows);
			data(segments);
			data(stacks);
			data(vertices);
			data(faces);
		}

		int brushType = 0;
		int brushOperation = 0;
		bool castShadows = true;
		int segments = 16;
		int stacks = 16;
		std::vector<SVector3> vertices;
		std::vector<SFaceInfo> faces;
	};
}