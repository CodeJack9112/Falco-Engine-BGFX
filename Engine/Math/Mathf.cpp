#pragma once

#include <ctime>

#include "Mathf.h"

#include "../Components/Transform.h"

namespace GX
{
	const float Mathf::PI = 4.0f * atan(1.0f);
	const float Mathf::fDeg2Rad = PI / 180.0f;
	const float Mathf::fRad2Deg = 180.0f / PI;
	const float Mathf::POS_INFINITY = std::numeric_limits<float>::infinity();
	const float Mathf::NEG_INFINITY = -std::numeric_limits<float>::infinity();

	float Mathf::remap(float value, float min1, float max1, float min2, float max2)
	{
		float start1 = min1;
		float stop1 = max1;
		float start2 = min2;
		float stop2 = max2;

		if (stop1 < start1) stop1 = start1;
		if (stop2 < start2) stop2 = start2;

		float outgoing = start2 + (stop2 - start2) * ((value - start1) / (stop1 - start1));

		return outgoing;
	}

	float Mathf::Clamp01(float value)
	{
		if (value < 0.0f)
		{
			return 0.0f;
		}
		if (value > 1.0f)
		{
			return 1.0f;
		}
		return value;
	}

	float Mathf::clamp(float value, float min1, float max1)
	{
		if (value < min1)
			return min1;

		if (value > max1)
			return max1;

		return value;
	}

	float Mathf::lerp(float a, float b, float t)
	{
		return (1.0 - t) * a + t * b;
	}

	glm::vec3 Mathf::lerp(glm::vec3 a, glm::vec3 b, float t)
	{
		t = Clamp01(t);
		return glm::vec3(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t);
	}

	float Mathf::normalize(float input, float min, float max)
	{
		float average = (min + max) / 2;
		float range = (max - min) / 2;
		float normalized_x = (input - average) / range;
		return normalized_x;
	}

	float Mathf::RandomFloat(float min, float max)
	{
		//assert(max > min);
		//srand(time(NULL));

		float random = ((float)rand()) / (float)RAND_MAX;

		float range = max - min;
		return (random * range) + min;
	}

	int Mathf::RandomInt(int min, int max)
	{
		//srand(time(NULL));

		int random = min + (rand() % static_cast<int>(max - min + 1));
		return random;
	}

	float Mathf::smoothstep(float edge0, float edge1, float x)
	{
		float t = clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
		return t * t * (3.0 - 2.0 * t);
	}

	glm::highp_quat Mathf::toQuaternion(glm::vec3 value)
	{
		return toQuaternion(value.z, value.y, value.x);
	}

	glm::highp_quat Mathf::toQuaternion(double yaw, double pitch, double roll)
	{
		yaw *= fDeg2Rad;
		pitch *= fDeg2Rad;
		roll *= fDeg2Rad;

		// Abbreviations for the various angular functions
		double cy = cos(yaw * 0.5);
		double sy = sin(yaw * 0.5);
		double cp = cos(pitch * 0.5);
		double sp = sin(pitch * 0.5);
		double cr = cos(roll * 0.5);
		double sr = sin(roll * 0.5);

		glm::highp_quat q;
		q.w = cy * cp * cr + sy * sp * sr;
		q.x = cy * cp * sr - sy * sp * cr;
		q.y = sy * cp * sr + cy * sp * cr;
		q.z = sy * cp * cr - cy * sp * sr;

		return q;
	}

	glm::vec3 Mathf::toEuler(glm::highp_quat q)
	{
		glm::vec3 angles;

		float sqw = q.w * q.w;
		float sqx = q.x * q.x;
		float sqy = q.y * q.y;
		float sqz = q.z * q.z;
		float unit = sqx + sqy + sqz + sqw; // if normalised is one, otherwise is correction factor
		float test = q.x * q.w - q.y * q.z;

		if (test > 0.4995f * unit) { // singularity at north pole
			angles.y = 2.0f * atan2(q.y, q.x);
			angles.x = PI / 2.0f;
			angles.z = 0;
			return normalizeAngles(angles * fRad2Deg);
		}
		if (test < -0.4995f * unit) { // singularity at south pole
			angles.y = -2.0f * atan2(q.y, q.x);
			angles.x = -PI / 2.0f;
			angles.z = 0;
			return normalizeAngles(angles * fRad2Deg);
		}

		// roll (x-axis rotation)
		double sinr_cosp = 2.0 * (q.w * q.x + q.y * q.z);
		double cosr_cosp = 1.0 - 2.0 * (q.x * q.x + q.y * q.y);
		angles.x = std::atan2(sinr_cosp, cosr_cosp);

		// pitch (y-axis rotation)
		double sinp = 2.0 * (q.w * q.y - q.z * q.x);
		if (std::abs(sinp) >= 1.0)
			angles.y = std::copysign(PI / 2.0, sinp); // use 90 degrees if out of range
		else
			angles.y = std::asin(sinp);

		// yaw (z-axis rotation)
		double siny_cosp = 2.0 * (q.w * q.z + q.x * q.y);
		double cosy_cosp = 1.0 - 2.0 * (q.y * q.y + q.z * q.z);
		angles.z = std::atan2(siny_cosp, cosy_cosp);

		return normalizeAngles(angles * fRad2Deg);
	}

