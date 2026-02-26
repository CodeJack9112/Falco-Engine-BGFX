#include "Material.h"

#include <iostream>
#include <cassert>

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#include <boost/iostreams/stream.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <fstream>
#include <iostream>

#include "../glm/gtc/type_ptr.hpp"

#include "../Core/Engine.h"
#include "../Core/Debug.h"
#include "../Core/APIManager.h"
#include "../Classes/IO.h"
#include "../Components/Camera.h"
#include "../Renderer/RenderTexture.h"
#include "../Renderer/Renderer.h"

#include "Shader.h"
#include "Texture.h"
#include "Cubemap.h"

#include "../Classes/ZipHelper.h"
#include "../Classes/StringConverter.h"
#include "../Serialization/Assets/SMaterial.h"

namespace GX
{
	std::string Material::ASSET_TYPE = "Material";

	///-------Uniform

	Uniform::~Uniform()
	{
		
	}

	void Uniform::setIsDefine(bool value)
	{
		isDefine = value;
	}

	///-------Material

	Material::Material() : Asset(APIManager::getSingleton()->material_class)
	{
	}

	Material::~Material()
	{
		
	}

	void Material::load()
	{
		if (!isLoaded())
		{
			compileShader();
			Asset::load();
		}
	}

	void Material::unload()
	{
		if (isLoaded())
		{
			Asset::unload();
			uniforms.clear();
			definesList.clear();
		}
	}

	void Material::unloadAll()
	{
		std::vector<Material*> deleteAssets;

		for (auto it = loadedInstances.begin(); it != loadedInstances.end(); ++it)
		{
			if (it->second->getAssetType() == Material::ASSET_TYPE)
			{
				if (!it->second->getPersistent())
					deleteAssets.push_back((Material*)it->second);
			}
		}

		for (auto it = deleteAssets.begin(); it != deleteAssets.end(); ++it)
			delete (*it);

		deleteAssets.clear();
	}

	Material* Material::create(std::string location, std::string name)
	{
		Asset* cachedAsset = getLoadedInstance(location, name);
		//assert(cachedAsset == nullptr && "Asset with these location and name already exists");

		Material* material = nullptr;

		if (cachedAsset != nullptr)
		{
			material = (Material*)cachedAsset;
			material->setShader(nullptr);
		}
		else
		{
			material = new Material();
			material->setLocation(location);
			material->setName(name);
		}

		Shader* shader = Shader::load(Engine::getSingleton()->getBuiltinResourcesPath(), "Shaders/StandardDeferred.shader");
		if (shader != nullptr)
			material->setShader(shader);
		else
			Debug::logWarning("[" + name + "] Material error: Standard shader is missing");

		return material;
	}

