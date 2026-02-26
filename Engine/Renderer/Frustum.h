#pragma once

#include "../glm/mat4x4.hpp"
#include "../glm/vec3.hpp"

#include "../Math/AxisAlignedBox.h"

namespace GX
{
	struct Sphere
	{
		float xPos, yPos, zPos, radius;
	};

	class Frustum
	{
	public:
		void calculateFrustum(glm::mat4x4 modelViewMatrix, glm::mat4x4 projectionMatrix);
		bool pointInFrustum(glm::vec3 point);
		bool sphereInFrustum(glm::vec3 point, float radius);
		bool cubeInFrustum(float x, float y, float z, float size);
		bool aabbInFrustum(AxisAlignedBox aabb);

	private:

		float m_Frustum[6][4];

		void normalize();
	};
}