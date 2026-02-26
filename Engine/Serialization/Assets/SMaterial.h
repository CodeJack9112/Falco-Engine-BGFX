#pragma once

#include <string>
#include <vector>

#include "../Data/SVector.h"
#include "../Data/SColor.h"

#include "../Serializers/BinarySerializer.h"

namespace GX
{
	struct SSampler2D : public Archive
	{
	public:
		virtual void serialize(Serializer* s)
		{
			Archive::serialize(s);
			data(reg);
			data(textureName);
		}

		SSampler2D() {}
		SSampler2D(int r, std::string path) { reg = r; textureName = path; }
		~SSampler2D() {}

		int reg = 0;
		std::string textureName = "";
	};

	struct SSamplerCube : public Archive
	{
	public:
		virtual void serialize(Serializer* s)
		{
			Archive::serialize(s);
			data(reg);
			data(cubemapName);
		}

		SSamplerCube() {}
		SSamplerCube(int r, std::string path) { reg = r; cubemapName = path; }
		~SSamplerCube() {}

		int reg = 0;
		std::string cubemapName = "";
	};

	struct SUniform : public Archive
	{
	public:
		virtual void serialize(Serializer* s)
		{
			Archive::serialize(s);
			data(name);
			data(uniformType);
			data(intVal);
			data(floatVal);
			data(boolVal);
			data(vec2Val);
			data(vec3Val);
			data(vec4Val);
			data(colorVal);
			data(sampler2DVal);
			data(samplerCubeVal);
		}

		SUniform() {}
		~SUniform() {}

		std::string name = "";
		int uniformType = 0;
		int intVal = 0;
		float floatVal = 0;
		bool boolVal = 0;
		SVector2 vec2Val = SVector2(0, 0);
		SVector3 vec3Val = SVector3(0, 0, 0);
		SVector4 vec4Val = SVector4(0, 0, 0, 0);
		SColor colorVal = SColor(1, 1, 1, 1);
		SSampler2D sampler2DVal;
		SSamplerCube samplerCubeVal;
	};

	class SMaterial : public Archive
	{
	public:
		virtual void serialize(Serializer* s)
		{
			Archive::serialize(s);
			data(shaderName);
			data(uniforms);
		}

		SMaterial() {}
		~SMaterial() {}

		std::string shaderName = "";
		std::vector<SUniform> uniforms;
	};
}