#include "Transform.h"

#include "glm/gtx/matrix_decompose.hpp"

#include "../Core/Engine.h"
#include "../Core/GameObject.h"
#include "../Core/APIManager.h"
#include "../Math/Mathf.h"
#include "../Classes/VectorUtils.h"

namespace GX
{
	std::string Transform::COMPONENT_TYPE = "Transform";

	Transform::Transform() : Component(APIManager::getSingleton()->transform_class)
	{
		
	}

	Transform::~Transform()
	{
		if (parent != nullptr)
		{
			auto it = std::find(parent->children.begin(), parent->children.end(), this);
			if (it != parent->children.end())
				parent->children.erase(it);
		}

		parent = nullptr;

		children.clear();
	}

	std::string Transform::getComponentType()
	{
		return COMPONENT_TYPE;
	}

	std::vector<Transform*> Transform::getPlainChildrenList(Transform* root)
	{
		std::vector<Transform*> lst;

		std::vector<Transform*> nstack;
		std::vector<Transform*>& init = root->getChildren();
		for (auto it = init.begin(); it != init.end(); ++it)
			nstack.push_back(*it);

		while (nstack.size() > 0)
		{
			Transform* child = *nstack.begin();
			nstack.erase(nstack.begin());

			//
			lst.push_back(child);
			//

			int j = 0;
			std::vector<Transform*>& children = child->getChildren();
			for (auto it = children.begin(); it != children.end(); ++it, ++j)
			{
				Transform* ch = *it;
				nstack.insert(nstack.begin() + j, ch);
			}
		}

		return lst;
	}

	void Transform::setParent(Transform* value, bool sortObjects)
	{
		if (parent == value)
			return;

		if (parent != nullptr)
		{
			auto it = std::find(parent->children.begin(), parent->children.end(), this);
			if (it != parent->children.end())
				parent->children.erase(it);
		}

		Transform* prevParent = parent;
		parent = value;

		if (gameObject != nullptr)
		{
			if (parent != nullptr)
			{
				std::vector<Transform*>& objects = Engine::getSingleton()->getRootTransforms();
				auto p = std::find(objects.begin(), objects.end(), this);
				if (p != objects.end())
					objects.erase(p);

				parent->children.push_back(this);
			}
			else
			{
				std::vector<Transform*>& objects = Engine::getSingleton()->getRootTransforms();
				auto p = std::find(objects.begin(), objects.end(), this);
				if (p == objects.end())
					objects.push_back(this);
			}
		}

		setPosition(position, false);
		setRotation(rotation, false);
		setScale(scale, false);

		updateChildTransform();

		Engine::getSingleton()->markGameObjectsOutdated();

		//Change parent callback
		std::vector<std::pair<Transform*, Transform*>> nstack; //Child, prevParent
		nstack.push_back(std::make_pair(this, prevParent));

		while (nstack.size() > 0)
		{
			Transform* child = nstack.begin()->first;
			prevParent = nstack.begin()->second;
			nstack.erase(nstack.begin());

			//
			if (child->getGameObject() != nullptr)
			{
				std::vector<Component*>& comps = child->getGameObject()->getComponents();
				for (auto it = comps.begin(); it != comps.end(); ++it)
					(*it)->onChangeParent(prevParent);
			}
			//

			int j = 0;
			for (auto it = child->children.begin(); it != child->children.end(); ++it, ++j)
			{
				Transform* ch = *it;
				nstack.insert(nstack.begin() + j, std::make_pair(ch, child));
			}
		}

		//Check active
		if (gameObject != nullptr)
		{
			if (parent != nullptr)
			{
				if (parent->getGameObject()->getActive() && gameObject->enabled)
					gameObject->active = true;
				else
					gameObject->active = false;
			}
			else
			{
				gameObject->active = gameObject->enabled;
			}
		}

		std::vector<Transform*> nstack2;
		for (auto it = children.begin(); it != children.end(); ++it)
			nstack2.push_back(*it);

		while (nstack2.size() > 0)
		{
			Transform* child = *nstack2.begin();
			nstack2.erase(nstack2.begin());

			//Update state
			if (child->getGameObject() != nullptr && child->getGameObject()->enabled)
			{
				child->getGameObject()->active = gameObject->active;

				//

				int j = 0;
				for (auto it = child->children.begin(); it != child->children.end(); ++it, ++j)
				{
					Transform* ch = *it;
					nstack2.insert(nstack2.begin() + j, ch);
				}
			}
		}
	}

