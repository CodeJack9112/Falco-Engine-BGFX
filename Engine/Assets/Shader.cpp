#include "Shader.h"

#include <iostream>

#include "../Core/Engine.h"
#include "../Classes/IO.h"
#include "../Classes/brtshaderc.h"
#include "../Classes/Hash.h"
#include "../Core/Debug.h"
#include "../Core/APIManager.h"
#include "Material.h"

#include "../../LibZip/include/zip.h"
#include "../Classes/ZipHelper.h"
#include "../Classes/StringConverter.h"

namespace GX
{
	std::string Shader::ASSET_TYPE = "Shader";

	struct PassTag
	{
	public:
		std::string name = "";
		std::vector<std::string> values;
	};

	struct PassInfo
	{
	public:
		std::string varying = "";
		std::string vertex = "";
		std::string fragment = "";
		std::vector<PassTag> tags;
	};

	struct ShaderSource
	{
	public:
		std::string name = "";
		std::string mode = "forward";
		std::vector<UniformInfo> uniforms;
		std::vector<PassInfo> passInfo;
	};

	///-------ProgramVariant

	const UniformVariant* ProgramVariant::getUniform(std::string name)
	{
		const UniformVariant* var = nullptr;

		for (auto it = uniforms.begin(); it != uniforms.end(); ++it)
		{
			if (strcmp(it->name.c_str(), name.c_str()) == 0)
			{
				var = &(*it);
				break;
			}
		}

		return var;
	}

	const UniformVariant* ProgramVariant::getUniform(std::size_t nameHash)
	{
		const UniformVariant* var = nullptr;

		for (auto it = uniforms.begin(); it != uniforms.end(); ++it)
		{
			if (it->nameHash == nameHash)
			{
				var = &(*it);
				break;
			}
		}

		return var;
	}

