#pragma once

#include "UIElement.h"

struct ImDrawList;

namespace GX
{
	class Texture;

	class Button : public UIElement
	{
	public:
		enum class ImageType
		{
			Simple,
			Sliced
		};

	private:
		Texture* texNormal = nullptr;
		Texture* texHover = nullptr;
		Texture* texActive = nullptr;
		Texture* texDisabled = nullptr;

		Color colHover = Color(0.9f, 0.9f, 0.9f, 1.0f);
		Color colActive = Color(0.7f, 0.7f, 0.7f, 1.0f);
		Color colDisabled = Color(0.5f, 0.5f, 0.5f, 1.0f);

		ImageType imageType = ImageType::Simple;

		bool interactable = true;

	public:
		Button();
		virtual ~Button();

		static std::string COMPONENT_TYPE;
		virtual std::string getComponentType();
		virtual Component* onClone();
		virtual void onRender(ImDrawList* drawList);

		static void render(ImDrawList* drawList,
			glm::vec4 rect,
			Texture* texNormal,
			Texture* texHover,
			Texture* texActive,
			Texture* texDisabled,
			Color color,
			Color colHover,
			Color colActive,
			Color colDisabled,
			ImageType imageType,
			bool interactable,
			State currentState,
			float zoom
		);

		virtual void onMouseEnter(glm::vec2 cursorPos);
		virtual void onMouseExit(glm::vec2 cursorPos);
		virtual void onMouseMove(glm::vec2 cursorPos);
		virtual void onMouseDown(int btn, glm::vec2 cursorPos);
		virtual void onMouseUp(int btn, glm::vec2 cursorPos);

		void setTextureNormal(Texture* value) { texNormal = value; }
		Texture* getTextureNormal() { return texNormal; }
		void setTextureHover(Texture* value) { texHover = value; }
		Texture* getTextureHover() { return texHover; }
		void setTextureActive(Texture* value) { texActive = value; }
		Texture* getTextureActive() { return texActive; }
		void setTextureDisabled(Texture* value) { texDisabled = value; }
		Texture* getTextureDisabled() { return texDisabled; }

		void setColorHover(Color value) { colHover = value; }
		Color getColorHover() { return colHover; }
		void setColorActive(Color value) { colActive = value; }
		Color getColorActive() { return colActive; }
		void setColorDisabled(Color value) { colDisabled = value; }
		Color getColorDisabled() { return colDisabled; }

		bool getInteractable() { return interactable; }
		void setInteractable(bool value) { interactable = value; }

		ImageType getImageType() { return imageType; }
		void setImageType(ImageType value) { imageType = value; }
	};
}