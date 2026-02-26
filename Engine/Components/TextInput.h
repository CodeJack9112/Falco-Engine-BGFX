#pragma once

#include "UIElement.h"

struct ImDrawList;

namespace GX
{
	class Texture;
	class Font;

	class TextInput : public UIElement
	{
	public:
		enum class ImageType
		{
			Simple,
			Sliced
		};

		enum class TextHorizontalAlignment
		{
			Left,
			Center,
			Right
		};

		enum class TextVerticalAlignment
		{
			Top,
			Middle,
			Bottom
		};

		enum class TextWordWrap
		{
			None,
			WordWrap,
			BreakWord
		};

	private:
		Texture* texNormal = nullptr;
		Texture* texHover = nullptr;
		Texture* texActive = nullptr;
		Texture* texDisabled = nullptr;

		Color colHover = Color(0.9f, 0.9f, 0.9f, 1.0f);
		Color colActive = Color(0.7f, 0.7f, 0.7f, 1.0f);
		Color colDisabled = Color(0.5f, 0.5f, 0.5f, 1.0f);
		Color colText = Color(0.9f, 0.9f, 0.9f, 1.0f);
		Color colPlaceholder = Color(0.4f, 0.4f, 0.4f, 0.7f);

		ImageType imageType = ImageType::Simple;

		std::string text = "";
		std::string placeholder = "";
		bool multiline = false;
		int fontResolution = 32;
		float fontSize = 14.0f;
		float lineSpacing = 0;
		glm::vec4 padding = glm::vec4(5.0f);

		TextWordWrap wordWrap = TextWordWrap::WordWrap;
		TextHorizontalAlignment horizontalTextAlignment = TextHorizontalAlignment::Left;
		TextVerticalAlignment verticalTextAlignment = TextVerticalAlignment::Middle;

		Font* font = nullptr;

		bool interactable = true;

		std::u32string str32text;
		std::u32string str32placeholder;

		std::vector<glm::vec2> charPositions;
		int caretPos = 0;
		
		std::string keyDownEventId = "";
		std::string keyUpEventId = "";

		int mouseCursor = -1;
		int pressedKey = -1;
		float repeatTimer = 0.0f;
		float repeatTime = 0.5f;

		void onKeyDown(int key);
		void onKeyUp(int key);

	public:
		TextInput();
		virtual ~TextInput();

		static std::string COMPONENT_TYPE;
		virtual std::string getComponentType();
		virtual Component* onClone();
		virtual void onRender(ImDrawList* drawList);
		virtual void onUpdate(float deltaTime);

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
			Color colText,
			Color colPlaceholder,
			ImageType imageType,
			std::u32string text,
			std::u32string placeholder,
			Font* font,
			float fontSize,
			float fontResolution,
			float lineSpacing,
			glm::vec4 padding,
			TextWordWrap wordWrap,
			TextHorizontalAlignment horizontalTextAlignment,
			TextVerticalAlignment verticalTextAlignment,
			bool interactable,
			bool multiline,
			float zoom,
			int& caretPos,
			std::vector<glm::vec2>& charPositions,
			int pressedKey,
			Properties& props
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
		void setColorText(Color value) { colText = value; }
		Color getColorText() { return colText; }
		void setColorPlaceholder(Color value) { colPlaceholder = value; }
		Color getColorPlaceholder() { return colPlaceholder; }

		std::string getText() { return text; }
		void setText(std::string value);

		std::string getPlaceholder() { return placeholder; }
		void setPlaceholder(std::string value);

		ImageType getImageType() { return imageType; }
		void setImageType(ImageType value) { imageType = value; }

		bool getMultiline() { return multiline; }
		void setMultiline(bool value) { multiline = value; }

		float getFontSize() { return fontSize; }
		void setFontSize(float value) { fontSize = value; }

		int getFontResolution() { return fontResolution; }
		void setFontResolution(int value);

		float getLineSpacing() { return lineSpacing; }
		void setLineSpacing(float value) { lineSpacing = value; }

		glm::vec4 getPadding() { return padding; }
		void setPadding(glm::vec4 value) { padding = value; }

		TextWordWrap getWordWrap() { return wordWrap; }
		void setWordWrap(TextWordWrap value) { wordWrap = value; }

		TextHorizontalAlignment getHorizontalTextAlignment() { return horizontalTextAlignment; }
		void setHorizontalTextAlignment(TextHorizontalAlignment value) { horizontalTextAlignment = value; }

		TextVerticalAlignment getVerticalTextAlignment() { return verticalTextAlignment; }
		void setVerticalTextAlignment(TextVerticalAlignment value) { verticalTextAlignment = value; }

		Font* getFont() { return font; }
		void setFont(Font* value) { font = value; }

		bool getInteractable() { return interactable; }
		void setInteractable(bool value) { interactable = value; }
	};
}