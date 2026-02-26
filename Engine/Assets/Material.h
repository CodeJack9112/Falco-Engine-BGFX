#pragma once

#include "Asset.h"

#include <string>
#include <vector>
#include <algorithm>

#include "bgfx/bgfx.h"
#include "../glm/glm.hpp"

#include "../Renderer/Color.h"

#undef Bool

namespace GX
{
	class Shader;
	struct ProgramVariant;
	class Texture;
	class Cubemap;
	class Camera;
	class SMaterial;

	enum class UniformType
	{
		Int,
		Float,
		Bool,
		Sampler2D,
		SamplerCube,
		Vec2,
		Vec3,
		Vec4,
		Color,
		Mat3,
		Mat4
	};

	typedef std::pair<int, Texture*> Sampler2DDef; // Register, texture
	typedef std::pair<int, Cubemap*> SamplerCubeDef; // Register, cubemap

	struct Uniform
	{
	private:
		UniformType uniformType = UniformType::Int;

		std::string name = "";
		std::size_t nameHash = 0;
		bool isDefine = false;

		int intVal = 0;
		float floatVal = 0;
		bool boolVal = 0;
		Sampler2DDef sampler2DVal;
		SamplerCubeDef samplerCubeVal;
		glm::vec2 vec2Val = glm::vec2(0, 0);
		glm::vec3 vec3Val = glm::vec3(0, 0, 0);
		glm::vec4 vec4Val = glm::vec4(0, 0, 0, 0);
		Color colorVal = Color(1, 1, 1, 1);
		glm::mat3x3 mat3Val = glm::identity<glm::mat3x3>();
		glm::mat4x4 mat4Val = glm::identity<glm::mat4x4>();

	public:
		Uniform() = default;
		~Uniform();

		UniformType getType() { return uniformType; }

		std::string getName() { return name; }
		void setName(std::string value) { name = value; }

		std::size_t getNameHash() { return nameHash; }
		void setNameHash(std::size_t value) { nameHash = value; }

		bool getIsDefine() { return isDefine; }
		void setIsDefine(bool value);

		template<typename T>
		T getValue();

		template<typename T>
		void setValue(T value) {}

		template<typename T>
		std::string getValueString();
	};

	class Material : public Asset
	{
	private:
		Shader* shader = nullptr;

		std::vector<std::string> definesList;
		std::string definesString = "";
		std::size_t definesStringHash = 0;
		std::vector<Uniform> uniforms;

		void checkDefine(std::string name, std::string prevValue, std::string value, bool define);

	public:
		Material();
		virtual ~Material();

		static std::string ASSET_TYPE;

		virtual void load();
		void load(SMaterial* mat);
		virtual void unload();
		static void unloadAll();
		virtual std::string getAssetType() { return ASSET_TYPE; }
		static Material* create(std::string location, std::string name);
		static Material* load(std::string location, std::string name);
		virtual void reload();
		void save();
		void save(std::string path);
		SMaterial* saveToMemory();

		Shader* getShader() { return shader; }
		void setShader(Shader* value);

		Material* clone(std::string newName);

		std::vector<Uniform>& getUniforms() { return uniforms; }
		bool getUniformIsDefine(std::string name);

		std::string getDefinesString() { return definesString; }
		std::size_t getDefinesStringHash() { return definesStringHash; }
		void updateDefinesString();

		void compileShader();
		
		bool hasUniform(std::string name);

		template<typename T>
		T getUniform(std::string name);

		template<typename T>
		void setUniform(std::string name, T value, bool define) {}

		void submitUniforms(ProgramVariant* pv, Camera* camera);
	};

	template<> inline int Uniform::getValue<int>() { return intVal; }
	template<> inline float Uniform::getValue<float>() { return floatVal; }
	template<> inline bool Uniform::getValue<bool>() { return boolVal; }
	template<> inline Sampler2DDef Uniform::getValue<Sampler2DDef>() { return sampler2DVal; }
	template<> inline SamplerCubeDef Uniform::getValue<SamplerCubeDef>() { return samplerCubeVal; }
	template<> inline glm::vec2 Uniform::getValue<glm::vec2>() { return vec2Val; }
	template<> inline glm::vec3 Uniform::getValue<glm::vec3>() { return vec3Val; }
	template<> inline glm::vec4 Uniform::getValue<glm::vec4>() { return vec4Val; }
	template<> inline Color Uniform::getValue<Color>() { return colorVal; }
	template<> inline glm::mat3x3 Uniform::getValue<glm::mat3x3>() { return mat3Val; }
	template<> inline glm::mat4x4 Uniform::getValue<glm::mat4x4>() { return mat4Val; }
		
