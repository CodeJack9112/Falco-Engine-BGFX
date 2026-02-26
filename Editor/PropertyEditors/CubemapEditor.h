#pragma once

#include "../Engine/glm/vec2.hpp"
#include "../Engine/glm/vec3.hpp"
#include "../Engine/glm/gtc/quaternion.hpp"

#include "PropertyEditor.h"

namespace GX
{
	class TreeNode;
	class Cubemap;
	class RenderTexture;

	class CubemapEditor : public PropertyEditor
	{
	private:
		Cubemap* cubemap = nullptr;
		RenderTexture* rt = nullptr;

	public:
		CubemapEditor();
		~CubemapEditor();

		virtual void init(Cubemap* cube);
		virtual void update();
		virtual void updateEditor();

	private:
		void onDropTexture(TreeNode* prop, TreeNode* from, int index);
		void onClearTexture(Property* prop, int index);

		//Preview
		static glm::vec2 prevSize;
		bool mousePressed = false;
		glm::vec2 prevMousePos = glm::vec2(0.0f);
		glm::vec3 camPos = glm::vec3(0, 0, 0);
		glm::highp_quat rotX = glm::identity<glm::highp_quat>();
		glm::highp_quat rotY = glm::identity<glm::highp_quat>();
		glm::mat4x4 meshTransform = glm::identity<glm::mat4x4>();

		void onUpdatePreview();
		void resetFrameBuffers(float w, float h);
	};
}