#pragma once

#include "Property.h"
#include <string>

#include "../Engine/glm/mat3x3.hpp"
#include "../Engine/glm/ext/matrix_transform.hpp"

namespace GX
{
	class PropertyEditor;

	class PropMatrix3 : public Property
	{
	public:
		PropMatrix3(PropertyEditor* ed, std::string name, glm::mat3x3 val = glm::identity<glm::mat3x3>());
		~PropMatrix3();

		virtual void update(bool opened) override;
		void setValue(glm::mat3x3 val);
		glm::mat3x3 getValue() { return value; }
		void setOnChangeCallback(std::function<void(Property * prop, glm::mat3x3 val)> callback) { onChangeCallback = callback; }

	private:
		glm::mat3x3 value = glm::identity<glm::mat3x3>();
		std::string guid1 = "";
		std::string guid2 = "";
		std::string guid3 = "";

		std::function<void(Property * prop, glm::mat3x3 val)> onChangeCallback = nullptr;
	};
}