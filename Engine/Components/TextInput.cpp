#include "TextInput.h"

#include <codecvt>
#include <algorithm>

#include <imgui.h>

#include "../Core/Engine.h"
#include "../Core/APIManager.h"
#include "../Core/InputManager.h"
#include "../Core/Time.h"

#include "../Assets/Texture.h"
#include "../Assets/Font.h"

#include "../Components/Canvas.h"
#include "../Components/Text.h"

#include <boost/algorithm/string.hpp>
#include <SDL2/SDL_keyboard.h>

namespace GX
{
	std::string TextInput::COMPONENT_TYPE = "TextInput";

	TextInput::TextInput() : UIElement(APIManager::getSingleton()->text_input_class)
	{
		keyDownEventId = InputManager::getSingleton()->subscribeKeyDownEvent([=](int key) { onKeyDown(key); });
		keyUpEventId = InputManager::getSingleton()->subscribeKeyUpEvent([=](int key) { onKeyUp(key); });
	}

	TextInput::~TextInput()
	{
		InputManager::getSingleton()->unsubscribeKeyDownEvent(keyDownEventId);
		InputManager::getSingleton()->unsubscribeKeyUpEvent(keyUpEventId);
	}

	std::string TextInput::getComponentType()
	{
		return COMPONENT_TYPE;
	}

	Component* TextInput::onClone()
	{
		TextInput* newComponent = new TextInput();
		newComponent->enabled = enabled;
		newComponent->anchor = anchor;
		newComponent->color = color;
		newComponent->size = size;
		newComponent->horizontalAlignment = horizontalAlignment;
		newComponent->verticalAlignment = verticalAlignment;
		newComponent->colHover = colHover;
		newComponent->colActive = colActive;
		newComponent->colDisabled = colDisabled;
		newComponent->colText = colText;
		newComponent->colPlaceholder = colPlaceholder;
		newComponent->texNormal = texNormal;
		newComponent->texHover = texHover;
		newComponent->texActive = texActive;
		newComponent->texDisabled = texDisabled;
		newComponent->text = text;
		newComponent->placeholder = placeholder;
		newComponent->str32text = str32text;
		newComponent->str32placeholder = str32placeholder;
		newComponent->imageType = imageType;
		newComponent->raycastTarget = raycastTarget;
		newComponent->multiline = multiline;
		newComponent->fontSize = fontSize;
		newComponent->font = font;
		newComponent->multiline = multiline;
		newComponent->fontResolution = fontResolution;
		newComponent->fontSize = fontSize;
		newComponent->lineSpacing = lineSpacing;
		newComponent->padding = padding;
		newComponent->wordWrap = wordWrap;
		newComponent->horizontalTextAlignment = horizontalTextAlignment;
		newComponent->verticalTextAlignment = verticalTextAlignment;
		newComponent->interactable = interactable;

		return newComponent;
	}

	void TextInput::onRender(ImDrawList* drawList)
	{
		UIElement::onRender(drawList);

		glm::vec4 rect = getRect();

		float zoom = 1.0f;

		if (canvas != nullptr)
			zoom = canvas->getZoom();

		render(drawList,
			rect,
			texNormal,
			texHover,
			texActive,
			texDisabled,
			color,
			colHover,
			colActive,
			colDisabled,
			colText,
			colPlaceholder,
			imageType,
			str32text,
			str32placeholder,
			font,
			fontSize,
			fontResolution,
			lineSpacing,
			padding,
			wordWrap,
			horizontalTextAlignment,
			verticalTextAlignment,
			interactable,
			multiline,
			zoom,
			caretPos,
			charPositions,
			pressedKey,
			props);
	}