	uint64_t ProgramVariant::getRenderState(uint64_t initialState)
	{
		uint64_t passState = initialState;

		if (blendMode == BlendMode::Add)
			passState |= BGFX_STATE_BLEND_ADD;
		else if (blendMode == BlendMode::Alpha)
		{
			passState |= BGFX_STATE_BLEND_FUNC_SEPARATE(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA, BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_ONE);
		}
		else if (blendMode == BlendMode::Multiply)
			passState |= BGFX_STATE_BLEND_MULTIPLY;

		if (!depthWrite)
			passState &= ~BGFX_STATE_WRITE_Z;

		if (cullMode == CullMode::None)
		{
			passState &= ~BGFX_STATE_CULL_CW;
			passState &= ~BGFX_STATE_CULL_CCW;
		}

		if (cullMode == CullMode::CounterClockwise)
		{
			passState &= ~BGFX_STATE_CULL_CW;
			passState |= BGFX_STATE_CULL_CCW;
		}

		passState &= ~BGFX_STATE_DEPTH_TEST_LEQUAL;
		if (depthFunction == DepthFunction::Always) passState |= BGFX_STATE_DEPTH_TEST_ALWAYS;
		else if (depthFunction == DepthFunction::Equal) passState |= BGFX_STATE_DEPTH_TEST_EQUAL;
		else if (depthFunction == DepthFunction::Greater) passState |= BGFX_STATE_DEPTH_TEST_GREATER;
		else if (depthFunction == DepthFunction::GreaterOrEqual) passState |= BGFX_STATE_DEPTH_TEST_GEQUAL;
		else if (depthFunction == DepthFunction::Less) passState |= BGFX_STATE_DEPTH_TEST_LESS;
		else if (depthFunction == DepthFunction::LessOrEqual) passState |= BGFX_STATE_DEPTH_TEST_LEQUAL;
		else if (depthFunction == DepthFunction::Never) passState |= BGFX_STATE_DEPTH_TEST_NEVER;
		else if (depthFunction == DepthFunction::NotEqual) passState |= BGFX_STATE_DEPTH_TEST_NOTEQUAL;

		if (stencilFunction == StencilFunction::Always) passState |= BGFX_STENCIL_TEST_ALWAYS;
		else if (stencilFunction == StencilFunction::Equal) passState |= BGFX_STENCIL_TEST_EQUAL;
		else if (stencilFunction == StencilFunction::Greater) passState |= BGFX_STENCIL_TEST_GREATER;
		else if (stencilFunction == StencilFunction::GreaterOrEqual) passState |= BGFX_STENCIL_TEST_GEQUAL;
		else if (stencilFunction == StencilFunction::Less) passState |= BGFX_STENCIL_TEST_LESS;
		else if (stencilFunction == StencilFunction::LessOrEqual) passState |= BGFX_STENCIL_TEST_LEQUAL;
		else if (stencilFunction == StencilFunction::Never) passState |= BGFX_STENCIL_TEST_NEVER;
		else if (stencilFunction == StencilFunction::NotEqual) passState |= BGFX_STENCIL_TEST_NOTEQUAL;

		if (stencilFuncRef != INT_MAX) passState |= BGFX_STENCIL_FUNC_REF(stencilFuncRef);
		if (stencilMask != INT_MAX) passState |= BGFX_STENCIL_FUNC_RMASK(stencilMask);

		if (stencilOpFailS == StencilOpFailS::Replace) passState |= BGFX_STENCIL_OP_FAIL_S_REPLACE;
		else if (stencilOpFailS == StencilOpFailS::Keep) passState |= BGFX_STENCIL_OP_FAIL_S_KEEP;
		else if (stencilOpFailS == StencilOpFailS::Decr) passState |= BGFX_STENCIL_OP_FAIL_S_DECR;
		else if (stencilOpFailS == StencilOpFailS::DecrSat) passState |= BGFX_STENCIL_OP_FAIL_S_DECRSAT;
		else if (stencilOpFailS == StencilOpFailS::Incr) passState |= BGFX_STENCIL_OP_FAIL_S_INCR;
		else if (stencilOpFailS == StencilOpFailS::IncrSat) passState |= BGFX_STENCIL_OP_FAIL_S_INCRSAT;
		else if (stencilOpFailS == StencilOpFailS::Invert) passState |= BGFX_STENCIL_OP_FAIL_S_INVERT;
		else if (stencilOpFailS == StencilOpFailS::Zero) passState |= BGFX_STENCIL_OP_FAIL_S_ZERO;

		if (stencilOpFailZ == StencilOpFailZ::Replace) passState |= BGFX_STENCIL_OP_FAIL_Z_REPLACE;
		else if (stencilOpFailZ == StencilOpFailZ::Keep) passState |= BGFX_STENCIL_OP_FAIL_Z_KEEP;
		else if (stencilOpFailZ == StencilOpFailZ::Decr) passState |= BGFX_STENCIL_OP_FAIL_Z_DECR;
		else if (stencilOpFailZ == StencilOpFailZ::DecrSat) passState |= BGFX_STENCIL_OP_FAIL_Z_DECRSAT;
		else if (stencilOpFailZ == StencilOpFailZ::Incr) passState |= BGFX_STENCIL_OP_FAIL_Z_INCR;
		else if (stencilOpFailZ == StencilOpFailZ::IncrSat) passState |= BGFX_STENCIL_OP_FAIL_Z_INCRSAT;
		else if (stencilOpFailZ == StencilOpFailZ::Invert) passState |= BGFX_STENCIL_OP_FAIL_Z_INVERT;
		else if (stencilOpFailZ == StencilOpFailZ::Zero) passState |= BGFX_STENCIL_OP_FAIL_Z_ZERO;

		if (stencilOpPassZ == StencilOpPassZ::Replace) passState |= BGFX_STENCIL_OP_PASS_Z_REPLACE;
		else if (stencilOpPassZ == StencilOpPassZ::Keep) passState |= BGFX_STENCIL_OP_PASS_Z_KEEP;
		else if (stencilOpPassZ == StencilOpPassZ::Decr) passState |= BGFX_STENCIL_OP_PASS_Z_DECR;
		else if (stencilOpPassZ == StencilOpPassZ::DecrSat) passState |= BGFX_STENCIL_OP_PASS_Z_DECRSAT;
		else if (stencilOpPassZ == StencilOpPassZ::Incr) passState |= BGFX_STENCIL_OP_PASS_Z_INCR;
		else if (stencilOpPassZ == StencilOpPassZ::IncrSat) passState |= BGFX_STENCIL_OP_PASS_Z_INCRSAT;
		else if (stencilOpPassZ == StencilOpPassZ::Invert) passState |= BGFX_STENCIL_OP_PASS_Z_INVERT;
		else if (stencilOpPassZ == StencilOpPassZ::Zero) passState |= BGFX_STENCIL_OP_PASS_Z_ZERO;

		return passState;
	}

	///-------Pass

	Pass::Pass()
	{

	}

	Pass::~Pass()
	{
		for (auto it = programVariants.begin(); it != programVariants.end(); ++it)
		{
			ProgramVariant& pv = *it;

			if (bgfx::isValid(pv.programHandle))
				bgfx::destroy(pv.programHandle);
		}

		programVariants.clear();
	}

	bgfx::ProgramHandle Pass::getProgramHandle(std::string defines)
	{
		bgfx::ProgramHandle ph = { bgfx::kInvalidHandle };

		auto it = std::find_if(programVariants.begin(), programVariants.end(), [defines](ProgramVariant& pv) -> bool {
			return pv.preprocessorDefines == defines;
		});

		if (it != programVariants.end())
			ph = it->programHandle;

		return ph;
	}

