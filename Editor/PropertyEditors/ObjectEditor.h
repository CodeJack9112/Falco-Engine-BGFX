#pragma once

#include "PropertyEditor.h"

#include "../Engine/glm/vec2.hpp"
#include "../Engine/glm/vec3.hpp"
#include "../Engine/glm/vec4.hpp"
#include "../Engine/glm/gtc/quaternion.hpp"

namespace GX
{
	class TreeView;
	class TreeNode;
	class Property;
	class GameObject;
	class Component;
	class ComponentEditor;

	class ObjectEditor : public PropertyEditor
	{
	public:
		ObjectEditor();
		~ObjectEditor();

		virtual void init(std::vector<GameObject*> objectList);
		virtual void update();
		virtual void updateEditor();

		std::vector<GameObject*>& getObjects() { return objects; }
		static void resetBufferObjects();

		void updateTransform();

		static Component* getBufferComponent() { return bufferComponent; }
		static void setBufferComponent(Component* value) { bufferComponent = value; }

		static glm::vec3 getBufferPosition() { return bufferPosition; }
		static glm::quat getBufferRotation() { return bufferRotation; }
		static glm::vec3 getBufferScale() { return bufferScale; }

		static glm::vec3 getBufferLocalPosition() { return bufferLocalPosition; }
		static glm::quat getBufferLocalRotation() { return bufferLocalRotation; }
		static glm::vec3 getBufferLocalScale() { return bufferLocalScale; }

	private:
		std::vector<GameObject*> objects;

		static bool bufferTransform;

		static glm::vec3 bufferPosition;
		static glm::quat bufferRotation;
		static glm::vec3 bufferScale;
		
		static glm::vec3 bufferLocalPosition;
		static glm::quat bufferLocalRotation;
		static glm::vec3 bufferLocalScale;

		static Component* bufferComponent;

		ComponentEditor* transformEditor = nullptr;
		ComponentEditor* uiElementEditor = nullptr;

		void onReorder(TreeNode* node, int newIndex);

		std::vector<Component*> getObjectsComponents(Component* base);
		void updateHierarchyNodeColor(GameObject* obj, bool enabled);

		void listComponents();

		void onChangeComponentEnabled(const std::vector<Component*>& comps, bool val);

		void onChangeEnabled(Property* prop, bool val);
		void onChangeName(Property* prop, std::string val);

		void addTransformUndo(int index, bool setPosition, bool setRotation, bool setScale);
		void removeComponent(int index);
		void onComponentPopup(TreeNode* node, int val);

		void onChangeTag(Property* node, int val);
		void onChangeLayer(Property* node, int val);
		void onChangeStatic(Property* node, int val);
	};
}