	glm::vec3 Mathf::normalizeAngles(glm::vec3 angles)
	{
		angles.x = normalizeAngle(angles.x);
		angles.y = normalizeAngle(angles.y);
		angles.z = normalizeAngle(angles.z);
		return angles;
	}

	float Mathf::normalizeAngle(float angle)
	{
		while (angle > 360.0f)
			angle -= 360.0f;
		while (angle < 0.0f)
			angle += 360.0f;
		return angle;
	}

	glm::vec3 Mathf::inverseTransformPoint(Transform* relativeTo, glm::vec3 point)
	{
		glm::vec3 lossyScale = relativeTo->getScale();
		glm::vec3 position = relativeTo->getPosition();
		glm::highp_quat rotation = relativeTo->getRotation();

		return glm::vec3(1.0f / lossyScale.x, 1.0f / lossyScale.y, 1.0f / lossyScale.z) * (glm::inverse(rotation) * ((point - position)));
	}

	glm::vec3 Mathf::transformPoint(Transform* relativeTo, glm::vec3 point)
	{
		glm::vec3 lossyScale = relativeTo->getScale();
		glm::vec3 position = relativeTo->getPosition();
		glm::highp_quat rotation = relativeTo->getRotation();

		return position + rotation * (lossyScale * point);
	}

	glm::vec3 Mathf::transformDirection(Transform* relativeTo, glm::vec3 axis)
	{
		glm::vec3 vec = glm::normalize(relativeTo->getRotation() * axis);

		glm::bvec3 _nanv = glm::isnan(vec);
		glm::bvec3 _infv = glm::isinf(vec);

		if (_nanv.x || _infv.x) vec.x = 0;
		if (_nanv.y || _infv.y) vec.y = 0;
		if (_nanv.z || _infv.z) vec.z = 0;

		return vec;
	}

	glm::vec3 Mathf::inverseTransformDirection(Transform* relativeTo, glm::vec3 axis)
	{
		glm::vec3 vec = glm::normalize(glm::inverse(relativeTo->getRotation()) * axis);

		glm::bvec3 _nanv = glm::isnan(vec);
		glm::bvec3 _infv = glm::isinf(vec);
		
		if (_nanv.x || _infv.x) vec.x = 0;
		if (_nanv.y || _infv.y) vec.y = 0;
		if (_nanv.z || _infv.z) vec.z = 0;

		return vec;
	}

	bool Mathf::isNanOrInfinity(glm::quat q)
	{
		glm::bvec4 _nanv = glm::isnan(q);
		glm::bvec4 _infv = glm::isinf(q);

		if (_nanv.x || _infv.x) return true;
		if (_nanv.y || _infv.y) return true;
		if (_nanv.z || _infv.z) return true;
		if (_nanv.w || _infv.w) return true;

		return false;
	}

	bool Mathf::isNanOrInfinity(glm::vec3 v)
	{
		glm::bvec3 _nanv = glm::isnan(v);
		glm::bvec3 _infv = glm::isinf(v);

		if (_nanv.x || _infv.x) return true;
		if (_nanv.y || _infv.y) return true;
		if (_nanv.z || _infv.z) return true;

		return false;
	}

	bool Mathf::isNanOrInfinity(glm::vec4 v)
	{
		glm::bvec4 _nanv = glm::isnan(v);
		glm::bvec4 _infv = glm::isinf(v);

		if (_nanv.x || _infv.x) return true;
		if (_nanv.y || _infv.y) return true;
		if (_nanv.z || _infv.z) return true;
		if (_nanv.w || _infv.w) return true;

		return false;
	}

