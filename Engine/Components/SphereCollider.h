#pragma once

#include "Collider.h"

namespace GX
{
	class SphereCollider : public Collider
	{
	private:
		float radius = 1;

	public:
		SphereCollider();
		virtual ~SphereCollider();

		static std::string COMPONENT_TYPE;
		virtual std::string getComponentType();
		virtual Component* onClone();

		float getRadius() { return radius; }
		void setRadius(float value);
	};
}