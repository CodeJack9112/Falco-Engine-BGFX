#pragma once

#include "Property.h"

#include "../Engine/glm/vec4.hpp"

namespace GX
{
	class PropertyEditor;

	class PropVector4 : public Property
	{
	public:
		PropVector4(PropertyEditor* ed, std::string name, glm::vec4 val = glm::vec4(0, 0, 0, 0));
		~PropVector4();

		virtual void update(bool opened) override;
		void setValue(glm::vec4 val);
		glm::vec4 getValue() { return value; }
		void setOnChangeCallback(std::function<void(Property * prop, glm::vec4 val)> callback) { onChangeCallback = callback; }

		bool getIsDraggable() { return isDraggable; }
		void setIsDraggable(bool value) { isDraggable = value; }

	private:
		bool isDraggable = true;
		glm::vec4 value = glm::vec4(0, 0, 0, 0);

		std::function<void(Property * prop, glm::vec4 val)> onChangeCallback = nullptr;
	};
}