	ProgramVariant* Pass::getProgramVariant(std::string defines)
	{
		ProgramVariant* result = nullptr;

		for (auto it = programVariants.begin(); it != programVariants.end(); ++it)
		{
			if (it->preprocessorDefines == defines)
			{
				result = &(*it);
				break;
			}
		}

		return result;
	}

	ProgramVariant* Pass::getProgramVariant(std::size_t definesHash)
	{
		ProgramVariant* result = nullptr;

		for (auto it = programVariants.begin(); it != programVariants.end(); ++it)
		{
			if (it->preprocessorDefinesHash == definesHash)
			{
				result = &(*it);
				break;
			}
		}

		return result;
	}

	void Pass::setProgramVariantTag(ProgramVariant* pv, std::string name, std::vector<std::string> values)
	{
		if (name == "backface_culling")
		{
			if (values[0] == "off") pv->cullMode = (CullMode::None);
			if (values[0] == "cw") pv->cullMode = (CullMode::Clockwise);
			if (values[0] == "ccw") pv->cullMode = (CullMode::CounterClockwise);
		}
		if (name == "blend_mode")
		{
			if (values[0] == "replace") pv->blendMode = (BlendMode::Replace);
			if (values[0] == "add") pv->blendMode = (BlendMode::Add);
			if (values[0] == "multiply") pv->blendMode = (BlendMode::Multiply);
			if (values[0] == "alpha") pv->blendMode = (BlendMode::Alpha);
		}
		if (name == "iteration_mode")
		{
			if (values[0] == "default") pv->iterationMode = (IterationMode::Default);
			if (values[0] == "per_light") pv->iterationMode = (IterationMode::PerLight);
		}
		if (name == "depth_write")
		{
			if (values[0] == "on") pv->depthWrite = (true);
			if (values[0] == "off") pv->depthWrite = (false);
		}
		if (name == "depth_func")
		{
			if (values[0] == "always") pv->depthFunction = (DepthFunction::Always);
			if (values[0] == "equal") pv->depthFunction = (DepthFunction::Equal);
			if (values[0] == "greater") pv->depthFunction = (DepthFunction::Greater);
			if (values[0] == "gequal") pv->depthFunction = (DepthFunction::GreaterOrEqual);
			if (values[0] == "less") pv->depthFunction = (DepthFunction::Less);
			if (values[0] == "lequal") pv->depthFunction = (DepthFunction::LessOrEqual);
			if (values[0] == "never") pv->depthFunction = (DepthFunction::Never);
			if (values[0] == "notequal") pv->depthFunction = (DepthFunction::NotEqual);
			if (values[0] == "off") pv->depthFunction = (DepthFunction::Off);
		}
		if (name == "stencil_mask")
		{
			pv->stencilMask = (atoi(values[0].c_str()));
		}
		if (name == "stencil_func")
		{
			if (values[0] == "always") pv->stencilFunction = (StencilFunction::Always);
			if (values[0] == "equal") pv->stencilFunction = (StencilFunction::Equal);
			if (values[0] == "greater") pv->stencilFunction = (StencilFunction::Greater);
			if (values[0] == "gequal") pv->stencilFunction = (StencilFunction::GreaterOrEqual);
			if (values[0] == "less") pv->stencilFunction = (StencilFunction::Less);
			if (values[0] == "lequal") pv->stencilFunction = (StencilFunction::LessOrEqual);
			if (values[0] == "never") pv->stencilFunction = (StencilFunction::Never);
			if (values[0] == "notequal") pv->stencilFunction = (StencilFunction::NotEqual);

			if (values.size() > 1)
				pv->stencilFuncRef = (atoi(values[1].c_str()));
		}
		if (name == "stencil_op")
		{
			if (values[0] == "replace") pv->stencilOpFailS = (StencilOpFailS::Replace);
			if (values[0] == "keep") pv->stencilOpFailS = (StencilOpFailS::Keep);
			if (values[0] == "incr") pv->stencilOpFailS = (StencilOpFailS::Incr);
			if (values[0] == "incrsat") pv->stencilOpFailS = (StencilOpFailS::IncrSat);
			if (values[0] == "decr") pv->stencilOpFailS = (StencilOpFailS::Decr);
			if (values[0] == "decrsat") pv->stencilOpFailS = (StencilOpFailS::DecrSat);
			if (values[0] == "invert") pv->stencilOpFailS = (StencilOpFailS::Invert);
			if (values[0] == "zero") pv->stencilOpFailS = (StencilOpFailS::Zero);
			if (values.size() > 1)
			{
				if (values[1] == "replace") pv->stencilOpFailZ = (StencilOpFailZ::Replace);
				if (values[1] == "keep") pv->stencilOpFailZ = (StencilOpFailZ::Keep);
				if (values[1] == "incr") pv->stencilOpFailZ = (StencilOpFailZ::Incr);
				if (values[1] == "incrsat") pv->stencilOpFailZ = (StencilOpFailZ::IncrSat);
				if (values[1] == "decr") pv->stencilOpFailZ = (StencilOpFailZ::Decr);
				if (values[1] == "decrsat") pv->stencilOpFailZ = (StencilOpFailZ::DecrSat);
				if (values[1] == "invert") pv->stencilOpFailZ = (StencilOpFailZ::Invert);
				if (values[1] == "zero") pv->stencilOpFailZ = (StencilOpFailZ::Zero);
			}
			if (values.size() > 2)
			{
				if (values[2] == "replace") pv->stencilOpPassZ = (StencilOpPassZ::Replace);
				if (values[2] == "keep") pv->stencilOpPassZ = (StencilOpPassZ::Keep);
				if (values[2] == "incr") pv->stencilOpPassZ = (StencilOpPassZ::Incr);
				if (values[2] == "incrsat") pv->stencilOpPassZ = (StencilOpPassZ::IncrSat);
				if (values[2] == "decr") pv->stencilOpPassZ = (StencilOpPassZ::Decr);
				if (values[2] == "decrsat") pv->stencilOpPassZ = (StencilOpPassZ::DecrSat);
				if (values[2] == "invert") pv->stencilOpPassZ = (StencilOpPassZ::Invert);
				if (values[2] == "zero") pv->stencilOpPassZ = (StencilOpPassZ::Zero);
			}
		}
	}

