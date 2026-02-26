#pragma once

#include "Property.h"
#include <string>

namespace GX
{
	class PropertyEditor;
	class GameObject;
	class Texture;

	class PropGameObject : public Property
	{
	public:
		PropGameObject(PropertyEditor* ed, std::string name, GameObject * val = nullptr);
		~PropGameObject();

		virtual void update(bool opened) override;
		void setValue(GameObject* val);
		GameObject* getValue() { return value; }
		
		Texture* getImage() { return image; }
		void setImage(Texture* val) { image = val; }

		void setOnChangeCallback(std::function<void(Property * prop, GameObject * val)> callback) { onChangeCallback = callback; }

	private:
		GameObject* value = nullptr;
		std::string guid1 = "";
		Texture* image = nullptr;

		std::function<void(Property * prop, GameObject * val)> onChangeCallback = nullptr;
	};
}