	int Transform::getChildIndex(Transform* child)
	{
		auto it = std::find(children.begin(), children.end(), child);
		if (it != children.end())
			return std::distance(children.begin(), it);

		return 0;
	}

	void Transform::setChildIndex(Transform* child, int index)
	{
		assert(index < children.size() && "Index is out of bounds!");

		auto it = std::find(children.begin(), children.end(), child);

		if (it != children.end())
		{
			int ii1 = std::distance(children.begin(), it);
			if (ii1 != index)
				VectorUtils::move(children, ii1, index);
		}

		Engine::getSingleton()->markGameObjectsOutdated();
	}

	Transform* Transform::getChild(int index)
	{
		if (index < children.size())
			return children[index];

		return nullptr;
	}

	glm::vec3 Transform::getPosition()
	{
		return position;
	}

	glm::vec3 Transform::getLocalPosition()
	{
		return localPosition;
	}

	glm::highp_quat Transform::getRotation()
	{
		return rotation;
	}

	glm::highp_quat Transform::getLocalRotation()
	{
		return localRotation;
	}

	glm::vec3 Transform::getScale()
	{
		return scale;
	}

	glm::vec3 Transform::getLocalScale()
	{
		return localScale;
	}

	void Transform::setPosition(glm::vec3 value, bool updateChildren)
	{
		position = value;

		if (parent == nullptr)
			localPosition = position;
		else
			localPosition = glm::inverse(parent->rotation) * (position - parent->position) / parent->scale;

		needUpdateTransformMatrix = true;
		needUpdateTransformMatrixInverse = true;

		if (changeCallback != nullptr)
			changeCallback();

		if (updateChildren)
			updateChildTransform();
	}

	void Transform::setLocalPosition(glm::vec3 value, bool updateChildren)
	{
		localPosition = value;

		needUpdateTransformMatrix = true;
		needUpdateTransformMatrixInverse = true;

		updateTransform();

		if (updateChildren)
			updateChildTransform();
	}

	void Transform::setRotation(glm::highp_quat value, bool updateChildren)
	{
		rotation = value;

		if (parent == nullptr)
			localRotation = rotation;
		else
			localRotation = glm::inverse(parent->rotation) * rotation;

		needUpdateTransformMatrix = true;
		needUpdateTransformMatrixInverse = true;

		if (changeCallback != nullptr)
			changeCallback();

		if (updateChildren)
			updateChildTransform();
	}

	void Transform::setLocalRotation(glm::highp_quat value, bool updateChildren)
	{
		localRotation = value;

		needUpdateTransformMatrix = true;
		needUpdateTransformMatrixInverse = true;

		updateTransform();

		if (updateChildren)
			updateChildTransform();
	}

	void Transform::setScale(glm::vec3 value, bool updateChildren)
	{
		scale = value;

		if (scale.x == 0) scale.x = 0.0001f;
		if (scale.y == 0) scale.y = 0.0001f;
		if (scale.z == 0) scale.z = 0.0001f;

		if (parent == nullptr)
			localScale = scale;
		else
			localScale = scale / parent->getScale();

		needUpdateTransformMatrix = true;
		needUpdateTransformMatrixInverse = true;

		if (changeCallback != nullptr)
			changeCallback();

		if (updateChildren)
			updateChildTransform();
	}

	void Transform::setLocalScale(glm::vec3 value, bool updateChildren)
	{
		localScale = value;

		if (localScale.x == 0) localScale.x = 0.0001f;
		if (localScale.y == 0) localScale.y = 0.0001f;
		if (localScale.z == 0) localScale.z = 0.0001f;

		needUpdateTransformMatrix = true;
		needUpdateTransformMatrixInverse = true;

		updateTransform();

		if (updateChildren)
			updateChildTransform();
	}

