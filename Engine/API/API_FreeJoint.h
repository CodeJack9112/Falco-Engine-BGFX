#pragma once

#include "API.h"

namespace GX
{
	class API_FreeJoint
	{
	public:
		static void Register()
		{
			mono_add_internal_call("FalcoEngine.FreeJoint::INTERNAL_get_anchor", (void*)getAnchor);
			mono_add_internal_call("FalcoEngine.FreeJoint::INTERNAL_set_anchor", (void*)setAnchor);

			mono_add_internal_call("FalcoEngine.FreeJoint::INTERNAL_get_connectedAnchor", (void*)getConnectedAnchor);
			mono_add_internal_call("FalcoEngine.FreeJoint::INTERNAL_set_connectedAnchor", (void*)setConnectedAnchor);

			mono_add_internal_call("FalcoEngine.FreeJoint::INTERNAL_get_limitMin", (void*)getLimitMin);
			mono_add_internal_call("FalcoEngine.FreeJoint::INTERNAL_set_limitMin", (void*)setLimitMin);

			mono_add_internal_call("FalcoEngine.FreeJoint::INTERNAL_get_limitMax", (void*)getLimitMax);
			mono_add_internal_call("FalcoEngine.FreeJoint::INTERNAL_set_limitMax", (void*)setLimitMax);

			mono_add_internal_call("FalcoEngine.FreeJoint::get_linkedBodiesCollision", (void*)getLinkedBodiesCollision);
			mono_add_internal_call("FalcoEngine.FreeJoint::set_linkedBodiesCollision", (void*)setLinkedBodiesCollision);

			mono_add_internal_call("FalcoEngine.FreeJoint::get_connectedObject", (void*)getConnectedObject);
			mono_add_internal_call("FalcoEngine.FreeJoint::set_connectedObject", (void*)setConnectedObject);

			mono_add_internal_call("FalcoEngine.FreeJoint::AutoConfigureAnchors", (void*)autoConfigureAnchors);
			mono_add_internal_call("FalcoEngine.FreeJoint::Rebuild", (void*)rebuild);
		}

		static void getAnchor(MonoObject* this_ptr, API::Vector3* out_val);

		static void setAnchor(MonoObject* this_ptr, API::Vector3* ref_val);

		static void getConnectedAnchor(MonoObject* this_ptr, API::Vector3* out_val);

		static void setConnectedAnchor(MonoObject* this_ptr, API::Vector3* ref_val);

		static void getLimitMin(MonoObject* this_ptr, API::Vector3* out_val);

		static void setLimitMin(MonoObject* this_ptr, API::Vector3* ref_val);

		static void getLimitMax(MonoObject* this_ptr, API::Vector3* out_val);

		static void setLimitMax(MonoObject* this_ptr, API::Vector3* ref_val);

		static bool getLinkedBodiesCollision(MonoObject* this_ptr);

		static void setLinkedBodiesCollision(MonoObject* this_ptr, bool value);

		static MonoObject* getConnectedObject(MonoObject* this_ptr);

		static void setConnectedObject(MonoObject* this_ptr, MonoObject* value);

		static void autoConfigureAnchors(MonoObject* this_ptr);

		static void rebuild(MonoObject* this_ptr);
	};
}