	///-------Shader

	Shader::Shader() : Asset(APIManager::getSingleton()->shader_class)
	{
	}

	Shader::~Shader()
	{
	}

	void Shader::load()
	{
		if (!isLoaded())
		{
			Asset::load();
		}
	}

	void Shader::unload()
	{
		if (isLoaded())
		{
			Asset::unload();

			removeAllPasses();
			uniformInfos.clear();
		}
	}

	void Shader::reload()
	{
		if (getOrigin().empty())
			return;

		if (!IO::FileExists(getOrigin()))
			return;

		std::vector<std::string> defines;

		if (isLoaded())
		{
			for (auto& it : loadedInstances)
			{
				if (it.second->getAssetType() == Material::ASSET_TYPE)
				{
					if (!it.second->isLoaded())
						continue;

					Material* mat = (Material*)it.second;
					if (mat->getShader() != this)
						continue;

					std::string defs = mat->getDefinesString();
					if (std::find(defines.begin(), defines.end(), defs) == defines.end())
						defines.push_back(defs);
				}
			}

			unload();
		}

		load(location, name);

		for (auto& d : defines)
			compile(d);

		defines.clear();
	}

	std::vector<UniformInfo> parseShaderParams(std::string src)
	{
		std::vector<UniformInfo> result;

		char c = src[0];
		std::string word = "";
		int cursor = 0;
		int line = 0;
		int depth = 0;
		bool readLine = false;
		bool readRange = false;
		bool readRangeRead = false;
		bool quotes = false;
		UniformInfo currentUniform;
		while (cursor < src.length())
		{
			c = src[cursor];
			++cursor;

			if (word == "//")
			{
				word = "";
				readLine = false;
				size_t p = src.find("\n", cursor);
				if (p != std::string::npos)
					cursor = p;
				continue;
			}

			if (c == ' ' || c == '\n' || c == '\t' || c == ':' || c == '"')
			{
				if (readLine)
				{
					if (!readRange)
					{
						if (quotes)
						{
							if (c == ' ')
							{
								word += c;
								continue;
							}
						}
						if (c == '"')
						{
							quotes = !quotes;
							continue;
						}

						if (depth == 0)
						{
							if (word == "define")
							{
								currentUniform.isDefine = true;
								depth = -1;
							}
							else if (word == "hidden")
							{
								currentUniform.isHidden = true;
								depth = -1;
							}
							else
								currentUniform.type = word;
						}

						if (depth == 1)
						{
							if (word == "backBufferColor")
							{
								currentUniform.attribute = ShaderUniformAttribute::BackBufferColor;
								depth = 0;
							}
							else if (word == "backBufferDepth")
							{
								currentUniform.attribute = ShaderUniformAttribute::BackBufferDepth;
								depth = 0;
							}
						}
						//if (word != "define")
						//{
							if (depth == 1) currentUniform.bindName = word;
							if (depth == 2) currentUniform.name = word;
							if (currentUniform.type == "int"
								|| currentUniform.type == "float"
								|| currentUniform.type == "bool"
								|| currentUniform.type == "sampler2D"
								|| currentUniform.type == "samplerCube")
							{
								if (depth == 3) currentUniform.value = word;
							}
							if (currentUniform.type == "vec2")
							{
								if (depth == 3) currentUniform.value = word;
								if (depth == 4) currentUniform.value += "," + word;
							}
							if (currentUniform.type == "vec3")
							{
								if (depth == 3) currentUniform.value = word;
								if (depth == 4) currentUniform.value += "," + word;
								if (depth == 5) currentUniform.value += "," + word;
							}
							if (currentUniform.type == "vec4" || currentUniform.type == "color")
							{
								if (depth == 3) currentUniform.value = word;
								if (depth == 4) currentUniform.value += "," + word;
								if (depth == 5) currentUniform.value += "," + word;
								if (depth == 6) currentUniform.value += "," + word;
							}
						//}
					}
					else
					{
						if (readRangeRead)
						{
							if (currentUniform.type == "int" || currentUniform.type == "float")
							{
								if (depth == 0) currentUniform.min = (float)atof(word.c_str());
								if (depth == 1) currentUniform.max = (float)atof(word.c_str());
							}
						}
					}

					if (c == ':')
					{
						readRange = true;
						depth = 0;
					}

					if (c == '\n')
					{
						result.push_back(currentUniform);
						currentUniform = UniformInfo();
					}
				}

				word = "";
				if (c == '\n')
				{
					readLine = false;
					readRange = false;
					readRangeRead = false;
					depth = 0;
					++line;
				}

				if ((c == ' ' || c == '\t' || c == ':') && readLine)
				{
					if (!quotes)
					{
						if (!readRange)
							++depth;
						else
						{
							if (readRangeRead)
								++depth;
						}
					}
				}
			}
			else
			{
				word += c;
				readLine = true;

				if (readRange)
				{
					readRangeRead = true;
				}
			}
		}

		return result;
	}