	bool Mathf::isNanOrInfinity(glm::mat4x4 mtx)
	{
		if (Mathf::isNanOrInfinity(mtx[0])) return true;
		if (Mathf::isNanOrInfinity(mtx[1])) return true;
		if (Mathf::isNanOrInfinity(mtx[2])) return true;
		if (Mathf::isNanOrInfinity(mtx[3])) return true;

		return false;
	}

	float Mathf::squaredLength(glm::vec3 v)
	{
		return v.x * v.x + v.y * v.y + v.z * v.z;
	}

	void Mathf::makeCeil(glm::vec3& v, const glm::vec3& cmp)
	{
		if (cmp.x > v.x) v.x = cmp.x;
		if (cmp.y > v.y) v.y = cmp.y;
		if (cmp.z > v.z) v.z = cmp.z;
	}

	void Mathf::makeCeil(glm::vec2& v, const glm::vec2& cmp)
	{
		if (cmp.x > v.x) v.x = cmp.x;
		if (cmp.y > v.y) v.y = cmp.y;
	}

	void Mathf::makeFloor(glm::vec3& v, const glm::vec3& cmp)
	{
		if (cmp.x < v.x) v.x = cmp.x;
		if (cmp.y < v.y) v.y = cmp.y;
		if (cmp.z < v.z) v.z = cmp.z;
	}

	void Mathf::makeFloor(glm::vec2& v, const glm::vec2& cmp)
	{
		if (cmp.x < v.x) v.x = cmp.x;
		if (cmp.y < v.y) v.y = cmp.y;
	}

	std::pair<bool, float> Mathf::intersects(const Ray& ray, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, const glm::vec3& normal, bool positiveSide, bool negativeSide)
	{
		//
			// Calculate intersection with plane.
			//
		float t;
		{
			float denom = glm::dot(normal, ray.direction);

			// Check intersect side
			if (denom > +std::numeric_limits<float>::epsilon())
			{
				if (!negativeSide)
					return std::pair<bool, float>(false, (float)0);
			}
			else if (denom < -std::numeric_limits<float>::epsilon())
			{
				if (!positiveSide)
					return std::pair<bool, float>(false, (float)0);
			}
			else
			{
				// Parallel or triangle area is close to zero when
				// the plane normal not normalised.
				return std::pair<bool, float>(false, (float)0);
			}

			t = glm::dot(normal, a - ray.origin) / denom;

			if (t < 0)
			{
				// Intersection is behind origin
				return std::pair<bool, float>(false, (float)0);
			}
		}

		//
		// Calculate the largest area projection plane in X, Y or Z.
		//
		size_t i0, i1;
		{
			float n0 = abs(normal[0]);
			float n1 = abs(normal[1]);
			float n2 = abs(normal[2]);

			i0 = 1; i1 = 2;
			if (n1 > n2)
			{
				if (n1 > n0) i0 = 0;
			}
			else
			{
				if (n2 > n0) i1 = 0;
			}
		}

		//
		// Check the intersection point is inside the triangle.
		//
		{
			float u1 = b[i0] - a[i0];
			float v1 = b[i1] - a[i1];
			float u2 = c[i0] - a[i0];
			float v2 = c[i1] - a[i1];
			float u0 = t * ray.direction[i0] + ray.origin[i0] - a[i0];
			float v0 = t * ray.direction[i1] + ray.origin[i1] - a[i1];

			float alpha = u0 * v2 - u2 * v0;
			float beta = u1 * v0 - u0 * v1;
			float area = u1 * v2 - u2 * v1;

			// epsilon to avoid float precision error
			const float EPSILON = 1e-6f;

			float tolerance = -EPSILON * area;

			if (area > 0)
			{
				if (alpha < tolerance || beta < tolerance || alpha + beta > area - tolerance)
					return std::pair<bool, float>(false, (float)0);
			}
			else
			{
				if (alpha > tolerance || beta > tolerance || alpha + beta < area - tolerance)
					return std::pair<bool, float>(false, (float)0);
			}
		}

		return std::pair<bool, float>(true, (float)t);
	}

	std::pair<bool, float> Mathf::intersects(const Ray& ray, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, bool positiveSide, bool negativeSide)
	{
		glm::vec3 normal = calculateBasicFaceNormalWithoutNormalize(a, b, c);
		return intersects(ray, a, b, c, normal, positiveSide, negativeSide);
	}

