#pragma once

#include "PropertyEditor.h"

#include "../Engine/glm/vec2.hpp"
#include "../Engine/glm/vec3.hpp"
#include "../Engine/glm/vec4.hpp"
#include "../Engine/glm/gtc/quaternion.hpp"
#include "../Engine/Renderer/Color.h"

#include <bgfx/bgfx.h>

namespace GX
{
	class TreeNode;
	class Material;
	class RenderTexture;
	class Mesh;
	class Texture;
	class Cubemap;

	class MaterialEditor : public PropertyEditor
	{
	private:
		Material* material = nullptr;
		RenderTexture* rt = nullptr;
		Mesh* sphereMesh = nullptr;

	public:
		MaterialEditor();
		~MaterialEditor();

		virtual void init(Material * mat, bool showPreview = false);
		virtual void update();
		virtual void updateEditor();

		bool getIsShaderEditable() { return isShaderEditable; }
		void setIsShaderEditable(bool value) { isShaderEditable = value; }

		bool getSaveOnEdit() { return saveOnEdit; }
		void setSaveOnEdit(bool value) { saveOnEdit = value; }

	private:
		bool isShaderEditable = true;
		bool saveOnEdit = true;
		float prevWidth = 0.0f;
		float prevHeight = 0.0f;
		bool mousePressed = false;
		glm::vec2 prevMousePos = glm::vec2(0.0f);
		glm::vec3 camPos = glm::vec3(-1, 0, 0);
		glm::highp_quat rotX = glm::identity<glm::highp_quat>();
		glm::highp_quat rotY = glm::identity<glm::highp_quat>();
		glm::mat4x4 meshTransform = glm::identity<glm::mat4x4>();

		bgfx::FrameBufferHandle gbuffer = { bgfx::kInvalidHandle };
		bgfx::FrameBufferHandle lightBuffer = { bgfx::kInvalidHandle };

		bgfx::TextureHandle gbufferTex[5] = { bgfx::kInvalidHandle, bgfx::kInvalidHandle, bgfx::kInvalidHandle, bgfx::kInvalidHandle, bgfx::kInvalidHandle };
		bgfx::TextureHandle lightBufferTex = { bgfx::kInvalidHandle };

		void onChangeInt(Property* prop, std::string varName, int val, bool isDefine);
		void onChangeFloat(Property* prop, std::string varName, float val, bool isDefine);
		void onChangeBool(Property* prop, std::string varName, bool val, bool isDefine);
		void onChangeVec2(Property* prop, std::string varName, glm::vec2 val, bool isDefine);
		void onChangeVec3(Property* prop, std::string varName, glm::vec3 val, bool isDefine);
		void onChangeVec4(Property* prop, std::string varName, glm::vec4 val, bool isDefine);
		void onChangeColor(Property* prop, std::string varName, Color val, bool isDefine);
		void onChangeSampler2D(TreeNode* prop, TreeNode* from, std::string varName, int val, bool isDefine);
		void onClearSampler2D(Property* prop, std::string varName, int val, bool isDefine);
		void onChangeSamplerCube(TreeNode* prop, TreeNode* from, std::string varName, int val, bool isDefine);
		void onClearSamplerCube(Property* prop, std::string varName, int val, bool isDefine);
		void onChangeShader(Property * prop, std::string value);

		void onUpdatePreview();
		void submitLightUniforms();
		void resetFrameBuffers(float w, float h);
	};
}