	void Material::setShader(Shader* value)
	{
		if (shader == value)
			return;

		shader = value;

		if (shader == nullptr) return;

		compileShader();

		const std::vector<UniformInfo> & shaderUniforms = shader->getUniforms();

		for (auto it = shaderUniforms.begin(); it != shaderUniforms.end(); ++it)
		{
			UniformInfo inf = *it;

			auto uit = std::find_if(uniforms.begin(), uniforms.end(), [=](Uniform& uni) -> bool { return uni.getName() == inf.name; });
			if (uit != uniforms.end())
				continue;

			if (inf.type == "int")
				setUniform<int>(inf.name, atoi(inf.value.c_str()), inf.isDefine);
			if (inf.type == "float")
				setUniform<float>(inf.name, atof(inf.value.c_str()), inf.isDefine);
			if (inf.type == "bool")
				setUniform<bool>(inf.name, inf.value == "true" ? true : false, inf.isDefine);
			if (inf.type == "sampler2D")
				setUniform<Sampler2DDef>(inf.name, std::make_pair(atoi(inf.value.c_str()), nullptr), inf.isDefine);
			if (inf.type == "samplerCube")
				setUniform<SamplerCubeDef>(inf.name, std::make_pair(atoi(inf.value.c_str()), nullptr), inf.isDefine);
			if (inf.type == "vec2")
			{
				std::vector<std::string> strs;
				boost::split(strs, inf.value, boost::is_any_of(","));
				float v1 = 0;
				float v2 = 0;
				if (strs.size() > 0) v1 = atof(strs[0].c_str());
				if (strs.size() > 1) v2 = atof(strs[1].c_str());

				setUniform<glm::vec2>(inf.name, glm::vec2(v1, v2), inf.isDefine);
			}
			if (inf.type == "vec3")
			{
				std::vector<std::string> strs;
				boost::split(strs, inf.value, boost::is_any_of(","));
				float v1 = 0;
				float v2 = 0;
				float v3 = 0;
				if (strs.size() > 0) v1 = atof(strs[0].c_str());
				if (strs.size() > 1) v2 = atof(strs[1].c_str());
				if (strs.size() > 2) v3 = atof(strs[2].c_str());

				setUniform<glm::vec3>(inf.name, glm::vec3(v1, v2, v3), inf.isDefine);
			}
			if (inf.type == "vec4")
			{
				std::vector<std::string> strs;
				boost::split(strs, inf.value, boost::is_any_of(","));
				float v1 = 0;
				float v2 = 0;
				float v3 = 0;
				float v4 = 0;
				if (strs.size() > 0) v1 = atof(strs[0].c_str());
				if (strs.size() > 1) v2 = atof(strs[1].c_str());
				if (strs.size() > 2) v3 = atof(strs[2].c_str());
				if (strs.size() > 3) v4 = atof(strs[3].c_str());

				setUniform<glm::vec4>(inf.name, glm::vec4(v1, v2, v3, v4), inf.isDefine);
			}
			if (inf.type == "color")
			{
				std::vector<std::string> strs;
				boost::split(strs, inf.value, boost::is_any_of(","));
				float v1 = 0;
				float v2 = 0;
				float v3 = 0;
				float v4 = 0;
				if (strs.size() > 0) v1 = atof(strs[0].c_str());
				if (strs.size() > 1) v2 = atof(strs[1].c_str());
				if (strs.size() > 2) v3 = atof(strs[2].c_str());
				if (strs.size() > 3) v4 = atof(strs[3].c_str());

				setUniform<Color>(inf.name, Color(v1, v2, v3, v4), inf.isDefine);
			}
		}
	}

	Material* Material::clone(std::string newName)
	{
		Material* newMat = Material::create(location, newName);

		newMat->shader = shader;
		newMat->uniforms = uniforms;
		newMat->definesList = definesList;
		newMat->definesString = definesString;
		newMat->definesStringHash = definesStringHash;

		newMat->compileShader();
		newMat->load();

		return newMat;
	}

	void Material::save()
	{
		if (location != Engine::getSingleton()->getBuiltinResourcesPath())
		{
			if (location.find("system/materials/") == std::string::npos)
			{
				if (IO::DirExists(Engine::getSingleton()->getAssetsPath()))
				{
					if (!IO::DirExists(IO::GetFilePath(getOrigin())))
						IO::CreateDir(IO::GetFilePath(getOrigin()), true);

					save(getOrigin());
				}
			}
		}
	}

	SMaterial* Material::saveToMemory()
	{
		static SMaterial mat;
		mat = SMaterial();

		if (shader != nullptr)
			mat.shaderName = shader->getName();

		for (auto it = uniforms.begin(); it != uniforms.end(); ++it)
		{
			Uniform& uniform = *it;
			SUniform suniform;

			Sampler2DDef smp = uniform.getValue<Sampler2DDef>();
			std::string texName = "";
			if (smp.second != nullptr)
				texName = smp.second->getName();
			SSampler2D ssmp(smp.first, texName);

			SamplerCubeDef smpCube = uniform.getValue<SamplerCubeDef>();
			texName = "";
			if (smpCube.second != nullptr)
				texName = smpCube.second->getName();
			SSamplerCube ssmpCube(smpCube.first, texName);

			suniform.name = uniform.getName();
			suniform.uniformType = static_cast<int>(uniform.getType());
			suniform.intVal = uniform.getValue<int>();
			suniform.floatVal = uniform.getValue<float>();
			suniform.boolVal = uniform.getValue<bool>();
			suniform.sampler2DVal = ssmp;
			suniform.samplerCubeVal = ssmpCube;
			suniform.vec2Val = uniform.getValue<glm::vec2>();
			suniform.vec3Val = uniform.getValue<glm::vec3>();
			suniform.vec4Val = uniform.getValue<glm::vec4>();
			suniform.colorVal.setValue(uniform.getValue<Color>());

			mat.uniforms.push_back(suniform);
		}

		return &mat;
	}

	void Material::save(std::string path)
	{
		SMaterial mat = *saveToMemory();

		std::ofstream ofs(path, std::ios::binary);
		BinarySerializer s;
		s.serialize(&ofs, &mat, Material::ASSET_TYPE);
		ofs.close();
	}