	bool Mathf::intersects(glm::vec3 center, float radius, AxisAlignedBox box)
	{
		if (box.isNull()) return false;
		if (box.isInfinite()) return true;

		// Use splitting planes
		glm::vec3& min = box.getMinimum();
		glm::vec3& max = box.getMaximum();

		// Arvo's algorithm
		float s, d = 0;
		for (int i = 0; i < 3; ++i)
		{
			if (center[i] < min[i])
			{
				s = center[i] - min[i];
				d += s * s;
			}
			else if (center[i] > max[i])
			{
				s = center[i] - max[i];
				d += s * s;
			}
		}
		return d <= radius * radius;
	}

	bool Mathf::intersects(AxisAlignedBox a, AxisAlignedBox b)
	{
		// SIMD optimized AABB-AABB test
		// Optimized by removing conditional branches
		bool x = std::abs(a.getCenter()[0] - b.getCenter()[0]) <= (a.getHalfSize()[0] + b.getHalfSize()[0]);
		bool y = std::abs(a.getCenter()[1] - b.getCenter()[1]) <= (a.getHalfSize()[1] + b.getHalfSize()[1]);
		bool z = std::abs(a.getCenter()[2] - b.getCenter()[2]) <= (a.getHalfSize()[2] + b.getHalfSize()[2]);

		return x && y && z;
	}

	glm::vec3 Mathf::calculateBasicFaceNormalWithoutNormalize(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3)
	{
		glm::vec3 normal = glm::cross(v2 - v1, v3 - v1);
		return normal;
	}

	void setColumn(glm::mat3 &mtx, size_t iCol, glm::vec3 vec)
	{
		assert(iCol < 3);
		mtx[0][iCol] = vec.x;
		mtx[1][iCol] = vec.y;
		mtx[2][iCol] = vec.z;
	}

	glm::mat3 Mathf::fromAxes(glm::vec3 right, glm::vec3 up, glm::vec3 dir)
	{
		glm::mat3 mtx = glm::identity<glm::mat3>();

		setColumn(mtx, 0, right);
		setColumn(mtx, 1, up);
		setColumn(mtx, 2, dir);

		return mtx;
	}

	bool Mathf::pointInTriangle(glm::vec2 s, glm::vec2 a, glm::vec2 b, glm::vec2 c)
	{
		int as_x = s.x - a.x;
		int as_y = s.y - a.y;

		bool s_ab = (b.x - a.x) * as_y - (b.y - a.y) * as_x > 0;

		if ((c.x - a.x) * as_y - (c.y - a.y) * as_x > 0 == s_ab) return false;

		if ((c.x - b.x) * (s.y - b.y) - (c.y - b.y) * (s.x - b.x) > 0 != s_ab) return false;

		return true;
	}

	glm::vec2 Mathf::rotateUV(glm::vec2 uv, float rotation)
	{
		float mid = 0.5f;
		return glm::vec2(
			cos(rotation) * (uv.x - mid) + sin(rotation) * (uv.y - mid) + mid,
			cos(rotation) * (uv.y - mid) - sin(rotation) * (uv.x - mid) + mid);
	}

	float Mathf::angleBetweenVectors(glm::vec3 dir1, glm::vec3 dir2)
	{
		float sqrMag1 = dir1.x * dir1.x + dir1.y * dir1.y + dir1.z * dir1.z;
		float sqrMag2 = dir2.x * dir2.x + dir2.y * dir2.y + dir2.z * dir2.z;

		float num = std::sqrt(sqrMag1 * sqrMag2);
		if (num < 1E-15f)
		{
			return 0.0f;
		}

		float f = Mathf::clamp(glm::dot(dir1, dir2) / num, -1.0f, 1.0f);
		return std::acos(f) * 57.29578f;
	}

	glm::vec3 Mathf::rotateAround(glm::vec3 aPointToRotate, glm::vec3 aRotationCenter, glm::mat4x4 aRotation)
	{
		glm::mat4x4 translate =
			glm::translate(
				glm::identity<glm::mat4x4>(),
				aRotationCenter);

		glm::mat4x4 invTranslate = glm::inverse(translate);

		// 1) Translate the object to the center
		// 2) Make the rotation
		// 3) Translate the object back to its original location

		glm::mat4x4 transform = translate * aRotation * invTranslate;

		return transform * glm::vec4(aPointToRotate, 1.0f);
	}
}