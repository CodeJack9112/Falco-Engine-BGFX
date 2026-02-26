#pragma once

#include <string>
#include <vector>

#include "../glm/vec3.hpp"

#include "Component.h"

namespace GX
{
	class Spline : public Component
	{
	private:
		std::vector<glm::vec3> points;
		bool closed = false;

		glm::vec3 getSplinePoint(float t, bool bLooped, glm::vec3* pts, int size);
		glm::vec3 getSplineDirection(float t, bool bLooped, glm::vec3* pts, int size);

	public:
		Spline();
		virtual ~Spline();

		static std::string COMPONENT_TYPE;
		virtual std::string getComponentType() { return COMPONENT_TYPE; }
		virtual Component* onClone();

		std::vector<glm::vec3>& getPoints() { return points; }
		bool getClosed() { return closed; }
		void setClosed(bool value) { closed = value; }

		glm::vec3 getSplinePoint(float t);
		glm::vec3 getSplineDirection(float t);
	};
}