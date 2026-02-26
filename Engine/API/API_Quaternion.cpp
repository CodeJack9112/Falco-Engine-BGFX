#include "API_Quaternion.h"

#include "../Core/Engine.h"
#include "../Math/Mathf.h"

namespace GX
{
	void API_Quaternion::euler(API::Vector3 * ref_vec, API::Quaternion * out_rot)
	{
		//Euler e = Euler(ref_vec->y, ref_vec->x, ref_vec->z);
		//Quaternion q = e.toQuaternion();
		glm::highp_quat q = Mathf::toQuaternion(glm::vec3(ref_vec->x, ref_vec->y, ref_vec->z));

		out_rot->x = q.x;
		out_rot->y = q.y;
		out_rot->z = q.z;
		out_rot->w = q.w;
	}

	void API_Quaternion::angleAxis(float angle, API::Vector3 * ref_axis, API::Quaternion * out_rot)
	{
		glm::vec3 axis = glm::vec3(ref_axis->x, ref_axis->y, ref_axis->z);
		glm::highp_quat q = glm::angleAxis(angle * Mathf::fDeg2Rad, axis);

		out_rot->x = q.x;
		out_rot->y = q.y;
		out_rot->z = q.z;
		out_rot->w = q.w;
	}

	void API_Quaternion::lookRotation(API::Vector3* direction, API::Quaternion* out_rot)
	{
		glm::vec3 dir = glm::normalize(glm::vec3(direction->x, direction->y, direction->z));
		glm::vec3 right = glm::normalize(glm::vec3(dir.z, 0, -dir.x));
		glm::vec3 up = glm::cross(dir, right);
		glm::highp_quat quat = glm::quatLookAt(dir, up);
		
		out_rot->x = quat.x;
		out_rot->y = quat.y;
		out_rot->z = quat.z;
		out_rot->w = quat.w;
	}

	void API_Quaternion::eulerAngles(API::Quaternion* quaternion, API::Vector3* ret)
	{
		glm::highp_quat q1 = glm::highp_quat(quaternion->w, quaternion->x, quaternion->y, quaternion->z);
		glm::vec3 euler = Mathf::toEuler(q1);

		ret->x = euler.x;
		ret->y = euler.y;
		ret->z = euler.z;
	}

	void API_Quaternion::slerp(API::Quaternion* q1, API::Quaternion* q2, float t, API::Quaternion* ret)
	{
		glm::highp_quat qq1 = glm::highp_quat(q1->w, q1->x, q1->y, q1->z);
		glm::highp_quat qq2 = glm::highp_quat(q2->w, q2->x, q2->y, q2->z);

		glm::highp_quat rret = glm::slerp(qq1, qq2, t);

		ret->x = rret.x;
		ret->y = rret.y;
		ret->z = rret.z;
		ret->w = rret.w;
	}
}