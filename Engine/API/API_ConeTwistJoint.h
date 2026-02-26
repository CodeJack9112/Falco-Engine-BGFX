#pragma once

#include "API.h"

namespace GX
{
	class API_ConeTwistJoint
	{
	public:
		static void Register()
		{
			mono_add_internal_call("FalcoEngine.ConeTwistJoint::INTERNAL_get_anchor", (void*)getAnchor);
			mono_add_internal_call("FalcoEngine.ConeTwistJoint::INTERNAL_set_anchor", (void*)setAnchor);

			mono_add_internal_call("FalcoEngine.ConeTwistJoint::INTERNAL_get_connectedAnchor", (void*)getConnectedAnchor);
			mono_add_internal_call("FalcoEngine.ConeTwistJoint::INTERNAL_set_connectedAnchor", (void*)setConnectedAnchor);

			mono_add_internal_call("FalcoEngine.ConeTwistJoint::INTERNAL_get_limits", (void*)getLimits);
			mono_add_internal_call("FalcoEngine.ConeTwistJoint::INTERNAL_set_limits", (void*)setLimits);

			mono_add_internal_call("FalcoEngine.ConeTwistJoint::get_linkedBodiesCollision", (void*)getLinkedBodiesCollision);
			mono_add_internal_call("FalcoEngine.ConeTwistJoint::set_linkedBodiesCollision", (void*)setLinkedBodiesCollision);

			mono_add_internal_call("FalcoEngine.ConeTwistJoint::get_connectedObject", (void*)getConnectedObject);
			mono_add_internal_call("FalcoEngine.ConeTwistJoint::set_connectedObject", (void*)setConnectedObject);

			mono_add_internal_call("FalcoEngine.ConeTwistJoint::AutoConfigureAnchors", (void*)autoConfigureAnchors);
			mono_add_internal_call("FalcoEngine.ConeTwistJoint::Rebuild", (void*)rebuild);
		}

		static void getAnchor(MonoObject* this_ptr, API::Vector3* out_val);

		static void setAnchor(MonoObject* this_ptr, API::Vector3* ref_val);

		static void getConnectedAnchor(MonoObject* this_ptr, API::Vector3* out_val);

		static void setConnectedAnchor(MonoObject* this_ptr, API::Vector3* ref_val);

		static void getLimits(MonoObject* this_ptr, API::Vector3* out_val);

		static void setLimits(MonoObject* this_ptr, API::Vector3* ref_val);

		static bool getLinkedBodiesCollision(MonoObject* this_ptr);

		static void setLinkedBodiesCollision(MonoObject* this_ptr, bool value);

		static MonoObject* getConnectedObject(MonoObject* this_ptr);

		static void setConnectedObject(MonoObject* this_ptr, MonoObject* value);

		static void autoConfigureAnchors(MonoObject* this_ptr);

		static void rebuild(MonoObject* this_ptr);
	};
}