	void readLegacyMaterialFile(std::string sourceText, SMaterial& mat)
	{
		char c = '0';
		std::string word = "";
		int mode = 0;
		int section = 0;
		bool quote = false;

		std::string texture = "";
		bool color = false;
		std::string red = "";
		std::string green = "";
		std::string blue = "";

		for (uint16_t i = 0; i < sourceText.size(); ++i)
		{
			if (color && !texture.empty())
				break;

			c = sourceText[i];

			if (c == '"')
				quote = !quote;
			
			if ((c == ' ' || c == '\t' || c == '\n') && !quote)
			{
				if (word == "program_shared_parameters")
				{
					mode = 0;
					section = 0;
				}

				if (word == "texture_unit")
				{
					mode = 1;
					section = 0;
				}

				if (!color)
				{
					if (mode == 0)
					{
						if (section == 0)
						{
							if (word == "cDiffuseColor")
								section = 1;
						}
						else if (section == 1)
						{
							if (word == "vec3")
								section = 2;
						}
						else if (section == 2)
						{
							if (red == "")
								red = word;
							else if (green == "")
								green = word;
							else
								blue = word;

							if (!red.empty() && !green.empty() && !blue.empty())
								color = true;
						}
					}
				}

				if (texture == "")
				{
					if (mode == 1)
					{
						if (section == 0)
						{
							if (word == "texture")
								section = 1;
						}
						else if (section == 1)
						{
							texture = boost::replace_all_copy(word, "\"", "");
						}
					}
				}

				word = "";
			}
			else
				word += c;
		}

		if (color)
		{
			float r = atof(red.c_str());
			float g = atof(green.c_str());
			float b = atof(blue.c_str());
			
			SUniform uni = SUniform();
			uni.colorVal = SColor(r, g, b, 1.0f);
			uni.name = "vColor";
			uni.uniformType = static_cast<int>(UniformType::Color);
			mat.uniforms.push_back(uni);
		}

		if (!texture.empty())
		{
			SUniform uni = SUniform();
			uni.sampler2DVal = SSampler2D(0, texture);
			uni.name = "albedoMap";
			uni.uniformType = static_cast<int>(UniformType::Sampler2D);
			mat.uniforms.push_back(uni);
		}

		mat.shaderName = "Shaders/StandardDeferred.shader";
	}

	void Material::load(SMaterial* mat)
	{
		Shader* shader = nullptr;

		if (!mat->shaderName.empty())
		{
			shader = Shader::load(Engine::getSingleton()->getBuiltinResourcesPath(), mat->shaderName);
			if (shader == nullptr)
				shader = Shader::load(location, mat->shaderName);
			if (shader != nullptr)
				setShader(shader);
		}

		if (shader != nullptr)
		{
			for (auto it = mat->uniforms.begin(); it != mat->uniforms.end(); ++it)
			{
				SUniform& uniform = *it;
				const UniformInfo* sinf = shader->getUniform(uniform.name);
				if (sinf == nullptr) continue;

				Texture* texture = nullptr;
				if (!uniform.sampler2DVal.textureName.empty()) texture = Texture::load(location, uniform.sampler2DVal.textureName, true, Texture::CompressionMethod::Default, false);

				Cubemap* cubemap = nullptr;
				if (!uniform.samplerCubeVal.cubemapName.empty()) cubemap = Cubemap::load(location, uniform.samplerCubeVal.cubemapName);

				bool isDefine = sinf->isDefine;

				if (sinf->type == "int") setUniform<int>(uniform.name, uniform.intVal, isDefine);
				if (sinf->type == "float") setUniform<float>(uniform.name, uniform.floatVal, isDefine);
				if (sinf->type == "bool") setUniform<bool>(uniform.name, uniform.boolVal, isDefine);
				if (sinf->type == "sampler2D") setUniform<Sampler2DDef>(uniform.name, std::make_pair(uniform.sampler2DVal.reg, texture), isDefine);
				if (sinf->type == "samplerCube") setUniform<SamplerCubeDef>(uniform.name, std::make_pair(uniform.samplerCubeVal.reg, cubemap), isDefine);
				if (sinf->type == "vec2") setUniform<glm::vec2>(uniform.name, uniform.vec2Val.getValue(), isDefine);
				if (sinf->type == "vec3") setUniform<glm::vec3>(uniform.name, uniform.vec3Val.getValue(), isDefine);
				if (sinf->type == "vec4") setUniform<glm::vec4>(uniform.name, uniform.vec4Val.getValue(), isDefine);
				if (sinf->type == "color") setUniform<Color>(uniform.name, uniform.colorVal.getValue(), isDefine);
			}
		}

		compileShader();
		load();
	}

