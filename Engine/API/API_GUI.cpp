#include "API_GUI.h"

#include <imgui.h>
#include <imgui_internal.h>

#include <codecvt>

#include "../Core/Engine.h"
#include "../Core/APIManager.h"
#include "../Classes/StringConverter.h"
#include "../Classes/IO.h"

#include "../Assets/Texture.h"
#include "../Assets/Font.h"

#include "../Components/Text.h"
#include "../Components/Image.h"
#include "../Components/Button.h"

#include "../Core/InputManager.h"

namespace GX
{
	std::vector<API_GUI::ElementInfo> API_GUI::elementStack;

	void API_GUI::guiImage(MonoObject* ref_texture, API::Rect* ref_rect, API::Color* ref_color, int imageType)
	{
		Texture* _texture = nullptr;

		if (ref_texture != nullptr)
			mono_field_get_value(ref_texture, APIManager::getSingleton()->asset_ptr_field, reinterpret_cast<void*>(&_texture));

		glm::vec4 rect = glm::vec4(ref_rect->m_XMin, ref_rect->m_YMin, ref_rect->m_XMin + ref_rect->m_Width, ref_rect->m_YMin + ref_rect->m_Height);
		Color color = Color(ref_color->r, ref_color->g, ref_color->b, ref_color->a);
		Image::ImageType _imageType = static_cast<Image::ImageType>(imageType);

		ImGuiWindow* win = ImGui::GetCurrentWindow();
		Image::render(win->DrawList, rect, _texture, color, _imageType, 1.0f);
	}

	void API_GUI::guiText(MonoObject* ref_text,
		API::Rect* ref_rect,
		MonoObject* ref_font,
		float fontSize,
		float fontResolution,
		float lineSpacing,
		int wordWrap,
		int horizontalTextAlignment,
		int verticalTextAlignment,
		API::Color* ref_color)
	{
		Font* _font = nullptr;

		if (ref_font != nullptr)
			mono_field_get_value(ref_font, APIManager::getSingleton()->asset_ptr_field, reinterpret_cast<void*>(&_font));

		std::string _text = (const char*)mono_string_to_utf8((MonoString*)ref_text);

		glm::vec4 rect = glm::vec4(ref_rect->m_XMin, ref_rect->m_YMin, ref_rect->m_XMin + ref_rect->m_Width, ref_rect->m_YMin + ref_rect->m_Height);
		Color color = Color(ref_color->r, ref_color->g, ref_color->b, ref_color->a);

		std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> cv;
		std::u32string str32 = cv.from_bytes(_text);

		ImGuiWindow* win = ImGui::GetCurrentWindow();
		Text::render(win->DrawList,
			rect,
			str32,
			_font,
			fontSize,
			fontResolution,
			lineSpacing,
			static_cast<Text::TextWordWrap>(wordWrap),
			static_cast<Text::TextHorizontalAlignment>(horizontalTextAlignment),
			static_cast<Text::TextVerticalAlignment>(verticalTextAlignment),
			color,
			1.0f
		);
	}

	bool API_GUI::guiButton(int id,
		API::Rect* ref_rect,
		MonoObject* ref_texNormal,
		MonoObject* ref_texHover,
		MonoObject* ref_texActive,
		MonoObject* ref_texDisabled,
		API::Color* ref_color,
		API::Color* ref_colHover,
		API::Color* ref_colActive,
		API::Color* ref_colDisabled,
		int imageType,
		bool interactable)
	{
		Texture* texNormal = nullptr;
		Texture* texHover = nullptr;
		Texture* texActive = nullptr;
		Texture* texDisabled = nullptr;

		if (ref_texNormal != nullptr)
			mono_field_get_value(ref_texNormal, APIManager::getSingleton()->asset_ptr_field, reinterpret_cast<void*>(&texNormal));

		if (ref_texHover != nullptr)
			mono_field_get_value(ref_texHover, APIManager::getSingleton()->asset_ptr_field, reinterpret_cast<void*>(&texHover));

		if (ref_texActive != nullptr)
			mono_field_get_value(ref_texActive, APIManager::getSingleton()->asset_ptr_field, reinterpret_cast<void*>(&texActive));

		if (ref_texDisabled != nullptr)
			mono_field_get_value(ref_texDisabled, APIManager::getSingleton()->asset_ptr_field, reinterpret_cast<void*>(&texDisabled));

		glm::vec4 rect = glm::vec4(ref_rect->m_XMin, ref_rect->m_YMin, ref_rect->m_XMin + ref_rect->m_Width, ref_rect->m_YMin + ref_rect->m_Height);
		Color color = Color(ref_color->r, ref_color->g, ref_color->b, ref_color->a);
		Color colorHov = Color(ref_colHover->r, ref_colHover->g, ref_colHover->b, ref_colHover->a);
		Color colorAct = Color(ref_colActive->r, ref_colActive->g, ref_colActive->b, ref_colActive->a);
		Color colorDis = Color(ref_colDisabled->r, ref_colDisabled->g, ref_colDisabled->b, ref_colDisabled->a);
		Button::ImageType _imageType = static_cast<Button::ImageType>(imageType);

		ImGuiWindow* win = ImGui::GetCurrentWindow();

		ElementInfo* info = nullptr;

		auto it = std::find_if(elementStack.begin(), elementStack.end(), [=](ElementInfo& inf) -> bool { return inf.id == id; });
		if (it == elementStack.end())
		{
			ElementInfo _inf;
			_inf.id = id;
			elementStack.push_back(_inf);
			info = &_inf;
		}
		else
			info = &*it;

		if (info->rect != rect)
			info->rect = rect;

		UIElement::State state = UIElement::State::Normal;

		auto mp = InputManager::getSingleton()->getMouseRelativePosition();
		glm::vec2 mousePos = glm::vec2(mp.first, mp.second);

		bool pressed = false;

		if (interactable)
		{
			int hovId = INT_MAX;

			for (auto& el : elementStack)
			{
				if (mousePos.x > el.rect.x && mousePos.x < el.rect.z &&
					mousePos.y > el.rect.y && mousePos.y < el.rect.w)
				{
					hovId = el.id;
				}
			}

			if (hovId == id)
			{
				state = UIElement::State::Hover;

				if (InputManager::getSingleton()->getMouseButtonDown(0))
					info->lmbDown = true;

				if (InputManager::getSingleton()->getMouseButtonUp(0))
				{
					if (info->lmbDown)
						pressed = true;
				}
			}
		}

		if (info->lmbDown)
			state = UIElement::State::Active;

		if (InputManager::getSingleton()->getMouseButtonUp(0))
			info->lmbDown = false;

		Button::render(win->DrawList, rect, texNormal, texHover, texActive, texDisabled, color, colorHov, colorAct, colorDis, _imageType, interactable, state, 1.0f);

		return pressed;
	}
}