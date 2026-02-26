#pragma once

#include "Property.h"
#include "../Engine/glm/vec3.hpp"

namespace GX
{
	class PropertyEditor;

	class PropVector3 : public Property
	{
	public:
		PropVector3(PropertyEditor* ed, std::string name, glm::vec3 val = glm::vec3(0, 0, 0));
		~PropVector3();

		virtual void update(bool opened) override;
		void setValue(glm::vec3 val);
		glm::vec3 getValue() { return value; }
		void setOnChangeCallback(std::function<void(Property * prop, glm::vec3 val)> callback) { onChangeCallback = callback; }

		bool getIsDraggable() { return isDraggable; }
		void setIsDraggable(bool value) { isDraggable = value; }

		bool getShowBadge() { return showBadge; }
		void setShowBadge(bool value) { showBadge = value; }

		void setMaskX(std::string value) { maskX = value; }
		void setMaskY(std::string value) { maskY = value; }
		void setMaskZ(std::string value) { maskZ = value; }

	private:
		bool isDraggable = true;
		glm::vec3 value = glm::vec3(0, 0, 0);
		bool showBadge = false;
		std::string maskX = "";
		std::string maskY = "";
		std::string maskZ = "";

		int getNumberOfZeroes(float value);

		std::function<void(Property * prop, glm::vec3 val)> onChangeCallback = nullptr;
	};
}