	Material* Material::load(std::string location, std::string name)
	{
		if (location.empty())
			return nullptr;

		std::string fullPath = location + name;

		Asset* cachedAsset = getLoadedInstance(location, name);

		if (cachedAsset != nullptr && cachedAsset->isLoaded())
		{
			return (Material*)cachedAsset;
		}
		else
		{
			if (IO::isDir(location))
			{
				if (!IO::FileExists(fullPath))
				{
					//if (location != Engine::getSingleton()->getBuiltinResourcesPath())
					//	Debug::logWarning("[" + name + "] Error loading material: file does not exists");

					return nullptr;
				}
			}
			else
			{
				zip_t* arch = Engine::getSingleton()->getZipArchive(location);
				if (!ZipHelper::isFileInZip(arch, name))
					return nullptr;
			}

			Material* material = nullptr;
			if (cachedAsset == nullptr)
			{
				material = new Material();
				material->setLocation(location);
				material->setName(name);
			}
			else
				material = (Material*)cachedAsset;

			SMaterial mat;
			bool legacyFormat = false;

			char* buffer = nullptr;
			try
			{
				if (IO::isDir(location))
				{
					std::ifstream ofs(fullPath, std::ios::binary);
					BinarySerializer s;
					s.deserialize(&ofs, &mat, Material::ASSET_TYPE);
					ofs.close();
				}
				else
				{
					zip_t* arch = Engine::getSingleton()->getZipArchive(location);

					int sz = 0;
					buffer = ZipHelper::readFileFromZip(arch, name, sz);
					boost::iostreams::stream<boost::iostreams::array_source> is(buffer, sz);
					BinarySerializer s;
					s.deserialize(&is, &mat, Material::ASSET_TYPE);
					is.close();
				}
			}
			catch (...)
			{
				legacyFormat = true;
				std::string sourceText = "";
				if (IO::isDir(location))
				{
					sourceText = IO::ReadText(fullPath);
				}
				else
				{
					zip_t* arch = Engine::getSingleton()->getZipArchive(location);
					if (ZipHelper::isFileInZip(arch, name))
					{
						int sz = 0;
						sourceText = ZipHelper::readFileFromZip(arch, name, sz);
						sourceText = IO::Replace(sourceText, "\r\n", "\n");
					}
				}

				if (!sourceText.empty())
					readLegacyMaterialFile(sourceText, mat);
			}
			if (buffer != nullptr)
				delete[] buffer;
			buffer = nullptr;

			material->load(&mat);

			if (legacyFormat)
				material->save();

			return material;
		}
	}

	void Material::reload()
	{
		if (getOrigin().empty())
			return;

		if (isLoaded())
			unload();

		load(location, name);
	}

	bool Material::getUniformIsDefine(std::string name)
	{
		auto it = std::find_if(uniforms.begin(), uniforms.end(), [name](Uniform& uniform) -> bool { return uniform.getName() == name; });

		if (it != uniforms.end())
			return it->getIsDefine();

		return false;
	}

	void Material::updateDefinesString()
	{
		definesString = "";

		for (auto it = definesList.begin(); it != definesList.end(); ++it)
		{
			definesString += *it + ";";
		}

		definesStringHash = std::hash<std::string>{}(definesString);
	}

	void Material::compileShader()
	{
		updateDefinesString();
		
		if (shader != nullptr)
			shader->compile(getDefinesString());
	}

	void Material::checkDefine(std::string name, std::string prevValue, std::string value, bool define)
	{
		std::string defPrev = name + "=" + prevValue;
		std::string def = name + "=" + value;
		auto defIt = std::find(definesList.begin(), definesList.end(), defPrev);
		if (define)
		{
			if (defIt == definesList.end()) definesList.push_back(def);
			else *defIt = def;
		}
		else
		{
			if (defIt != definesList.end()) definesList.erase(defIt);
		}

		updateDefinesString();
	}