	glm::vec3 Transform::getForward()
	{
		return glm::normalize(rotation * glm::vec3(0, 0, 1));
	}

	glm::vec3 Transform::getUp()
	{
		return glm::normalize(rotation * glm::vec3(0, 1, 0));
	}

	glm::vec3 Transform::getRight()
	{
		return glm::normalize(rotation * glm::vec3(1, 0, 0));
	}

	glm::mat3x3 Transform::getLocalAxes()
	{
		glm::vec3 axisX = glm::vec3(1, 0, 0);
		glm::vec3 axisY = glm::vec3(0, 1, 0);
		glm::vec3 axisZ = glm::vec3(0, 0, 1);

		axisX = getLocalRotation() * axisX;
		axisY = getLocalRotation() * axisY;
		axisZ = getLocalRotation() * axisZ;

		return glm::mat3x3(axisX.x, axisY.x, axisZ.x,
			axisX.y, axisY.y, axisZ.y,
			axisX.z, axisY.z, axisZ.z);
	}

	glm::mat4x4 Transform::getTransformMatrix()
	{
		if (needUpdateTransformMatrix)
		{
			needUpdateTransformMatrix = false;
			needUpdateTransformMatrixInverse = true;

			glm::mat4x4 trans = glm::identity<glm::mat4x4>();
			glm::mat4x4 rotMat = glm::mat4_cast(rotation);

			glm::vec3 pos = glm::inverse(rotMat) * (glm::vec4(position, 1.0f));

			trans = glm::translate(trans, pos);
			trans = rotMat * trans;
			trans = glm::scale(trans, scale);

			cachedTransform = trans;

			return cachedTransform;
		}
		else
			return cachedTransform;
	}

	glm::mat4x4 Transform::getTransformMatrixInverse()
	{
		if (needUpdateTransformMatrixInverse)
		{
			needUpdateTransformMatrixInverse = false;
			cachedTransformInverse = glm::inverse(getTransformMatrix());
			return cachedTransformInverse;
		}
		else
			return cachedTransformInverse;
	}

	glm::mat4x4 Transform::getLocalTransformMatrix()
	{
		if (parent != nullptr)
			return glm::inverse(parent->getTransformMatrix()) * getTransformMatrix();
		else
			return getTransformMatrix();
	}

	void Transform::setTransformMatrix(glm::mat4x4 value)
	{
		glm::vec3 pos = glm::vec3(0, 0, 0);
		glm::vec3 scl = glm::vec3(1, 1, 1);
		glm::highp_quat rot = glm::identity<glm::highp_quat>();

		glm::vec3 scew = glm::vec3(0, 0, 0);
		glm::vec4 persp = glm::vec4(0, 0, 0, 0);

		glm::decompose(value, scl, rot, pos, scew, persp);

		setPosition(pos, false);
		setRotation(rot, false);
		setScale(scl, false);

		needUpdateTransformMatrix = true;
		needUpdateTransformMatrixInverse = true;

		updateChildTransform();
	}

	void Transform::setLocalTransformMatrix(glm::mat4x4 value)
	{
		if (parent != nullptr)
			setTransformMatrix(parent->getTransformMatrix() * value);
		else
			setTransformMatrix(value);
	}

	void Transform::yaw(float degree, bool world)
	{
		rotate(glm::vec3(0, 1, 0), degree, world);
	}

	void Transform::pitch(float degree, bool world)
	{
		rotate(glm::vec3(1, 0, 0), degree, world);
	}

	void Transform::roll(float degree, bool world)
	{
		rotate(glm::vec3(0, 0, 1), degree, world);
	}

	void Transform::rotate(glm::vec3 axis, float degree, bool world)
	{
		glm::highp_quat q = glm::angleAxis(degree * Mathf::fDeg2Rad, axis);
		rotate(q, world);
	}

