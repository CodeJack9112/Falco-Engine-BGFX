#pragma once

#include "Collider.h"

class btCapsuleShape;

namespace GX
{
	class CapsuleCollider : public Collider
	{
	private:
		float radius = 0.5;
		float height = 2.0;

	public:
		CapsuleCollider();
		virtual ~CapsuleCollider();

		static std::string COMPONENT_TYPE;
		virtual std::string getComponentType();
		virtual Component* onClone();

		float getRadius() { return radius; }
		float getHeight() { return height; }

		void setRadius(float r);
		void setHeight(float h);
	};
}