	template<> inline std::string Uniform::getValueString<int>() { return std::to_string(intVal); }
	template<> inline std::string Uniform::getValueString<float>() { return std::to_string(floatVal); }
	template<> inline std::string Uniform::getValueString<bool>() { return boolVal ? "1" : "0"; }
	template<> inline std::string Uniform::getValueString<glm::vec2>() { return "vec2(" + std::to_string(vec2Val.x) + "," + std::to_string(vec2Val.y) + ")"; }
	template<> inline std::string Uniform::getValueString<glm::vec3>() { return "vec3(" + std::to_string(vec3Val.x) + "," + std::to_string(vec3Val.y) + "," + std::to_string(vec3Val.z) + ")"; }
	template<> inline std::string Uniform::getValueString<glm::vec4>() { return "vec4(" + std::to_string(vec4Val.x) + "," + std::to_string(vec4Val.y) + "," + std::to_string(vec4Val.z) + "," + std::to_string(vec4Val.w) + ")"; }
	template<> inline std::string Uniform::getValueString<Color>() { return "vec4(" + std::to_string(colorVal[0]) + "," + std::to_string(colorVal[1]) + "," + std::to_string(colorVal[2]) + "," + std::to_string(colorVal[3]) + ")"; }

	//Setters
	template<> inline
	void Uniform::setValue<int>(int value)
	{
		uniformType = UniformType::Int;
		intVal = value;
	}

	template<> inline
	void Uniform::setValue<float>(float value)
	{
		uniformType = UniformType::Float;
		floatVal = value;
	}

	template<> inline
	void Uniform::setValue<bool>(bool value)
	{
		uniformType = UniformType::Bool;
		boolVal = value;
	}

	template<> inline
	void Uniform::setValue<Sampler2DDef>(Sampler2DDef value)
	{
		uniformType = UniformType::Sampler2D;
		sampler2DVal = value;
	}

	template<> inline
	void Uniform::setValue<SamplerCubeDef>(SamplerCubeDef value)
	{
		uniformType = UniformType::SamplerCube;
		samplerCubeVal = value;
	}

	template<> inline
	void Uniform::setValue<glm::vec2>(glm::vec2 value)
	{
		uniformType = UniformType::Vec2;
		vec2Val = value;
	}

	template<> inline
	void Uniform::setValue<glm::vec3>(glm::vec3 value)
	{
		uniformType = UniformType::Vec3;
		vec3Val = value;
	}

	template<> inline
	void Uniform::setValue<glm::vec4>(glm::vec4 value)
	{
		uniformType = UniformType::Vec4;
		vec4Val = value;
	}

	template<> inline
	void Uniform::setValue<Color>(Color value)
	{
		uniformType = UniformType::Color;
		colorVal = value;
	}

	template<> inline
	void Uniform::setValue<glm::mat3x3>(glm::mat3x3 value)
	{
		uniformType = UniformType::Mat3;
		mat3Val = value;
	}

	template<> inline
	void Uniform::setValue<glm::mat4x4>(glm::mat4x4 value)
	{
		uniformType = UniformType::Mat4;
		mat4Val = value;
	}


	//Getters

	template<> inline
	int Material::getUniform<int>(std::string name)
	{
		auto it = std::find_if(uniforms.begin(), uniforms.end(), [name](Uniform& uniform) -> bool { return uniform.getName() == name; });

		if (it != uniforms.end())
			return it->getValue<int>();

		return 0;
	}

	template<> inline
	float Material::getUniform<float>(std::string name)
	{
		auto it = std::find_if(uniforms.begin(), uniforms.end(), [name](Uniform& uniform) -> bool { return uniform.getName() == name; });

		if (it != uniforms.end())
			return it->getValue<float>();

		return 0;
	}

	template<> inline
	bool Material::getUniform<bool>(std::string name)
	{
		auto it = std::find_if(uniforms.begin(), uniforms.end(), [name](Uniform& uniform) -> bool { return uniform.getName() == name; });

		if (it != uniforms.end())
			return it->getValue<bool>();

		return false;
	}