	void TextInput::render(ImDrawList* drawList,
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
			Properties& props)
	{
		ImU32 elemColor = ImGui::ColorConvertFloat4ToU32(ImVec4(color.r(), color.g(), color.b(), color.a()));
		Texture* currentTexture = nullptr;

		if (!interactable)
		{
			currentTexture = texDisabled;
			elemColor = ImGui::ColorConvertFloat4ToU32(ImVec4(colDisabled.r(), colDisabled.g(), colDisabled.b(), colDisabled.a()));
		}
		else
		{
			if (props.currentState == State::Normal)
			{
				currentTexture = texNormal;
				elemColor = ImGui::ColorConvertFloat4ToU32(ImVec4(color.r(), color.g(), color.b(), color.a()));
			}
			if (props.currentState == State::Hover)
			{
				currentTexture = texHover;
				elemColor = ImGui::ColorConvertFloat4ToU32(ImVec4(colHover.r(), colHover.g(), colHover.b(), colHover.a()));
			}
			if (props.currentState == State::Active || props.focused)
			{
				currentTexture = texActive;
				elemColor = ImGui::ColorConvertFloat4ToU32(ImVec4(colActive.r(), colActive.g(), colActive.b(), colActive.a()));
			}
		}

		if (currentTexture == nullptr)
			drawList->AddRectFilled(ImVec2(rect.x, rect.y), ImVec2(rect.z, rect.w), elemColor, 0.0f);
		else
		{
			if (imageType == ImageType::Simple)
				drawList->AddImage((void*)currentTexture->getHandle().idx, ImVec2(rect.x, rect.y), ImVec2(rect.z, rect.w), ImVec2(0, 1), ImVec2(1, 0), elemColor);
			else
			{
				float w = currentTexture->getOriginalWidth() * zoom;
				float h = currentTexture->getOriginalHeight() * zoom;

				Rect border = currentTexture->getBorder() * zoom;

				//Top Left
				ImVec2 tx11 = ImVec2(0.0f, 1.0f);
				ImVec2 tx12 = ImVec2(1.0f / w * border.left, 1.0f - (1.0f / h * border.top));
				drawList->AddImage((void*)currentTexture->getHandle().idx, ImVec2(rect.x, rect.y), ImVec2(rect.x + border.left, rect.y + border.top), tx11, tx12, elemColor);

				//Top right
				ImVec2 tx21 = ImVec2(1.0f - (1.0f / w * border.right), 1.0f);
				ImVec2 tx22 = ImVec2(1.0f, 1.0f - (1.0f / h * border.top));
				drawList->AddImage((void*)currentTexture->getHandle().idx, ImVec2(rect.z - border.right, rect.y), ImVec2(rect.z, rect.y + border.top), tx21, tx22, elemColor);

				//Top
				ImVec2 tx31 = ImVec2(1.0f / w * border.left, 1.0f);
				ImVec2 tx32 = ImVec2(1.0f - (1.0f / w * border.right), 1.0f - (1.0f / h * border.top));
				drawList->AddImage((void*)currentTexture->getHandle().idx, ImVec2(rect.x + border.left, rect.y), ImVec2(rect.z - border.right, rect.y + border.top), tx31, tx32, elemColor);

				//Left
				ImVec2 tx41 = ImVec2(0.0f, 1.0f - (1.0f / h * border.top));
				ImVec2 tx42 = ImVec2(1.0f / w * border.left, 1.0f / h * border.bottom);
				drawList->AddImage((void*)currentTexture->getHandle().idx, ImVec2(rect.x, rect.y + border.top), ImVec2(rect.x + border.left, rect.w - border.bottom), tx41, tx42, elemColor);

				//Center
				ImVec2 tx51 = ImVec2(1.0f / w * border.left, 1.0f - (1.0f / h * border.top));
				ImVec2 tx52 = ImVec2(1.0f - (1.0f / w * border.right), 1.0f / h * border.bottom);
				drawList->AddImage((void*)currentTexture->getHandle().idx, ImVec2(rect.x + border.left, rect.y + border.top), ImVec2(rect.z - border.right, rect.w - border.bottom), tx51, tx52, elemColor);

				//Right
				ImVec2 tx61 = ImVec2(1.0f - (1.0f / w * border.right), 1.0f - (1.0f / h * border.top));
				ImVec2 tx62 = ImVec2(1.0f, 1.0f / h * border.bottom);
				drawList->AddImage((void*)currentTexture->getHandle().idx, ImVec2(rect.z - border.right, rect.y + border.top), ImVec2(rect.z, rect.w - border.bottom), tx61, tx62, elemColor);

				//Bottom Left
				ImVec2 tx71 = ImVec2(0.0f, 1.0f / h * border.bottom);
				ImVec2 tx72 = ImVec2(1.0f / w * border.left, 0.0f);
				drawList->AddImage((void*)currentTexture->getHandle().idx, ImVec2(rect.x, rect.w - border.bottom), ImVec2(rect.x + border.left, rect.w), tx71, tx72, elemColor);

				//Bottom Right
				ImVec2 tx81 = ImVec2(1.0f - (1.0f / w * border.right), 1.0f / h * border.bottom);
				ImVec2 tx82 = ImVec2(1.0f, 0.0f);
				drawList->AddImage((void*)currentTexture->getHandle().idx, ImVec2(rect.z - border.right, rect.w - border.bottom), ImVec2(rect.z, rect.w), tx81, tx82, elemColor);

				//Bottom
				ImVec2 tx91 = ImVec2(1.0f / w * border.left, 1.0f / h * border.bottom);
				ImVec2 tx92 = ImVec2(1.0f - (1.0f / w * border.right), 0.0f);
				drawList->AddImage((void*)currentTexture->getHandle().idx, ImVec2(rect.x + border.left, rect.w - border.bottom), ImVec2(rect.z - border.right, rect.w), tx91, tx92, elemColor);
			}
		}

		float padX = rect.x + padding.x * zoom;
		float padY = rect.y + padding.y * zoom;
		float padZ = rect.z - padding.z * zoom;
		float padW = rect.w - padding.w * zoom;

		if (padZ - padX <= 0)
		{
			padX = rect.x;
			padZ = rect.z;
		}

		if (padW - padY <= 0)
		{
			padY = rect.y;
			padW = rect.w;
		}

		glm::vec4 textRect = glm::vec4(padX, padY, padZ, padW);

		if (!text.empty())
		{
			Text::render(drawList,
				textRect,
				text,
				font,
				fontSize,
				fontResolution,
				lineSpacing,
				static_cast<Text::TextWordWrap>(wordWrap),
				static_cast<Text::TextHorizontalAlignment>(horizontalTextAlignment),
				static_cast<Text::TextVerticalAlignment>(verticalTextAlignment),
				colText,
				zoom);
		}
		else
		{
			if (!placeholder.empty())
			{
				Text::render(drawList,
					textRect,
					placeholder,
					font,
					fontSize,
					fontResolution,
					lineSpacing,
					static_cast<Text::TextWordWrap>(wordWrap),
					static_cast<Text::TextHorizontalAlignment>(horizontalTextAlignment),
					static_cast<Text::TextVerticalAlignment>(verticalTextAlignment),
					colPlaceholder,
					zoom);
			}
		}

		if (!props.isEditor)
		{
			if (props.focused)
			{
				Font* _font = font;

				if (_font == nullptr)
					_font = Font::getDefaultFont();

				if (_font != nullptr)
				{
					charPositions = Text::getCharPositions(textRect,
						text,
						font,
						fontSize,
						fontResolution,
						lineSpacing,
						static_cast<Text::TextWordWrap>(wordWrap),
						static_cast<Text::TextHorizontalAlignment>(horizontalTextAlignment),
						static_cast<Text::TextVerticalAlignment>(verticalTextAlignment),
						zoom);

					if (caretPos > -1 && caretPos < charPositions.size())
					{
						glm::vec2 charPos = charPositions[caretPos];
						FontAtlas* atlas = _font->getFontAtlas(fontResolution);

						float scale = fontSize / fontResolution;
						float caretHeight = (atlas->maxGlyphHeight + lineSpacing) * scale * zoom;

						float alpha = (int)(Time::getTimeSinceGameStartScaled() * 2.0f) % 2 == 0 ? 0.0f : 1.0f;
						
						if (pressedKey != -1)
							alpha = 1.0f;

						uint32_t color = Color::packABGR(Color(1.0f, 1.0f, 1.0f, alpha));

						drawList->PushClipRect(ImVec2(textRect.x - 1.0f, textRect.y), ImVec2(textRect.z + 1.0f, textRect.w), true);
						drawList->AddLine(ImVec2(charPos.x, charPos.y - 2.0f), ImVec2(charPos.x, charPos.y + caretHeight - 2.0f), color);
						drawList->PopClipRect();
					}
				}
			}
		}
	}

