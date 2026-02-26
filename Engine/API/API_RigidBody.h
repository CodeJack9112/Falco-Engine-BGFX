#pragma once

#include "API.h"

namespace GX
{
	class API_RigidBody
	{
	public:
		//Register methods
		static void Register()
		{
			mono_add_internal_call("FalcoEngine.Rigidbody::INTERNAL_get_position", (void*)getPosition);
			mono_add_internal_call("FalcoEngine.Rigidbody::INTERNAL_set_position", (void*)setPosition);
			mono_add_internal_call("FalcoEngine.Rigidbody::INTERNAL_get_rotation", (void*)getRotation);
			mono_add_internal_call("FalcoEngine.Rigidbody::INTERNAL_set_rotation", (void*)setRotation);
			mono_add_internal_call("FalcoEngine.Rigidbody::INTERNAL_get_linear_velocity", (void*)getLinearVelocity);
			mono_add_internal_call("FalcoEngine.Rigidbody::INTERNAL_set_linear_velocity", (void*)setLinearVelocity);
			mono_add_internal_call("FalcoEngine.Rigidbody::INTERNAL_get_angular_velocity", (void*)getAngularVelocity);
			mono_add_internal_call("FalcoEngine.Rigidbody::INTERNAL_set_angular_velocity", (void*)setAngularVelocity);
			mono_add_internal_call("FalcoEngine.Rigidbody::INTERNAL_get_is_kinematic", (void*)getIsKinematic);
			mono_add_internal_call("FalcoEngine.Rigidbody::INTERNAL_set_is_kinematic", (void*)setIsKinematic);
			mono_add_internal_call("FalcoEngine.Rigidbody::get_isStatic", (void*)getIsStatic);
			mono_add_internal_call("FalcoEngine.Rigidbody::set_isStatic", (void*)setIsStatic);
			mono_add_internal_call("FalcoEngine.Rigidbody::get_isTrigger", (void*)getIsTrigger);
			mono_add_internal_call("FalcoEngine.Rigidbody::INTERNAL_get_mass", (void*)getMass);
			mono_add_internal_call("FalcoEngine.Rigidbody::INTERNAL_set_mass", (void*)setMass);
			mono_add_internal_call("FalcoEngine.Rigidbody::INTERNAL_add_force", (void*)addForce);
			mono_add_internal_call("FalcoEngine.Rigidbody::INTERNAL_add_torque", (void*)addTorque);
			mono_add_internal_call("FalcoEngine.Rigidbody::SetFreezePosition", (void*)setFreezePosition);
			mono_add_internal_call("FalcoEngine.Rigidbody::SetFreezeRotation", (void*)setFreezeRotation);
			mono_add_internal_call("FalcoEngine.Rigidbody::get_friction", (void*)getFriction);
			mono_add_internal_call("FalcoEngine.Rigidbody::set_friction", (void*)setFriction);
			mono_add_internal_call("FalcoEngine.Rigidbody::get_bounciness", (void*)getBounciness);
			mono_add_internal_call("FalcoEngine.Rigidbody::set_bounciness", (void*)setBounciness);
			mono_add_internal_call("FalcoEngine.Rigidbody::get_linearDamping", (void*)getLinearDamping);
			mono_add_internal_call("FalcoEngine.Rigidbody::set_linearDamping", (void*)setLinearDamping);
			mono_add_internal_call("FalcoEngine.Rigidbody::get_angularDamping", (void*)getAngularDamping);
			mono_add_internal_call("FalcoEngine.Rigidbody::set_angularDamping", (void*)setAngularDamping);
		}

	private:
		//Get position
		static void getPosition(MonoObject * this_ptr, API::Vector3 * out_pos);

		//Set position
		static void setPosition(MonoObject * this_ptr, API::Vector3 * ref_pos);

		//Get rotation
		static void getRotation(MonoObject * this_ptr, API::Quaternion * out_rot);

		//Set rotation
		static void setRotation(MonoObject * this_ptr, API::Quaternion * ref_rot);

		//Get linear velocity
		static void getLinearVelocity(MonoObject * this_ptr, API::Vector3 * out_vel);

		//Set linear velocity
		static void setLinearVelocity(MonoObject * this_ptr, API::Vector3 * ref_vel);

		//Get angular velocity
		static void getAngularVelocity(MonoObject * this_ptr, API::Vector3 * out_vel);

		//Set angular velocity
		static void setAngularVelocity(MonoObject * this_ptr, API::Vector3 * ref_vel);

		//Add force
		static void addForce(MonoObject * this_ptr, API::Vector3 * ref_force, API::Vector3 * ref_pos);

		//Add force
		static void addTorque(MonoObject * this_ptr, API::Vector3 * ref_torque);

		//Get is kinematic
		static bool getIsKinematic(MonoObject * this_ptr);

		//Set is kinematic
		static void setIsKinematic(MonoObject * this_ptr, bool value);

		//Get is static
		static bool getIsStatic(MonoObject* this_ptr);

		//Set is static
		static void setIsStatic(MonoObject* this_ptr, bool value);

		//Get is trigger
		static bool getIsTrigger(MonoObject* this_ptr);

		//Get mass
		static float getMass(MonoObject* this_ptr);

		//Set mass
		static void setMass(MonoObject* this_ptr, float value);

		//Set freeze position
		static void setFreezePosition(MonoObject* this_ptr, bool freezeX, bool freezeY, bool freezeZ);

		//Set freeze rotation
		static void setFreezeRotation(MonoObject* this_ptr, bool freezeX, bool freezeY, bool freezeZ);
		
		//Get friction
		static float getFriction(MonoObject* this_ptr);

		//Set friction
		static void setFriction(MonoObject* this_ptr, float value);

		//Get bounciness
		static float getBounciness(MonoObject* this_ptr);

		//Set bounciness
		static void setBounciness(MonoObject* this_ptr, float value);

		//Get linear damping
		static float getLinearDamping(MonoObject* this_ptr);

		//Set linear damping
		static void setLinearDamping(MonoObject* this_ptr, float value);

		//Get angular damping
		static float getAngularDamping(MonoObject* this_ptr);

		//Set angular damping
		static void setAngularDamping(MonoObject* this_ptr, float value);
	};
}