	template<> inline
	Sampler2DDef Material::getUniform<Sampler2DDef>(std::string name)
	{
		auto it = std::find_if(uniforms.begin(), uniforms.end(), [name](Uniform& uniform) -> bool { return uniform.getName() == name; });

		if (it != uniforms.end())
			return it->getValue<Sampler2DDef>();

		return std::make_pair(0, nullptr);
	}

	template<> inline
	SamplerCubeDef Material::getUniform<SamplerCubeDef>(std::string name)
	{
		auto it = std::find_if(uniforms.begin(), uniforms.end(), [name](Uniform& uniform) -> bool { return uniform.getName() == name; });

		if (it != uniforms.end())
			return it->getValue<SamplerCubeDef>();

		return std::make_pair(0, nullptr);
	}

	template<> inline
	glm::vec2 Material::getUniform<glm::vec2>(std::string name)
	{
		auto it = std::find_if(uniforms.begin(), uniforms.end(), [name](Uniform& uniform) -> bool { return uniform.getName() == name; });

		if (it != uniforms.end())
			return it->getValue<glm::vec2>();

		return glm::vec2(0, 0);
	}

	template<> inline
	glm::vec3 Material::getUniform<glm::vec3>(std::string name)
	{
		auto it = std::find_if(uniforms.begin(), uniforms.end(), [name](Uniform& uniform) -> bool { return uniform.getName() == name; });

		if (it != uniforms.end())
			return it->getValue<glm::vec3>();

		return glm::vec3(0, 0, 0);
	}

	template<> inline
	glm::vec4 Material::getUniform<glm::vec4>(std::string name)
	{
		auto it = std::find_if(uniforms.begin(), uniforms.end(), [name](Uniform& uniform) -> bool { return uniform.getName() == name; });

		if (it != uniforms.end())
			return it->getValue<glm::vec4>();

		return glm::vec4(0, 0, 0, 0);
	}

	template<> inline
	Color Material::getUniform<Color>(std::string name)
	{
		auto it = std::find_if(uniforms.begin(), uniforms.end(), [name](Uniform& uniform) -> bool { return uniform.getName() == name; });

		if (it != uniforms.end())
			return it->getValue<Color>();

		return Color(1, 1, 1, 1);
	}

	template<> inline
	glm::mat3x3 Material::getUniform<glm::mat3x3>(std::string name)
	{
		auto it = std::find_if(uniforms.begin(), uniforms.end(), [name](Uniform& uniform) -> bool { return uniform.getName() == name; });

		if (it != uniforms.end())
			return it->getValue<glm::mat3x3>();

		return glm::identity<glm::mat3x3>();
	}

	template<> inline
	glm::mat4x4 Material::getUniform<glm::mat4x4>(std::string name)
	{
		auto it = std::find_if(uniforms.begin(), uniforms.end(), [name](Uniform& uniform) -> bool { return uniform.getName() == name; });

		if (it != uniforms.end())
			return it->getValue<glm::mat4x4>();

		return glm::identity<glm::mat4x4>();
	}

	//Setters

	template<> inline
	void Material::setUniform<int>(std::string name, int value, bool define)
	{
		auto it = std::find_if(uniforms.begin(), uniforms.end(), [name](Uniform& uniform) -> bool { return uniform.getName() == name; });

		if (it != uniforms.end())
		{
			bool isDefine = it->getIsDefine();
			int val = it->getValue<int>();
			std::string prevVal = it->getValueString<int>();
			it->setValue<int>(value);
			it->setIsDefine(define);
			if (isDefine != define || (define && val != value))
			{
				checkDefine(name, prevVal, it->getValueString<int>(), define);
				compileShader();
			}
		}
		else
		{
			Uniform uniform;
			uniform.setName(name);
			uniform.setNameHash(std::hash<std::string>{}(name));
			uniform.setValue<int>(value);
			uniform.setIsDefine(define);
			uniforms.push_back(uniform);
			if (define)
			{
				std::string val = uniform.getValueString<int>();
				checkDefine(name, val, val, define);
				compileShader();
			}
		}
	}

