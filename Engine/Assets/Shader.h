#pragma once

#include "Asset.h"

#include <cfloat>
#include <climits>
#include <string>
#include <vector>

#include "bgfx/bgfx.h"

#undef None
#undef Always

namespace GX
{
	enum class RenderMode { Forward, Deferred };
	enum class CullMode { None, Clockwise, CounterClockwise };
	enum class DepthFunction { Less, LessOrEqual, Equal, GreaterOrEqual, Greater, NotEqual, Never, Always, Off };
	enum class BlendMode { Replace, Add, Multiply, Alpha };
	enum class StencilFunction { None, Less, LessOrEqual, Equal, GreaterOrEqual, Greater, NotEqual, Never, Always };
	enum class StencilOpFailS { None, Replace, Keep, Incr, IncrSat, Decr, DecrSat, Invert, Zero };
	enum class StencilOpFailZ { None, Replace, Keep, Incr, IncrSat, Decr, DecrSat, Invert, Zero };
	enum class StencilOpPassZ { None, Replace, Keep, Incr, IncrSat, Decr, DecrSat, Invert, Zero };
	enum class IterationMode { Default, PerLight };

	enum class ShaderUniformAttribute
	{
		None,
		BackBufferColor,
		BackBufferDepth,
		CustomTexture
	};

	struct UniformInfo
	{
	public:
		bool isDefine = false;
		std::string type = "";
		std::string bindName = "";
		std::string name = "";
		std::string value = "0";
		float min = FLT_MIN;
		float max = FLT_MAX;
		bool isHidden = false;
		ShaderUniformAttribute attribute = ShaderUniformAttribute::None;
	};

	struct UniformVariant
	{
	public:
		bool isDefine = false;
		std::string type = "";
		std::string bindName = "";
		std::string name = "";
		std::size_t nameHash = 0;
		std::string value = "0";
		float min = FLT_MIN;
		float max = FLT_MAX;
		bgfx::UniformHandle handle = { bgfx::kInvalidHandle };
		bool isHidden = false;
		ShaderUniformAttribute attribute = ShaderUniformAttribute::None;
	};

	struct TagVariant
	{
	public:
		std::string name = "";
		size_t nameHash = 0;
		std::string define = "";
		size_t defineHash = 0;
		std::vector<std::string> values;
		std::pair<std::vector<std::string>, std::vector<std::string>> valuesCond;
		
	};

	struct ProgramVariant
	{
	public:
		ProgramVariant()
		{
			colorWrite[0] = true;
			colorWrite[1] = true;
			colorWrite[2] = true;
			colorWrite[3] = true;
		}

		~ProgramVariant()
		{
			uniforms.clear();
		}

		std::vector<UniformVariant> uniforms;
		
		bgfx::ProgramHandle programHandle = { bgfx::kInvalidHandle };
		std::string preprocessorDefines = "";
		std::size_t preprocessorDefinesHash = 0;

		//Settings
		int stencilMask = INT_MAX;
		int stencilFuncRef = INT_MAX;
		bool depthWrite = true;
		bool depthCheck = true;
		bool colorWrite[4];
		CullMode cullMode = CullMode::Clockwise;
		DepthFunction depthFunction = DepthFunction::LessOrEqual;
		BlendMode blendMode = BlendMode::Replace;
		IterationMode iterationMode = IterationMode::Default;
		StencilFunction stencilFunction = StencilFunction::None;
		StencilOpFailS stencilOpFailS = StencilOpFailS::None;
		StencilOpFailZ stencilOpFailZ = StencilOpFailZ::None;
		StencilOpPassZ stencilOpPassZ = StencilOpPassZ::None;
		//

		const UniformVariant* getUniform(std::string name);
		const UniformVariant* getUniform(std::size_t nameHash);

		uint64_t getRenderState(uint64_t initialState);
	};

	class Pass
	{
		friend class Shader;

	private:
		std::string srcVarying = "";
		std::string srcVertex = "";
		std::string srcFragment = "";

		std::vector<ProgramVariant> programVariants;
		std::vector<TagVariant> tags;

		void setProgramVariantTag(ProgramVariant* pv, std::string name, std::vector<std::string> values);

	public:
		Pass();
		~Pass();

		bgfx::ProgramHandle getProgramHandle(std::string defines);
		ProgramVariant * getProgramVariant(std::string defines);
		ProgramVariant * getProgramVariant(std::size_t definesHash);
	};

	class Shader : public Asset
	{
	private:
		RenderMode renderMode = RenderMode::Forward;

		std::vector<Pass*> passes;
		std::vector<UniformInfo> uniformInfos;

		std::string alias = "";

	public:
		Shader();
		virtual ~Shader();

		static std::string ASSET_TYPE;

		virtual void load();
		virtual void unload();
		virtual void reload();
		virtual std::string getAssetType() { return ASSET_TYPE; }

		static Shader* create(std::string location, std::string name);
		static Shader* load(std::string location, std::string name);
		void loadFromSource(std::string source);
		void loadSelf(std::string location, std::string name);

		std::vector<Pass*>& getPasses() { return passes; }
		const UniformVariant* getUniformVariant(std::string name, std::string defines);
		const std::vector<UniformInfo>& getUniforms() { return uniformInfos; }
		const UniformInfo* getUniform(std::string name);
		
		std::string getAlias() { return alias; }

		Pass* createPass();
		int getPassCount() { return (int)passes.size(); }
		Pass* getPass(int index);
		void removePass(Pass* pass);
		void removePass(int index);
		void removeAllPasses();

		void compile(std::string definesString);

		RenderMode getRenderMode() { return renderMode; }
		void setRenderMode(RenderMode mode) { renderMode = mode; }
	};
}