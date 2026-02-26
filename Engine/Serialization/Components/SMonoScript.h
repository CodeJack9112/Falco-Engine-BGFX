#pragma once

#include <boost/serialization/vector.hpp>

namespace GX
{
	class SMonoFieldInfo : public Archive
	{
	public:
		SMonoFieldInfo() = default;
		~SMonoFieldInfo() {}

		virtual void serialize(Serializer* s)
		{
			Archive::serialize(s);
			data(fieldName);
			data(fieldType);
			data(stringVal);
			data(boolVal);
			data(intVal);
			data(floatVal);
			data(objectVal);
			data(vec2Val);
			data(vec3Val);
			data(vec4Val);
			data(stringArrVal);
		}

	public:
		std::string fieldName = "";
		std::string fieldType = "";

		std::string stringVal = "";
		std::vector<std::string> stringArrVal;
		bool boolVal = false;
		int intVal = 0;
		float floatVal = 0;
		std::string objectVal = "[None]";
		SVector2 vec2Val = SVector2(0, 0);
		SVector3 vec3Val = SVector3(0, 0, 0);
		SVector4 vec4Val = SVector4(0, 0, 0, 0);
	};

	class SMonoScript : public SComponent
	{
	public:
		SMonoScript() = default;
		~SMonoScript() {}

		virtual void serialize(Serializer* s)
		{
			SComponent::serialize(s);
			data(className);
			data(fieldList);
		}

	public:
		std::string className;
		std::vector<SMonoFieldInfo> fieldList;
	};
}