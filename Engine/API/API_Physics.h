#pragma once

#include "API.h"

namespace GX
{
	namespace API
	{
		struct RaycastHit
		{
		public:
			bool hasHit;
			MonoObject* rigidBody;
			API::Vector3 hitPoint;
			API::Vector3 worldNormal;
		};
	}

	class API_Physics
	{
	public:
		//Register methods
		static void Register()
		{
			mono_add_internal_call("FalcoEngine.Physics::INTERNAL_raycast", (void*)raycast);
			mono_add_internal_call("FalcoEngine.Physics::INTERNAL_overlapSphere", (void*)overlapSphere);
		}

	private:
		//Raycast
		static void raycast(API::Vector3* ref_from, API::Vector3* ref_to, unsigned long layer, API::RaycastHit* out_hit);

		//Overlap sphere
		static MonoArray* overlapSphere(API::Vector3* center, float radius, unsigned long layer);
	};
}