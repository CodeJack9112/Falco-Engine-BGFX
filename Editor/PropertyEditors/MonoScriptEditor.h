#pragma once

#include <map>

#include "ComponentEditor.h"

#include "../Engine/glm/vec2.hpp"
#include "../Engine/glm/vec3.hpp"
#include "../Engine/glm/vec4.hpp"
#include "../Engine/Renderer/Color.h"

namespace GX
{
	class Property;
	class PropVector3;
	class Asset;
	class GameObject;

	class MonoScriptEditor : public ComponentEditor
	{
	public:
		MonoScriptEditor();
		~MonoScriptEditor();

		virtual void init(std::vector<Component*> comps);

	private:
		void addUndo(void* data);

		std::map<std::string, std::string> componentsIcons;

		void setAssetField(TreeNode* prop, Asset* asset);
		void setGameObjectField(TreeNode* prop, GameObject* gameObject);

		void onChangeInt(Property* prop, int val);
		void onChangeFloat(Property* prop, float val);
		void onChangeBool(Property* prop, bool val);
		void onChangeVector2(Property* prop, glm::vec2 val);
		void onChangeVector3(Property* prop, glm::vec3 val);
		void onChangeVector4(Property* prop, glm::vec4 val);
		void onChangeColor(Property* prop, Color val);
		void onChangeString(Property* prop, std::string val);
		void onChangeStringArray(Property* prop, std::string val, int index);
		void onChangeStringArraySize(Property* prop, int size);
		void onDropGameObject(TreeNode* prop, TreeNode* from);

		//Assets
		void onDropAudioClip(TreeNode* prop, TreeNode* from);
		void onDropVideoClip(TreeNode* prop, TreeNode* from);
		void onDropMaterial(TreeNode* prop, TreeNode* from);
		void onDropPrefab(TreeNode* prop, TreeNode* from);
		void onDropShader(TreeNode* prop, TreeNode* from);
		void onDropTexture(TreeNode* prop, TreeNode* from);
		void onDropAnimationClip(TreeNode* prop, TreeNode* from);
		void onDropCubemap(TreeNode* prop, TreeNode* from);
		void onDropFont(TreeNode* prop, TreeNode* from);
	};
}