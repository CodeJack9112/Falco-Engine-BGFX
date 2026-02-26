#pragma once

#include "Property.h"
#include "../Engine/glm/vec2.hpp"

namespace GX
{
	class PropertyEditor;

	class PropVector2 : public Property
	{
	public:
		enum class ValueType
		{
			Float,
			Integer
		};

		PropVector2(PropertyEditor* ed, std::string name, glm::vec2 val = glm::vec2(0, 0));
		~PropVector2();

		virtual void update(bool opened) override;
		void setValue(glm::vec2 val);
		glm::vec2 getValue() { return value; }
		void setOnChangeCallback(std::function<void(Property * prop, glm::vec2 val)> callback) { onChangeCallback = callback; }

		bool getIsDraggable() { return isDraggable; }
		void setIsDraggable(bool value) { isDraggable = value; }

		ValueType getValueType() { return type; }
		void setValueType(ValueType value) { type = value; }

	private:
		bool isDraggable = true;
		ValueType type = ValueType::Float;
		glm::vec2 value = glm::vec2(0, 0);
		glm::ivec2 valueInt = glm::ivec2(0, 0);

		std::function<void(Property * prop, glm::vec2 val)> onChangeCallback = nullptr;
	};
}