	std::vector<PassTag> parseTags(std::string src)
	{
		std::vector<PassTag> result;

		char c = src[0];
		std::string word = "";
		int cursor = 0;
		int line = 0;
		int depth = 0;
		bool readLine = false;
		bool quotes = false;
		PassTag currentTag;
		while (cursor < src.length())
		{
			c = src[cursor];
			++cursor;

			if (word == "//")
			{
				word = "";
				readLine = false;
				size_t p = src.find("\n", cursor);
				if (p != std::string::npos)
					cursor = p;
				continue;
			}

			if (c == ' ' || c == '\n' || c == '\t' || c == '"')
			{
				if (readLine)
				{
					if (quotes)
					{
						if (c == ' ')
						{
							word += c;
							continue;
						}
					}
					if (c == '"')
					{
						quotes = !quotes;
						continue;
					}

					if (depth == 0)
						currentTag.name = word;
					else
						currentTag.values.push_back(word);

					if (c == '\n')
					{
						result.push_back(currentTag);
						currentTag = PassTag();
					}
				}

				word = "";
				if (c == '\n')
				{
					readLine = false;
					depth = 0;
					++line;
				}

				if ((c == ' ' || c == '\t') && readLine)
				{
					if (!quotes)
					{
						++depth;
					}
				}
			}
			else
			{
				word += c;
				readLine = true;
			}
		}

		return result;
	}

