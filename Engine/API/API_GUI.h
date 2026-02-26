#pragma once

#include <vector>

#include "API.h"

#include <glm/vec4.hpp>

namespace GX
{
	class API_GUI
	{
	public:
		//Register methods
		static void Register()
		{
			mono_add_internal_call("FalcoEngine.GUI::INTERNAL_text", (void*)guiText);
			mono_add_internal_call("FalcoEngine.GUI::INTERNAL_image", (void*)guiImage);
			mono_add_internal_call("FalcoEngine.GUI::INTERNAL_button", (void*)guiButton);
		}

	private:
		struct ElementInfo
		{
		public:
			int id = 0;
			bool lmbDown = false;
			glm::vec4 rect = glm::vec4(0.0f);
		};

		static std::vector<ElementInfo> elementStack;

		static void guiImage(MonoObject* ref_texture,
			API::Rect* ref_rect,
			API::Color* ref_color,
			int imageType
		);

		static void guiText(MonoObject* ref_text,
			API::Rect* ref_rect,
			MonoObject* ref_font,
			float fontSize,
			float fontResolution,
			float lineSpacing,
			int wordWrap,
			int horizontalTextAlignment,
			int verticalTextAlignment,
			API::Color* ref_color
		);

		static bool guiButton(int id,
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
			bool interactable
		);
	};
}