	template<> inline
	void Material::setUniform<float>(std::string name, float value, bool define)
	{
		auto it = std::find_if(uniforms.begin(), uniforms.end(), [name](Uniform& uniform) -> bool { return uniform.getName() == name; });

		if (it != uniforms.end())
		{
			bool isDefine = it->getIsDefine();
			float val = it->getValue<float>();
			std::string prevVal = it->getValueString<float>();
			it->setValue<float>(value);
			it->setIsDefine(define);
			if (isDefine != define || (define && val != value))
			{
				checkDefine(name, prevVal, it->getValueString<float>(), define);
				compileShader();
			}
		}
		else
		{
			Uniform uniform;
			uniform.setName(name);
			uniform.setNameHash(std::hash<std::string>{}(name));
			uniform.setValue<float>(value);
			uniform.setIsDefine(define);
			uniforms.push_back(uniform);
			if (define)
			{
				std::string val = uniform.getValueString<float>();
				checkDefine(name, val, val, define);
				compileShader();
			}
		}
	}

	template<> inline
	void Material::setUniform<bool>(std::string name, bool value, bool define)
	{
		auto it = std::find_if(uniforms.begin(), uniforms.end(), [name](Uniform& uniform) -> bool { return uniform.getName() == name; });

		if (it != uniforms.end())
		{
			bool isDefine = it->getIsDefine();
			bool val = it->getValue<bool>();
			std::string prevVal = it->getValueString<bool>();
			it->setValue<bool>(value);
			it->setIsDefine(define);
			if (isDefine != define || (define && val != value))
			{
				checkDefine(name, prevVal, it->getValueString<bool>(), define);
				compileShader();
			}
		}
		else
		{
			Uniform uniform;
			uniform.setName(name);
			uniform.setNameHash(std::hash<std::string>{}(name));
			uniform.setValue<bool>(value);
			uniform.setIsDefine(define);
			uniforms.push_back(uniform);
			if (define)
			{
				std::string val = uniform.getValueString<bool>();
				checkDefine(name, val, val, define);
				compileShader();
			}
		}
	}

	template<> inline
	void Material::setUniform<Sampler2DDef>(std::string name, Sampler2DDef value, bool define)
	{
		auto it = std::find_if(uniforms.begin(), uniforms.end(), [name](Uniform& uniform) -> bool { return uniform.getName() == name; });

		if (it != uniforms.end())
		{
			it->setValue<Sampler2DDef>(value);
			it->setIsDefine(false);
		}
		else
		{
			Uniform uniform;
			uniform.setName(name);
			uniform.setNameHash(std::hash<std::string>{}(name));
			uniform.setValue<Sampler2DDef>(value);
			uniform.setIsDefine(false);
			uniforms.push_back(uniform);
		}
	}

	template<> inline
	void Material::setUniform<SamplerCubeDef>(std::string name, SamplerCubeDef value, bool define)
	{
		auto it = std::find_if(uniforms.begin(), uniforms.end(), [name](Uniform& uniform) -> bool { return uniform.getName() == name; });

		if (it != uniforms.end())
		{
			it->setValue<SamplerCubeDef>(value);
			it->setIsDefine(false);
		}
		else
		{
			Uniform uniform;
			uniform.setName(name);
			uniform.setNameHash(std::hash<std::string>{}(name));
			uniform.setValue<SamplerCubeDef>(value);
			uniform.setIsDefine(false);
			uniforms.push_back(uniform);
		}
	}

	template<> inline
	void Material::setUniform<glm::vec2>(std::string name, glm::vec2 value, bool define)
	{
		auto it = std::find_if(uniforms.begin(), uniforms.end(), [name](Uniform& uniform) -> bool { return uniform.getName() == name; });

		if (it != uniforms.end())
		{
			bool isDefine = it->getIsDefine();
			glm::vec2 val = it->getValue<glm::vec2>();
			std::string prevVal = it->getValueString<glm::vec2>();
			it->setValue<glm::vec2>(value);
			it->setIsDefine(define);
			if (isDefine != define || (define && val != value))
			{
				checkDefine(name, prevVal, it->getValueString<glm::vec2>(), define);
				compileShader();
			}
		}
		else
		{
			Uniform uniform;
			uniform.setName(name);
			uniform.setNameHash(std::hash<std::string>{}(name));
			uniform.setValue<glm::vec2>(value);
			uniform.setIsDefine(define);
			uniforms.push_back(uniform);
			if (define)
			{
				std::string val = uniform.getValueString<glm::vec2>();
				checkDefine(name, val, val, define);
				compileShader();
			}
		}
	}