	ShaderSource parseShader(std::string name, std::string src)
	{
		ShaderSource result;

		bool noErr = true;
		char alphabet[] = { 'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z', 
			'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z', '{', '}', '_' };

		std::string uniforms = "";
		std::string alias = "";
		std::string mode = "";
		std::string tags = "";
		std::string varying = "";
		std::string vertex = "";
		std::string fragment = "";

		enum class Section { Name, Mode, Tags, Varying, Vertex, Fragment, None };
		bool readSection = false;
		bool readParams = false;
		char c = src[0];
		std::string word = "";
		Section section = Section::None;
		int braketCount = 0;
		PassInfo currentPass;
		bool quotes = false;

		int cursor = 0;
		int line = 0;
		while (cursor < src.length())
		{
			c = src[cursor];
			++cursor;

			if (c == '\n') ++line;

			if (!readSection)
			{
				if (readParams)
				{
					if (c != '{' && c != '}')
						uniforms += c;
				}

				bool found = false;
				for (int i = 0; i < sizeof(alphabet) / sizeof(char); ++i)
				{
					if (alphabet[i] == c)
					{
						found = true;
						break;
					}
				}

				if (!found)
					continue;

				word += c;
			}
			else
			{
				if (c == ' ' || c == '\n' || c == '\t')
					word = "";
				else
					word += c;
			}

			if (section == Section::None)
			{
				if (word == "name")
				{
					word = "";
					section = Section::Name;
					readSection = true;
					continue;
				}
				if (word == "render_mode")
				{
					word = "";
					section = Section::Mode;
					readSection = true;
					continue;
				}
				if (word == "pass")
				{
					word = "";
					continue;
				}
				if (word == "params")
				{
					word = "";
					readParams = true;
					continue;
				}
				if (word == "tags")
				{
					word = "";
					section = Section::Tags;
					continue;
				}
				if (word == "varying")
				{
					word = "";
					section = Section::Varying;
					continue;
				}
				if (word == "vertex")
				{
					word = "";
					section = Section::Vertex;
					continue;
				}
				if (word == "fragment")
				{
					word = "";
					section = Section::Fragment;
					continue;
				}
				if (c == '{')
				{
					++braketCount;
					word = "";
					continue;
				}
				if (c == '}')
				{
					if (!readParams)
					{
						--braketCount;
						if (braketCount == 0)
						{
							currentPass.varying = varying;
							currentPass.vertex = vertex;
							currentPass.fragment = fragment;
							currentPass.tags = parseTags(tags);
							uniforms = "";
							tags = "";
							varying = "";
							vertex = "";
							fragment = "";
							result.passInfo.push_back(currentPass);
							currentPass = PassInfo();
						}
					}
					else
					{
						result.uniforms = parseShaderParams(uniforms);
						braketCount = 0;
						readParams = false;
					}
					word = "";
					continue;
				}
			}
			else
			{
				if (!readSection)
				{
					if (c == '{')
					{
						word = "";
						readSection = true;
						continue;
					}
				}
				else
				{
					if (c == '{')
					{
						++braketCount;
					}

					if (c == '}')
					{
						if (braketCount == 1)
						{
							readSection = false;
							section = Section::None;
							word = "";
							continue;
						}

						--braketCount;
					}

					if (section == Section::Tags)
						tags += c;

					if (section == Section::Varying)
						varying += c;

					if (section == Section::Vertex)
						vertex += c;

					if (section == Section::Fragment)
						fragment += c;

					if (section == Section::Name)
					{
						if (c != '\n')
						{
							if (c == '"')
							{
								quotes = !quotes;
								continue;
							}
							if (quotes)
							{
								alias += c;
								continue;
							}
						}
						else
						{
							result.name = alias;
							section = Section::None;
							readSection = false;
							continue;
						}
					}
					if (section == Section::Mode)
					{
						if (c != '\n')
						{
							if (c != ' ' && c != '\t')
							{
								mode += c;
								continue;
							}
						}
						else
						{
							result.mode = mode;
							section = Section::None;
							readSection = false;
							continue;
						}
					}
				}
			}
		}

		return result;
	}

	Shader* Shader::create(std::string location, std::string name)
	{
		Asset* cachedAsset = getLoadedInstance(location, name);
		//assert(cachedAsset == nullptr && "Asset with these location and name already exists");

		Shader* shader = nullptr;

		if (cachedAsset != nullptr)
		{
			shader = (Shader*)cachedAsset;
		}
		else
		{
			shader = new Shader();
			shader->setLocation(location);
			shader->setName(name);
		}

		return shader;
	}

	Shader* Shader::load(std::string location, std::string name)
	{
		std::string fullPath = location + name;

		if (IO::isDir(location))
		{
			if (!IO::FileExists(fullPath))
			{
				if (location == Engine::getSingleton()->getAssetsPath())
					Debug::logWarning("[" + fullPath + "] Error loading shader: file does not exists");

				return nullptr;
			}
		}
		else
		{
			zip_t* arch = Engine::getSingleton()->getZipArchive(location);
			if (!ZipHelper::isFileInZip(arch, name))
			{
				Debug::logWarning("[" + fullPath + "] Error loading shader: file does not exists");

				return nullptr;
			}
		}

		Asset* cachedAsset = getLoadedInstance(location, name);

		if (cachedAsset != nullptr && cachedAsset->isLoaded())
		{
			return (Shader*)cachedAsset;
		}
		else
		{
			Shader* shader = nullptr;
			if (cachedAsset == nullptr)
			{
				shader = new Shader();
				shader->setLocation(location);
				shader->setName(name);
			}
			else
				shader = (Shader*)cachedAsset;

			shader->loadSelf(location, name);

			if (!shader->isLoaded())
			{
				Debug::logWarning("[" + fullPath + "] Error loading shader: compile error");
			}

			return shader;
		}
	}

	void Shader::loadFromSource(std::string source)
	{
		ShaderSource shaderInfo = parseShader(name, source);

		alias = shaderInfo.name;

		if (shaderInfo.mode == "forward") renderMode = RenderMode::Forward;
		if (shaderInfo.mode == "deferred") renderMode = RenderMode::Deferred;

		if (shaderInfo.passInfo.size() > 0)
		{
			uniformInfos = shaderInfo.uniforms;
		}
		else
			Debug::logWarning("[" + name + "] Shader warning: no passes found");

		int p = 0;
		for (auto it = shaderInfo.passInfo.begin(); it != shaderInfo.passInfo.end(); ++it, ++p)
		{
			PassInfo inf = *it;

			Pass* pass = createPass();
			pass->srcVarying = inf.varying;
			pass->srcVertex = inf.vertex;
			pass->srcFragment = inf.fragment;

			for (auto p = inf.tags.begin(); p != inf.tags.end(); ++p)
			{
				if (p->values.size() > 0)
				{
					TagVariant tag;
					tag.name = p->name;
					tag.nameHash = Hash::getHash(tag.name);

					if (p->values[0] != "=")
					{
						tag.values = p->values;
					}
					else
					{
						if (p->values.size() > 3)
						{
							tag.define = p->values[1];
							tag.defineHash = Hash::getHash(tag.define);
							if (p->values[2] == "?")
							{
								int mode = 0;
								for (int t = 3; t < p->values.size(); ++t)
								{
									if (p->values[t] == ":")
									{
										mode = 1;
										continue;
									}

									if (mode == 0)
										tag.valuesCond.first.push_back(p->values[t]);
									else
										tag.valuesCond.second.push_back(p->values[t]);
								}
							}
						}
					}

					pass->tags.push_back(tag);
				}
			}
		}

		if (passes.size() > 0)
			load();
	}