	void TextInput::onKeyDown(int key)
	{
		pressedKey = key;
		repeatTimer = -1.0f;
	}

	void TextInput::onKeyUp(int key)
	{
		pressedKey = -1;
	}

	void TextInput::onUpdate(float deltaTime)
	{
		UIElement::onUpdate(deltaTime);

		if (!props.focused)
			return;

		if (pressedKey != -1)
		{
			if (repeatTimer > 0.0f)
			{
				repeatTimer -= deltaTime;
			}
			else
			{
				if (repeatTimer == -1.0f)
					repeatTime = 0.25f;
				else
					repeatTime = 0.05f;

				repeatTimer = repeatTime;

				if (pressedKey == SDL_SCANCODE_LEFT)
				{
					if (text.length() > 0)
					{
						if (caretPos - 1 >= 0)
							caretPos -= 1;
					}
				}
				else if (pressedKey == SDL_SCANCODE_RIGHT)
				{
					if (text.length() > 0)
					{
						if (caretPos + 1 <= text.length())
							caretPos += 1;
					}
				}
				else if (pressedKey == SDL_SCANCODE_BACKSPACE)
				{
					if (text.length() > 0)
					{
						if (caretPos > 0)
						{
							text.erase(text.begin() + caretPos - 1);
							setText(text);

							if (caretPos > 0)
								caretPos -= 1;
						}
					}
				}
				else if (pressedKey == SDL_SCANCODE_DELETE)
				{
					if (text.length() > 0)
					{
						if (caretPos < text.size())
						{
							text.erase(text.begin() + caretPos);
							setText(text);
						}
					}
				}
				else if (pressedKey == SDL_SCANCODE_RETURN)
				{
					if (multiline)
					{
						text.insert(text.begin() + caretPos, '\n');
						setText(text);
						caretPos += 1;
					}
					else
					{
						props.focused = false;
					}
				}
				else if (pressedKey != SDL_SCANCODE_UP && pressedKey != SDL_SCANCODE_DOWN)
				{
					int charCode = SDL_GetKeyFromScancode((SDL_Scancode)pressedKey);

					if (std::isalnum(static_cast<unsigned char>(charCode)))
					{
						if (InputManager::getSingleton()->getKey(SDL_SCANCODE_LSHIFT))
						{
							if (SDL_GetModState() & KMOD_CAPS)
								charCode = std::tolower(charCode);
							else
								charCode = std::toupper(charCode);
						}
						else
						{
							if (SDL_GetModState() & KMOD_CAPS)
								charCode = std::toupper(charCode);
						}

						text.insert(text.begin() + caretPos, (char)charCode);
						setText(text);
						caretPos += 1;
					}
					else
					{
						if (charCode == ' ')
						{
							text.insert(text.begin() + caretPos, ' ');
							setText(text);
							caretPos += 1;
						}
					}
				}
			}
		}
	}