	template<> inline
	void Material::setUniform<glm::vec3>(std::string name, glm::vec3 value, bool define)
	{
		auto it = std::find_if(uniforms.begin(), uniforms.end(), [name](Uniform& uniform) -> bool { return uniform.getName() == name; });

		if (it != uniforms.end())
		{
			bool isDefine = it->getIsDefine();
			glm::vec3 val = it->getValue<glm::vec3>();
			std::string prevVal = it->getValueString<glm::vec3>();
			it->setValue<glm::vec3>(value);
			it->setIsDefine(define);
			if (isDefine != define || (define && val != value))
			{
				checkDefine(name, prevVal, it->getValueString<glm::vec3>(), define);
				compileShader();
			}
		}
		else
		{
			Uniform uniform;
			uniform.setName(name);
			uniform.setNameHash(std::hash<std::string>{}(name));
			uniform.setValue<glm::vec3>(value);
			uniform.setIsDefine(define);
			uniforms.push_back(uniform);
			if (define)
			{
				std::string val = uniform.getValueString<glm::vec3>();
				checkDefine(name, val, val, define);
				compileShader();
			}
		}
	}

	template<> inline
	void Material::setUniform<glm::vec4>(std::string name, glm::vec4 value, bool define)
	{
		auto it = std::find_if(uniforms.begin(), uniforms.end(), [name](Uniform& uniform) -> bool { return uniform.getName() == name; });

		if (it != uniforms.end())
		{
			bool isDefine = it->getIsDefine();
			glm::vec4 val = it->getValue<glm::vec4>();
			std::string prevVal = it->getValueString<glm::vec4>();
			it->setValue<glm::vec4>(value);
			it->setIsDefine(define);
			if (isDefine != define || (define && val != value))
			{
				checkDefine(name, prevVal, it->getValueString<glm::vec4>(), define);
				compileShader();
			}
		}
		else
		{
			Uniform uniform;
			uniform.setName(name);
			uniform.setNameHash(std::hash<std::string>{}(name));
			uniform.setValue<glm::vec4>(value);
			uniform.setIsDefine(define);
			uniforms.push_back(uniform);
			if (define)
			{
				std::string val = uniform.getValueString<glm::vec4>();
				checkDefine(name, val, val, define);
				compileShader();
			}
		}
	}

	template<> inline
	void Material::setUniform<Color>(std::string name, Color value, bool define)
	{
		auto it = std::find_if(uniforms.begin(), uniforms.end(), [name](Uniform& uniform) -> bool { return uniform.getName() == name; });

		if (it != uniforms.end())
		{
			bool isDefine = it->getIsDefine();
			Color val = it->getValue<Color>();
			std::string prevVal = it->getValueString<Color>();
			it->setValue<Color>(value);
			it->setIsDefine(define);
			if (isDefine != define || (define && val != value))
			{
				checkDefine(name, prevVal, it->getValueString<Color>(), define);
				compileShader();
			}
		}
		else
		{
			Uniform uniform;
			uniform.setName(name);
			uniform.setNameHash(std::hash<std::string>{}(name));
			uniform.setValue<Color>(value);
			uniform.setIsDefine(define);
			uniforms.push_back(uniform);
			if (define)
			{
				std::string val = uniform.getValueString<Color>();
				checkDefine(name, val, val, define);
				compileShader();
			}
		}
	}

	template<> inline
	void Material::setUniform<glm::mat3x3>(std::string name, glm::mat3x3 value, bool define)
	{
		auto it = std::find_if(uniforms.begin(), uniforms.end(), [name](Uniform& uniform) -> bool { return uniform.getName() == name; });

		if (it != uniforms.end())
		{
			it->setValue<glm::mat3x3>(value);
			it->setIsDefine(false);
		}
		else
		{
			Uniform uniform;
			uniform.setName(name);
			uniform.setNameHash(std::hash<std::string>{}(name));
			uniform.setValue<glm::mat3x3>(value);
			uniform.setIsDefine(false);
			uniforms.push_back(uniform);
		}
	}

	template<> inline
	void Material::setUniform<glm::mat4x4>(std::string name, glm::mat4x4 value, bool define)
	{
		auto it = std::find_if(uniforms.begin(), uniforms.end(), [name](Uniform& uniform) -> bool { return uniform.getName() == name; });

		if (it != uniforms.end())
		{
			it->setValue<glm::mat4x4>(value);
			it->setIsDefine(false);
		}
		else
		{
			Uniform uniform;
			uniform.setName(name);
			uniform.setNameHash(std::hash<std::string>{}(name));
			uniform.setValue<glm::mat4x4>(value);
			uniform.setIsDefine(false);
			uniforms.push_back(uniform);
		}
	}
}