	void Shader::loadSelf(std::string location, std::string name)
	{
		std::string src = "";

		if (IO::isDir(location))
		{
			std::string fullPath = location + name;
			src = IO::ReadText(fullPath);
		}
		else
		{
			zip_t* arch = Engine::getSingleton()->getZipArchive(location);
			int sz = 0;
			src = ZipHelper::readFileFromZip(arch, name, sz);
			src = IO::Replace(src, "\r\n", "\n");
		}

		loadFromSource(src);
	}

	const UniformVariant* Shader::getUniformVariant(std::string name, std::string defines)
	{
		const UniformVariant* result = nullptr;

		for (auto it = passes.begin(); it != passes.end(); ++it)
		{
			Pass* pass = *it;

			ProgramVariant * pv = pass->getProgramVariant(defines);

			if (pv != nullptr)
			{
				const UniformVariant * uni = pv->getUniform(name);
				if (uni != nullptr)
				{
					result = uni;
					break;
				}
			}
		}

		return result;
	}

	const UniformInfo* Shader::getUniform(std::string name)
	{
		auto u = std::find_if(uniformInfos.begin(), uniformInfos.end(), [name](UniformInfo& uniform) -> bool { return uniform.name == name; });

		if (u != uniformInfos.end())
		{
			return &(*u);
		}

		return nullptr;
	}

	Pass* Shader::createPass()
	{
		Pass* pass = new Pass();
		passes.push_back(pass);

		return pass;
	}

	Pass* Shader::getPass(int index)
	{
		if (passes.size() > 0)
			return passes.at(index);

		return nullptr;
	}

	void Shader::removePass(Pass* pass)
	{
		auto it = std::find(passes.begin(), passes.end(), pass);

		if (it != passes.end())
			passes.erase(it);

		delete pass;
	}

	void Shader::removePass(int index)
	{
		if (index < passes.size())
		{
			auto it = passes.begin() + index;
			Pass* pass = *it;
			passes.erase(it);

			delete pass;
		}
	}

	void Shader::removeAllPasses()
	{
		for (auto it = passes.begin(); it != passes.end(); ++it)
			delete* it;

		passes.clear();
	}

