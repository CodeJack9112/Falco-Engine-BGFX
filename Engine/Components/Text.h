#pragma once

#include "UIElement.h"

struct ImDrawList;

namespace GX
{
	class Font;

	class Text : public UIElement
	{
	public:
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
		std::string text = "";
		float fontSize = 14;
		int fontResolution = 32;
		float lineSpacing = 0;
		TextWordWrap wordWrap = TextWordWrap::WordWrap;
		Font* font = nullptr;
		
		TextHorizontalAlignment horizontalTextAlignment = TextHorizontalAlignment::Left;
		TextVerticalAlignment verticalTextAlignment = TextVerticalAlignment::Middle;

		std::u32string str32;

	public:
		Text();
		virtual ~Text();

		static std::string COMPONENT_TYPE;
		virtual std::string getComponentType();
		virtual Component* onClone();
		virtual void onRender(ImDrawList* drawList);

		static void render(ImDrawList* drawList,
			glm::vec4 rect,
			std::u32string str32,
			Font* font,
			float fontSize,
			float fontResolution,
			float lineSpacing,
			TextWordWrap wordWrap,
			TextHorizontalAlignment horizontalTextAlignment,
			TextVerticalAlignment verticalTextAlignment,
			Color color,
			float zoom
		);

		static std::vector<glm::vec2> getCharPositions(glm::vec4 rect,
			std::u32string text,
			Font* font,
			float fontSize,
			float fontResolution,
			float lineSpacing,
			TextWordWrap wordWrap,
			TextHorizontalAlignment horizontalTextAlignment,
			TextVerticalAlignment verticalTextAlignment,
			float zoom);

		std::string getText() { return text; }
		void setText(std::string value);

		float getFontSize() { return fontSize; }
		void setFontSize(float value);

		int getFontResolution() { return fontResolution; }
		void setFontResolution(int value);

		float getLineSpacing() { return lineSpacing; }
		void setLineSpacing(float value) { lineSpacing = value; }

		TextWordWrap getWordWrap() { return wordWrap; }
		void setWordWrap(TextWordWrap value) { wordWrap = value; }

		Font* getFont() { return font; }
		void setFont(Font* value);

		TextHorizontalAlignment getHorizontalTextAlignment() { return horizontalTextAlignment; }
		void setHorizontalTextAlignment(TextHorizontalAlignment value) { horizontalTextAlignment = value; }

		TextVerticalAlignment getVerticalTextAlignment() { return verticalTextAlignment; }
		void setVerticalTextAlignment(TextVerticalAlignment value) { verticalTextAlignment = value; }
	};
}