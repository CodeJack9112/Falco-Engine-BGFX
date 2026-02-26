#pragma once

#include "API.h"

namespace GX
{
	class API_Transform
	{
	public:
		//Register methods
		static void Register()
		{
			mono_add_internal_call("FalcoEngine.Transform::INTERNAL_get_position", (void*)getPosition);
			mono_add_internal_call("FalcoEngine.Transform::INTERNAL_set_position", (void*)setPosition);
			mono_add_internal_call("FalcoEngine.Transform::INTERNAL_get_scale", (void*)getScale);
			mono_add_internal_call("FalcoEngine.Transform::INTERNAL_set_scale", (void*)setScale);
			mono_add_internal_call("FalcoEngine.Transform::INTERNAL_get_rotation", (void*)getRotation);
			mono_add_internal_call("FalcoEngine.Transform::INTERNAL_set_rotation", (void*)setRotation);
			mono_add_internal_call("FalcoEngine.Transform::INTERNAL_get_local_position", (void*)getLocalPosition);
			mono_add_internal_call("FalcoEngine.Transform::INTERNAL_set_local_position", (void*)setLocalPosition);
			mono_add_internal_call("FalcoEngine.Transform::INTERNAL_get_local_rotation", (void*)getLocalRotation);
			mono_add_internal_call("FalcoEngine.Transform::INTERNAL_set_local_rotation", (void*)setLocalRotation);
			mono_add_internal_call("FalcoEngine.Transform::INTERNAL_get_local_scale", (void*)getLocalScale);
			mono_add_internal_call("FalcoEngine.Transform::INTERNAL_set_local_scale", (void*)setLocalScale);
			mono_add_internal_call("FalcoEngine.Transform::INTERNAL_get_forward", (void*)getForward);
			mono_add_internal_call("FalcoEngine.Transform::INTERNAL_get_up", (void*)getUp);
			mono_add_internal_call("FalcoEngine.Transform::INTERNAL_get_left", (void*)getLeft);
			mono_add_internal_call("FalcoEngine.Transform::INTERNAL_get_right", (void*)getRight);
			mono_add_internal_call("FalcoEngine.Transform::get_childCount", (void*)getChildCount);
			mono_add_internal_call("FalcoEngine.Transform::get_parent", (void*)getParent);
			mono_add_internal_call("FalcoEngine.Transform::set_parent", (void*)setParent);
			mono_add_internal_call("FalcoEngine.Transform::INTERNAL_get_child", (void*)getChild);
			mono_add_internal_call("FalcoEngine.Transform::INTERNAL_find_child", (void*)findChild);
			mono_add_internal_call("FalcoEngine.Transform::INTERNAL_TransformDirection", (void*)transformDirection);
			mono_add_internal_call("FalcoEngine.Transform::INTERNAL_InverseTransformDirection", (void*)inverseTransformDirection);
			mono_add_internal_call("FalcoEngine.Transform::INTERNAL_TransformPoint", (void*)transformPoint);
			mono_add_internal_call("FalcoEngine.Transform::INTERNAL_InverseTransformPoint", (void*)inverseTransformPoint);
		}

	private:
		//Get position
		static void getPosition(MonoObject * this_ptr, API::Vector3 * out_pos);

		//Set position
		static void setPosition(MonoObject * this_ptr, API::Vector3 * ref_pos);

		//Get scale
		static void getScale(MonoObject * this_ptr, API::Vector3 * out_scale);

		//Set scale
		static void setScale(MonoObject * this_ptr, API::Vector3 * ref_scale);

		//Get rotation
		static void getRotation(MonoObject * this_ptr, API::Quaternion * out_rot);

		//Set rotation
		static void setRotation(MonoObject * this_ptr, API::Quaternion * ref_rot);

		//Get local position
		static void getLocalPosition(MonoObject * this_ptr, API::Vector3 * out_pos);

		//Set local position
		static void setLocalPosition(MonoObject * this_ptr, API::Vector3 * ref_pos);

		//Get rotation
		static void getLocalRotation(MonoObject * this_ptr, API::Quaternion * out_rot);

		//Set rotation
		static void setLocalRotation(MonoObject * this_ptr, API::Quaternion * ref_rot);

		//Get local scale
		static void getLocalScale(MonoObject* this_ptr, API::Vector3* out_val);

		//Set local scale
		static void setLocalScale(MonoObject* this_ptr, API::Vector3* ref_val);

		//Get forward vector
		static void getForward(MonoObject* this_ptr, API::Vector3* out_val);
		
		//Get up vector
		static void getUp(MonoObject* this_ptr, API::Vector3* out_val);

		//Get left vector
		static void getLeft(MonoObject* this_ptr, API::Vector3* out_val);

		//Get right vector
		static void getRight(MonoObject* this_ptr, API::Vector3* out_val);

		//Get child count
		static int getChildCount(MonoObject * this_ptr);

		//Get parent
		static MonoObject* getParent(MonoObject * this_ptr);

		//Set parent
		static void setParent(MonoObject * this_ptr, MonoObject * parent);

		//Get child by index
		static MonoObject* getChild(MonoObject* this_ptr, int index);

		//Find child by name
		static MonoObject* findChild(MonoObject* this_ptr, MonoString * name);

		//Transform direction
		static void transformDirection(MonoObject* this_ptr, API::Vector3* ref_direction, API::Vector3* out_ret);

		//Inverse transform direction
		static void inverseTransformDirection(MonoObject* this_ptr, API::Vector3* ref_direction, API::Vector3* out_ret);
		
		//Transform point
		static void transformPoint(MonoObject* this_ptr, API::Vector3* ref_position, API::Vector3* out_ret);
		
		//Inverse transform point
		static void inverseTransformPoint(MonoObject* this_ptr, API::Vector3* ref_position, API::Vector3* out_ret);
	};
}