	void Shader::compile(std::string definesString)
	{
		bool exists = false;
		for (auto it = passes.begin(); it != passes.end(); ++it)
		{
			Pass* pass = *it;

			auto ip = find_if(pass->programVariants.begin(), pass->programVariants.end(), [definesString](ProgramVariant& pv) -> bool {
				return pv.preprocessorDefines == definesString;
			});

			if (ip != pass->programVariants.end())
			{
				exists = true;
				break;
			}
		}

		if (exists)
			return;

		int p = 0;
		std::string srcVarying = "";
		std::string srcVertex = "";
		std::string srcFragment = "";

		for (auto it = passes.begin(); it != passes.end(); ++it, ++p)
		{
			Pass* pass = *it;

			if (!pass->srcVarying.empty()) srcVarying = pass->srcVarying;
			if (!pass->srcVertex.empty()) srcVertex = pass->srcVertex;
			if (!pass->srcFragment.empty()) srcFragment = pass->srcFragment;

			//compile vertex shader
			const bgfx::Memory* memVsh = shaderc::compileShaderFromSources(shaderc::ST_VERTEX, getOrigin().c_str(), srcVertex.c_str(), definesString.c_str(), srcVarying.c_str());

			// compile fragment shader
			const bgfx::Memory* memFsh = shaderc::compileShaderFromSources(shaderc::ST_FRAGMENT, getOrigin().c_str(), srcFragment.c_str(), definesString.c_str(), srcVarying.c_str());

			if (memVsh != nullptr && memFsh != nullptr)
			{
				bgfx::ShaderHandle vsh = bgfx::createShader(memVsh);
				bgfx::ShaderHandle fsh = bgfx::createShader(memFsh);

				// build program using shaders
				bgfx::ProgramHandle programHandle = bgfx::createProgram(vsh, fsh, true);

				ProgramVariant * pp = nullptr;

				auto ip = find_if(pass->programVariants.begin(), pass->programVariants.end(), [programHandle, definesString](ProgramVariant& pv) -> bool {
					return pv.programHandle.idx == programHandle.idx && pv.preprocessorDefines == definesString;
				});

				if (ip == pass->programVariants.end())
				{
					ProgramVariant pp1 = ProgramVariant();
					pp1.programHandle = programHandle;
					pp1.preprocessorDefines = definesString;
					pp1.preprocessorDefinesHash = std::hash<std::string>{}(definesString);

					pass->programVariants.push_back(pp1);

					pp = &pass->programVariants[pass->programVariants.size() - 1];
				}
				else
					pp = &(*ip);

				pp->uniforms.clear();

				for (auto tg = pass->tags.begin(); tg != pass->tags.end(); ++tg)
				{
					TagVariant& tag = *tg;

					if (tag.define.empty())
					{
						pass->setProgramVariantTag(pp, tag.name, tag.values);
					}
					else
					{
						if (definesString.find(tag.define + "=1") != std::string::npos)
							pass->setProgramVariantTag(pp, tag.name, tag.valuesCond.first);
						else
							pass->setProgramVariantTag(pp, tag.name, tag.valuesCond.second);
					}
				}

				bgfx::UniformHandle* handles = new bgfx::UniformHandle[64];

				//Vertex uniforms
				memset(handles, 0, 64);
				int sz = bgfx::getShaderUniforms(vsh, handles, 64);
				for (int i = 0; i < sz; ++i)
				{
					bgfx::UniformInfo inf;
					bgfx::getUniformInfo(handles[i], inf);
					auto v = std::find_if(uniformInfos.begin(), uniformInfos.end(), [=](UniformInfo& uinf) -> bool {
						return uinf.name == inf.name;
					});
					
					if (v != uniformInfos.end())
					{
						auto u = std::find_if(pp->uniforms.begin(), pp->uniforms.end(), [=](UniformVariant& uvar) -> bool { return uvar.name == v->name; });
						if (u == pp->uniforms.end())
						{
							UniformVariant ui;
							ui.bindName = v->bindName;
							ui.isDefine = v->isDefine;
							ui.max = v->max;
							ui.min = v->min;
							ui.name = v->name;
							ui.nameHash = std::hash<std::string>{}(v->name);
							ui.type = v->type;
							ui.value = v->value;
							ui.handle = handles[i];
							ui.isHidden = v->isHidden;
							ui.attribute = v->attribute;
							pp->uniforms.push_back(ui);
						}
					}
				}

				delete[] handles;
				handles = new bgfx::UniformHandle[64];

				//Fragment uniforms
				memset(handles, 0, 64);
				sz = bgfx::getShaderUniforms(fsh, handles, 64);
				for (int i = 0; i < sz; ++i)
				{
					bgfx::UniformInfo inf;
					bgfx::getUniformInfo(handles[i], inf);
					auto v = std::find_if(uniformInfos.begin(), uniformInfos.end(), [=](UniformInfo& uinf) -> bool {
						return uinf.name == inf.name;
					});
					if (v != uniformInfos.end())
					{
						auto u = std::find_if(pp->uniforms.begin(), pp->uniforms.end(), [=](UniformVariant& uvar) -> bool { return uvar.name == v->name; });
						if (u == pp->uniforms.end())
						{
							UniformVariant ui;
							ui.bindName = v->bindName;
							ui.isDefine = v->isDefine;
							ui.max = v->max;
							ui.min = v->min;
							ui.name = v->name;
							ui.nameHash = std::hash<std::string>{}(v->name);
							ui.type = v->type;
							ui.value = v->value;
							ui.handle = handles[i];
							ui.isHidden = v->isHidden;
							ui.attribute = v->attribute;
							pp->uniforms.push_back(ui);
						}
					}
				}

				delete[] handles;

				int upos = 0;
				for (auto u = uniformInfos.begin(); u != uniformInfos.end(); ++u, ++upos)
				{
					if (u->isDefine)
					{
						UniformVariant ui;
						ui.bindName = u->bindName;
						ui.isDefine = u->isDefine;
						ui.name = u->name;
						ui.nameHash = std::hash<std::string>{}(u->name);
						ui.type = u->type;
						ui.value = u->value;
						ui.isHidden = u->isHidden;
						ui.attribute = u->attribute;

						if (upos < pp->uniforms.size())
							pp->uniforms.insert(pp->uniforms.begin() + upos, ui);
						else
							pp->uniforms.push_back(ui);
					}
				}
			}
			else
			{
				Debug::logWarning("[" + getName() + "] Error parsing shader: syntax error (Pass " + std::to_string(p) + ")");
			}
		}
	}
}