	void Material::submitUniforms(ProgramVariant* pv, Camera* camera)
	{
		for (auto it = uniforms.begin(); it != uniforms.end(); ++it)
		{
			Uniform& uniform = *it;

			const UniformVariant* sinf = pv->getUniform(uniform.getNameHash());

			if (uniform.getIsDefine()) continue;
			if (sinf == nullptr || sinf->handle.idx == bgfx::kInvalidHandle)
				continue;

			///Pass uniforms
			if (uniform.getType() == UniformType::Sampler2D)
			{
				Sampler2DDef sampler = uniform.getValue<Sampler2DDef>();

				if (sinf->attribute == ShaderUniformAttribute::BackBufferColor)
				{
					if (camera != nullptr)
					{
						RenderTexture* rt = camera->getBackBuffer();
						bgfx::setTexture(sampler.first, sinf->handle, rt->getColorTextureHandle());
					}
					else
						bgfx::setTexture(sampler.first, sinf->handle, Texture::getNullTexture()->getHandle());
				}
				else if (sinf->attribute == ShaderUniformAttribute::BackBufferDepth)
				{
					if (camera != nullptr)
					{
						RenderTexture* rt = camera->getBackBuffer();
						bgfx::setTexture(sampler.first, sinf->handle, rt->getDepthTextureHandle());
					}
					else
						bgfx::setTexture(sampler.first, sinf->handle, Texture::getNullTexture()->getHandle());
				}
				else
				{
					if (sampler.second != nullptr)
						bgfx::setTexture(sampler.first, sinf->handle, sampler.second->getHandle());
					else
						bgfx::setTexture(sampler.first, sinf->handle, Texture::getNullTexture()->getHandle());
				}
			}
			else if (uniform.getType() == UniformType::SamplerCube)
			{
				SamplerCubeDef sampler = uniform.getValue<SamplerCubeDef>();
				if (sampler.second != nullptr)
					bgfx::setTexture(sampler.first, sinf->handle, sampler.second->getHandle());
				else
					bgfx::setTexture(sampler.first, sinf->handle, Cubemap::getNullCubemap()->getHandle());
			}
			else if (uniform.getType() == UniformType::Int)
			{
				int v = uniform.getValue<int>();
				glm::vec4 val = glm::vec4((float)v, 0, 0, 0);
				bgfx::setUniform(sinf->handle, glm::value_ptr(val), 1);
			}
			else if (uniform.getType() == UniformType::Float)
			{
				float v = uniform.getValue<float>();
				glm::vec4 val = glm::vec4(v, 0, 0, 0);
				bgfx::setUniform(sinf->handle, glm::value_ptr(val), 1);
			}
			else if (uniform.getType() == UniformType::Bool)
			{
				bool v = uniform.getValue<bool>();
				glm::vec4 val = glm::vec4(v ? 1 : 0, 0, 0, 0);
				bgfx::setUniform(sinf->handle, glm::value_ptr(val), 1);
			}
			else if (uniform.getType() == UniformType::Vec2)
			{
				glm::vec4 val = glm::vec4(uniform.getValue<glm::vec2>(), 0.0f, 0.0f);
				bgfx::setUniform(sinf->handle, glm::value_ptr(val), 1);
			}
			else if (uniform.getType() == UniformType::Vec3)
			{
				glm::vec4 val = glm::vec4(uniform.getValue<glm::vec3>(), 0.0f);
				bgfx::setUniform(sinf->handle, glm::value_ptr(val), 1);
			}
			else if (uniform.getType() == UniformType::Vec4)
			{
				glm::vec4 val = uniform.getValue<glm::vec4>();
				bgfx::setUniform(sinf->handle, glm::value_ptr(val), 1);
			}
			else if (uniform.getType() == UniformType::Color)
			{
				Color val = uniform.getValue<Color>();
				bgfx::setUniform(sinf->handle, val.ptr(), 1);
			}
			else if (uniform.getType() == UniformType::Mat3)
			{
				glm::mat3x3 val = uniform.getValue<glm::mat3x3>();
				bgfx::setUniform(sinf->handle, glm::value_ptr(val), 1);
			}
			else if (uniform.getType() == UniformType::Mat4)
			{
				glm::mat4x4 val = uniform.getValue<glm::mat4x4>();
				bgfx::setUniform(sinf->handle, glm::value_ptr(val), 1);
			}
		}
	}

	bool Material::hasUniform(std::string name)
	{
		auto it = std::find_if(uniforms.begin(), uniforms.end(), [name](Uniform& uniform) -> bool { return uniform.getName() == name; });

		if (it != uniforms.end())
			return true;

		return false;
	}
}