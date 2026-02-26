#pragma once

#include "Property.h"
#include <string>

#include "../Engine/glm/mat4x4.hpp"
#include "../Engine/glm/ext/matrix_transform.hpp"

namespace GX
{
	class PropertyEditor;

	class PropMatrix4 : public Property
	{
	public:
		PropMatrix4(PropertyEditor* ed, std::string name, glm::mat4x4 val = glm::identity<glm::mat4x4>());
		~PropMatrix4();

		virtual void update(bool opened) override;
		void setValue(glm::mat4x4 val);
		glm::mat4x4 getValue() { return value; }
		void setOnChangeCallback(std::function<void(Property * prop, glm::mat4x4 val)> callback) { onChangeCallback = callback; }

	private:
		glm::mat4x4 value = glm::identity<glm::mat4x4>();
		std::string guid1 = "";
		std::string guid2 = "";
		std::string guid3 = "";
		std::string guid4 = "";

		std::function<void(Property * prop, glm::mat4x4 val)> onChangeCallback = nullptr;
	};
}