	void TextInput::setText(std::string value)
	{
		text = value;
		std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> cv;
		str32text = cv.from_bytes(text);
	}

	void TextInput::setPlaceholder(std::string value)
	{
		placeholder = value;
		std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> cv;
		str32placeholder = cv.from_bytes(placeholder);
	}

	void TextInput::setFontResolution(int value)
	{
		if (value == 0) return;
		if (value > 128) return;

		Font* _font = font;

		if (_font == nullptr)
			_font = Font::getDefaultFont();

		if (_font != nullptr)
		{
			fontResolution = value;
			_font->clearFontAtlases();
		}
	}

	void TextInput::onMouseEnter(glm::vec2 cursorPos)
	{
		if (!props.isEditor)
		{
			if (interactable)
			{
				if (mouseCursor == -1)
					mouseCursor = static_cast<int>(ImGui::GetMouseCursor());

				ImGui::SetMouseCursor(ImGuiMouseCursor_TextInput);

				UIElement::onMouseEnter(cursorPos);
			}
		}
		else
			UIElement::onMouseEnter(cursorPos);
	}

	void TextInput::onMouseExit(glm::vec2 cursorPos)
	{
		if (!props.isEditor)
		{
			if (interactable)
			{
				if (mouseCursor != -1)
				{
					ImGui::SetMouseCursor(static_cast<ImGuiMouseCursor>(mouseCursor));
					mouseCursor = -1;
				}

				UIElement::onMouseExit(cursorPos);
			}
		}
		else
			UIElement::onMouseExit(cursorPos);
	}

	void TextInput::onMouseMove(glm::vec2 cursorPos)
	{
		if (!props.isEditor)
		{
			if (interactable)
				UIElement::onMouseMove(cursorPos);
		}
		else
			UIElement::onMouseMove(cursorPos);
	}

	void TextInput::onMouseDown(int btn, glm::vec2 cursorPos)
	{
		if (!props.isEditor)
		{
			if (interactable)
			{
				UIElement::onMouseDown(btn, cursorPos);

				if (text.length() > 0)
				{
					int cp = 0;
					for (auto& pos : charPositions)
					{
						if (cursorPos.x > pos.x && cursorPos.y > pos.y)
							caretPos = cp;

						++cp;
					}
				}
			}
		}
		else
			UIElement::onMouseDown(btn, cursorPos);
	}

	void TextInput::onMouseUp(int btn, glm::vec2 cursorPos)
	{
		if (!props.isEditor)
		{
			if (interactable)
				UIElement::onMouseUp(btn, cursorPos);
		}
		else
			UIElement::onMouseUp(btn, cursorPos);
	}
}