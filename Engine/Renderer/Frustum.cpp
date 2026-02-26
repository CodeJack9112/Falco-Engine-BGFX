#include "Frustum.h"

#include <complex>

#include "../glm/gtc/type_ptr.hpp"

namespace GX
{
	enum FrustumSide
	{
		RIGHT = 0,
		LEFT = 1,
		BOTTOM = 2,
		TOP = 3,
		BACK = 4,
		FRONT = 5
	};

	enum PlaneData
	{
		A = 0,
		B = 1,
		C = 2,
		D = 3
	};

	void Frustum::calculateFrustum(glm::mat4x4 modelViewMatrix, glm::mat4x4 projectionMatrix)
	{
		glm::mat4x4 clip = projectionMatrix * modelViewMatrix;

		//Right
		m_Frustum[RIGHT][A] = clip[0].w - clip[0].x;
		m_Frustum[RIGHT][B] = clip[1].w - clip[1].x;
		m_Frustum[RIGHT][C] = clip[2].w - clip[2].x;
		m_Frustum[RIGHT][D] = clip[3].w - clip[3].x;

		//Left
		m_Frustum[LEFT][A] = clip[0].w + clip[0].x;
		m_Frustum[LEFT][B] = clip[1].w + clip[1].x;
		m_Frustum[LEFT][C] = clip[2].w + clip[2].x;
		m_Frustum[LEFT][D] = clip[3].w + clip[3].x;

		//Bottom
		m_Frustum[BOTTOM][A] = clip[0].w + clip[0].y;
		m_Frustum[BOTTOM][B] = clip[1].w + clip[1].y;
		m_Frustum[BOTTOM][C] = clip[2].w + clip[2].y;
		m_Frustum[BOTTOM][D] = clip[3].w + clip[3].y;

		//Top
		m_Frustum[TOP][A] = clip[0].w - clip[0].y;
		m_Frustum[TOP][B] = clip[1].w - clip[1].y;
		m_Frustum[TOP][C] = clip[2].w - clip[2].y;
		m_Frustum[TOP][D] = clip[3].w - clip[3].y;

		//Back
		m_Frustum[BACK][A] = clip[0].w - clip[0].z;
		m_Frustum[BACK][B] = clip[1].w - clip[1].z;
		m_Frustum[BACK][C] = clip[2].w - clip[2].z;
		m_Frustum[BACK][D] = clip[3].w - clip[3].z;

		//Front
		m_Frustum[FRONT][A] = clip[0].z + clip[0].z;
		m_Frustum[FRONT][B] = clip[1].z + clip[1].z;
		m_Frustum[FRONT][C] = clip[2].z + clip[2].z;
		m_Frustum[FRONT][D] = clip[3].z + clip[3].z;

		normalize();
	}

	bool Frustum::pointInFrustum(glm::vec3 point)
	{
		bool visible = true;

		for (int plane = 0; plane < 6; ++plane)
		{
			glm::vec3 normal = glm::vec3(m_Frustum[plane][A], m_Frustum[plane][B], m_Frustum[plane][C]);
			float fDistance = glm::dot(normal, point) + m_Frustum[plane][D];
			int side = -1;

			if (fDistance < 0.0) side = 2;
			if (fDistance > 0.0) side = 1;

			if (side == 2)
			{
				visible = true;
				break;
			}
		}

		return visible;
	}

	bool Frustum::sphereInFrustum(glm::vec3 point, float radius)
	{
		for (int i = 0; i < 6; i++)
		{
			if (m_Frustum[i][A] * point.x + m_Frustum[i][B] * point.y + m_Frustum[i][C] * point.z +
				m_Frustum[i][D] <= - radius)
			{
				return false;
			}
		}

		return true;
	}

	bool Frustum::cubeInFrustum(float x, float y, float z, float size)
	{
		for (int i = 0; i < 6; i++)
		{
			if (m_Frustum[i][A] * (x - size) + m_Frustum[i][B] * (y - size) +
				m_Frustum[i][C] * (z - size) + m_Frustum[i][D] > 0)
				continue;
			if (m_Frustum[i][A] * (x + size) + m_Frustum[i][B] * (y - size) +
				m_Frustum[i][C] * (z - size) + m_Frustum[i][D] > 0)
				continue;
			if (m_Frustum[i][A] * (x - size) + m_Frustum[i][B] * (y + size) +
				m_Frustum[i][C] * (z - size) + m_Frustum[i][D] > 0)
				continue;
			if (m_Frustum[i][A] * (x + size) + m_Frustum[i][B] * (y + size) +
				m_Frustum[i][C] * (z - size) + m_Frustum[i][D] > 0)
				continue;
			if (m_Frustum[i][A] * (x - size) + m_Frustum[i][B] * (y - size) +
				m_Frustum[i][C] * (z + size) + m_Frustum[i][D] > 0)
				continue;
			if (m_Frustum[i][A] * (x + size) + m_Frustum[i][B] * (y - size) +
				m_Frustum[i][C] * (z + size) + m_Frustum[i][D] > 0)
				continue;
			if (m_Frustum[i][A] * (x - size) + m_Frustum[i][B] * (y + size) +
				m_Frustum[i][C] * (z + size) + m_Frustum[i][D] > 0)
				continue;
			if (m_Frustum[i][A] * (x + size) + m_Frustum[i][B] * (y + size) +
				m_Frustum[i][C] * (z + size) + m_Frustum[i][D] > 0)
				continue;

			return false;
		}

		return true;
	}

	bool Frustum::aabbInFrustum(AxisAlignedBox aabb)
	{
		if (aabb.isNull()) return false;
		if (aabb.isInfinite()) return true;

		glm::vec3 center = aabb.getCenter();
		glm::vec3 halfSize = aabb.getHalfSize();

		bool visible = true;
		for (int plane = 0; plane < 6; ++plane)
		{
			glm::vec3 normal = glm::vec3(m_Frustum[plane][A], m_Frustum[plane][B], m_Frustum[plane][C]);
			float dist = glm::dot(normal, center) + m_Frustum[plane][D];
			int side = -1;

			if (dist < 0.0) side = 2;
			if (dist > 0.0) side = 1;
			float maxAbsDist = glm::abs(glm::dot(normal, halfSize));

			int side2 = -1;
			if (dist < -maxAbsDist) side2 = 2;
			if (dist > +maxAbsDist) side2 = 1;
			
			if (side2 == 2)
			{
				visible = false;
				break;
			}
		}

		return visible;
	}

	void Frustum::normalize()
	{
		for (int i = 0; i < 6; ++i)
		{
			float magnitude = (float)std::sqrt(m_Frustum[i][A] * m_Frustum[i][A] +
												m_Frustum[i][B] * m_Frustum[i][B] +
												m_Frustum[i][C] * m_Frustum[i][C]);

			m_Frustum[i][A] /= magnitude;
			m_Frustum[i][B] /= magnitude;
			m_Frustum[i][C] /= magnitude;
			m_Frustum[i][D] /= magnitude;
		}
	}
}