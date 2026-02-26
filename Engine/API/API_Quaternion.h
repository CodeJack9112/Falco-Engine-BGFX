#pragma once

#include "API.h"

namespace GX
{
	class API_Quaternion
	{
	public:
		//Register methods
		static void Register()
		{
			mono_add_internal_call("FalcoEngine.Quaternion::INTERNAL_Euler", (void*)euler);
			mono_add_internal_call("FalcoEngine.Quaternion::INTERNAL_AngleAxis", (void*)angleAxis);
			mono_add_internal_call("FalcoEngine.Quaternion::INTERNAL_LookRotation", (void*)lookRotation);
			mono_add_internal_call("FalcoEngine.Quaternion::INTERNAL_EulerAngles", (void*)eulerAngles);
			mono_add_internal_call("FalcoEngine.Quaternion::INTERNAL_Slerp", (void*)slerp);
		}

	private:
		//Get position
		static void euler(API::Vector3 * ref_vec, API::Quaternion * out_rot);
		static void angleAxis(float angle, API::Vector3 * ref_axis, API::Quaternion * out_rot);
		static void lookRotation(API::Vector3 * direction, API::Quaternion * out_rot);
		static void eulerAngles(API::Quaternion * quaternion, API::Vector3 * ret);
		static void slerp(API::Quaternion * q1, API::Quaternion* q2, float t, API::Quaternion* ret);
	};
}