#pragma once

#include <string>
#include <vector>
#include <math.h>

#include "../Engine/glm/glm.hpp"

#include "../Engine/Math/AxisAlignedBox.h"

namespace GX
{
	class GameObject;
	class Component;
	class Transform;

	enum class JointType
	{
		ConeTwist,
		Free
	};

	class BoneInfoStruct
	{
	public:
		BoneInfoStruct() = default;

		std::string name;

		Transform* anchor = nullptr;
		Component* joint = nullptr;
		BoneInfoStruct* parent = nullptr;
		int direction = 0;

		glm::vec3 minLimit = glm::vec3(0);
		glm::vec3 maxLimit = glm::vec3(0);
		JointType jointType = JointType::ConeTwist;

		glm::vec3 axis = glm::vec3(0);
		glm::vec3 normalAxis = glm::vec3(0);

		float radiusScale = 0;
		float heightScale = 1.0f;
		std::string colliderType = "";

		std::vector<BoneInfoStruct*> children;
		float density = 0;
		float summedMass = 0;// The mass of this and all children bodies
	};

	class DialogRagdollEditor
	{
	public:
		DialogRagdollEditor();
		~DialogRagdollEditor();

		void show();
		void update();

	private:
		bool visible = false;

		float massValue = 70.0f;
		float strength = 0.0F;

		glm::vec3 right = glm::vec3(1, 0, 0);
		glm::vec3 up = glm::vec3(0, 1, 0);
		glm::vec3 forward = glm::vec3(0, 0, 1);

		glm::vec3 worldRight = glm::vec3(1, 0, 0);
		glm::vec3 worldUp = glm::vec3(0, 1, 0);
		glm::vec3 worldForward = glm::vec3(0, 0, 1);
		bool flipForward = false;

		std::vector<BoneInfoStruct*> bones;
		BoneInfoStruct* rootBone;

		Transform* pelvisObject = nullptr;
		Transform* lhipObject = nullptr;
		Transform* lkneeObject = nullptr;
		Transform* rhipObject = nullptr;
		Transform* rkneeObject = nullptr;
		Transform* larmObject = nullptr;
		Transform* lelbowObject = nullptr;
		Transform* rarmObject = nullptr;
		Transform* relbowObject = nullptr;
		Transform* mspineObject = nullptr;
		Transform* headObject = nullptr;

		void acceptDragDrop(Transform*& sceneNode);
		void createRagdoll();

		std::string checkConsistency();
		void decomposeVector(glm::vec3& normalCompo, glm::vec3& tangentCompo, glm::vec3 outwardDir, glm::vec3 outwardNormal);
		void calculateAxes();
		void prepareBones();
		BoneInfoStruct* findBone(std::string name);
		void addMirroredJoint(std::string name, Transform * leftAnchor, Transform* rightAnchor, std::string parent, glm::vec3 worldTwistAxis, glm::vec3 worldSwingAxis, glm::vec3 minLimit, glm::vec3 maxLimit, JointType jointType, std::string colliderType, float radiusScale, float density);
		void addJoint(std::string name, Transform* anchor, std::string parent, glm::vec3 worldTwistAxis, glm::vec3 worldSwingAxis, glm::vec3 minLimit, glm::vec3 maxLimit, JointType jointType, std::string colliderType, float radiusScale, float density);
		void buildCapsules();
		void cleanup(Transform* root);
		void buildBodies();
		void buildJoints();
		void calculateMassRecurse(BoneInfoStruct* bone);
		void calculateMass();
		static void calculateDirection(glm::vec3 point, int & direction, float & distance);
		static glm::vec3 calculateDirectionAxis(glm::vec3 point);
		static int smallestComponent(glm::vec3 point);
		static int largestComponent(glm::vec3 point);
		static int secondLargestComponent(glm::vec3 point);
		AxisAlignedBox clip(AxisAlignedBox bounds, Transform* relativeTo, Transform* clipTransform, bool below);
		AxisAlignedBox getBreastBounds(Transform* relativeTo);
		void addBreastColliders();
		void addHeadCollider();
	};
}