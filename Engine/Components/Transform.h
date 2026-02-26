#pragma once

#include <vector>
#include <functional>

#include "../glm/vec3.hpp"
#include "../glm/gtc/quaternion.hpp"

#include "Component.h"

namespace GX
{
	class Transform : public Component
	{
		friend class GameObject;
		friend class Engine;
		friend class Renderer;
		friend class MeshRenderer;
		friend class DecalRenderer;

	private:
		glm::vec3 position = glm::vec3(0, 0, 0);
		glm::highp_quat rotation = glm::identity<glm::highp_quat>();
		glm::vec3 scale = glm::vec3(1, 1, 1);

		glm::vec3 localPosition = glm::vec3(0, 0, 0);
		glm::highp_quat localRotation = glm::identity<glm::highp_quat>();
		glm::vec3 localScale = glm::vec3(1, 1, 1);

		Transform* parent = nullptr;
		std::vector<Transform*> children;

		bool needUpdateTransformMatrix = true;
		bool needUpdateTransformMatrixInverse = true;
		glm::mat4x4 cachedTransform = glm::identity<glm::mat4x4>();
		glm::mat4x4 cachedTransformInverse = glm::identity<glm::mat4x4>();

		std::function<void()> changeCallback = nullptr;

		void updateTransform();
		void updateChildTransform();

	public:
		Transform();
		virtual ~Transform();

		static std::string COMPONENT_TYPE;
		virtual std::string getComponentType();

		Transform* getParent() { return parent; }
		void setParent(Transform* value, bool sortObjects = true);

		static std::vector<Transform*> getPlainChildrenList(Transform* root);
		std::vector<Transform*>& getChildren() { return children; }
		int getChildIndex(Transform* child);
		void setChildIndex(Transform * child, int index);
		Transform* getChild(int index);

		glm::vec3 getPosition();
		glm::vec3 getLocalPosition();
		glm::highp_quat getRotation();
		glm::highp_quat getLocalRotation();
		glm::vec3 getScale();
		glm::vec3 getLocalScale();
		glm::vec3 getForward();
		glm::vec3 getUp();
		glm::vec3 getRight();
		glm::mat4x4 getTransformMatrix();
		glm::mat4x4 getTransformMatrixInverse();
		glm::mat4x4 getLocalTransformMatrix();
		glm::mat3x3 getLocalAxes();

		static glm::mat4x4 makeTransformMatrix(glm::vec3 position, glm::highp_quat rotation, glm::vec3 scale);

		void setPosition(glm::vec3 value, bool updateChildren = true);
		void setLocalPosition(glm::vec3 value, bool updateChildren = true);
		void setRotation(glm::highp_quat value, bool updateChildren = true);
		void setLocalRotation(glm::highp_quat value, bool updateChildren = true);
		void setScale(glm::vec3 value, bool updateChildren = true);
		void setLocalScale(glm::vec3 value, bool updateChildren = true);
		void setTransformMatrix(glm::mat4x4 value);
		void setLocalTransformMatrix(glm::mat4x4 value);

		void yaw(float degree, bool world = true);
		void pitch(float degree, bool world = true);
		void roll(float degree, bool world = true);
		void rotate(glm::vec3 axis, float degree, bool world = true);
		void rotate(glm::highp_quat q, bool world = true);
		void translate(glm::vec3 direction, bool world = true);

		glm::vec3 worldToLocalPosition(glm::vec3 worldPos);
		glm::highp_quat worldToLocalRotation(glm::highp_quat worldRot);
		glm::vec3 localToWorldPosition(glm::vec3 localPos);
		glm::highp_quat localToWorldRotation(glm::highp_quat localRot);

		void iterateChildren(std::function<bool(Transform* child)> cb, bool includeSelf = true);

		void setOnChangeCallback(std::function<void()> cb) { changeCallback = cb; }
	};
}