	void Transform::rotate(glm::highp_quat q, bool world)
	{
		if (world)
		{
			setLocalRotation(getLocalRotation() * glm::inverse(getRotation()) * q * getRotation());
		}
		else
		{
			setLocalRotation(getLocalRotation() * q);
		}
	}

	void Transform::translate(glm::vec3 direction, bool world)
	{
		if (world)
		{
			setPosition(getPosition() + getRotation() * direction);
		}
		else
		{
			if (parent != nullptr)
				setLocalPosition(getLocalPosition() + direction);
			else
				setPosition(getPosition() + getRotation() * direction);
		}
	}

	void Transform::updateTransform()
	{
		if (parent != nullptr)
		{
			// Update orientation
			glm::highp_quat parentOrientation = parent->getRotation();

			// Combine orientation with that of parent
			rotation = parentOrientation * localRotation;

			// Update scale
			glm::vec3 parentScale = parent->getScale();

			// Scale own position by parent scale, NB just combine
			// as equivalent axes, no shearing
			scale = parentScale * localScale;

			// Change position vector based on parent's orientation & scale
			position = parentOrientation * (parentScale * localPosition);

			// Add altered position vector to parents
			position += parent->getPosition();
		}
		else
		{
			// Root node, no parent
			position = localPosition;
			rotation = localRotation;
			scale = localScale;
		}

		needUpdateTransformMatrix = true;
		needUpdateTransformMatrixInverse = true;

		if (changeCallback != nullptr)
			changeCallback();
	}

	void Transform::updateChildTransform()
	{
		//updateTransform();

		//Update children
		std::vector<Transform*> nstack;
		for (auto it = children.begin(); it != children.end(); ++it)
			nstack.push_back(*it);

		while (nstack.size() > 0)
		{
			Transform* child = *nstack.begin();
			nstack.erase(nstack.begin());

			//Update position
			child->updateTransform();
			//

			int j = 0;
			for (auto it = child->children.begin(); it != child->children.end(); ++it, ++j)
			{
				Transform* ch = *it;
				nstack.insert(nstack.begin() + j, ch);
			}
		}
	}

	glm::vec3 Transform::worldToLocalPosition(glm::vec3 worldPos)
	{
		return glm::inverse(rotation) * (worldPos - position) / scale;
	}

	glm::highp_quat Transform::worldToLocalRotation(glm::highp_quat worldRot)
	{
		return glm::inverse(rotation) * worldRot;
	}

	glm::vec3 Transform::localToWorldPosition(glm::vec3 localPos)
	{
		return getTransformMatrix() * glm::vec4(localPos, 1.0f);
	}

	glm::highp_quat Transform::localToWorldRotation(glm::highp_quat localRot)
	{
		return rotation * localRot;
	}

	glm::mat4x4 Transform::makeTransformMatrix(glm::vec3 position, glm::highp_quat rotation, glm::vec3 scale)
	{
		glm::mat4x4 trans = glm::identity<glm::mat4x4>();
		glm::mat4x4 rotMat = glm::mat4_cast(rotation);

		glm::vec3 pos = glm::inverse(rotMat) * (glm::vec4(position, 1.0f));

		trans = glm::translate(trans, pos);
		trans = rotMat * trans;
		trans = glm::scale(trans, scale);

		return trans;
	}

	void Transform::iterateChildren(std::function<bool(Transform* child)> cb, bool includeSelf)
	{
		std::vector<Transform*> nstack;

		if (includeSelf)
		{
			nstack.push_back(this);
		}
		else
		{
			for (auto it = children.begin(); it != children.end(); ++it)
				nstack.push_back(*it);
		}

		while (nstack.size() > 0)
		{
			Transform* child = *nstack.begin();
			nstack.erase(nstack.begin());

			//
			if (!cb(child))
				break;
			//

			int j = 0;
			for (auto it = child->getChildren().begin(); it != child->getChildren().end(); ++it, ++j)
			{
				Transform* ch = *it;
				nstack.insert(nstack.begin() + j, ch);
			}
		}
	}
}