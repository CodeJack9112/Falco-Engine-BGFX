#pragma once

#include "API.h"

namespace GX
{
	class API_UIElement
	{
	public:
		static void Register()
		{
			mono_add_internal_call("FalcoEngine.UIElement::INTERNAL_getColor", (void*)getColor);
			mono_add_internal_call("FalcoEngine.UIElement::INTERNAL_setColor", (void*)setColor);
			mono_add_internal_call("FalcoEngine.UIElement::INTERNAL_getAnchor", (void*)getAnchor);
			mono_add_internal_call("FalcoEngine.UIElement::INTERNAL_setAnchor", (void*)setAnchor);
			mono_add_internal_call("FalcoEngine.UIElement::INTERNAL_getHorizontalAlignment", (void*)getHorizontalAlignment);
			mono_add_internal_call("FalcoEngine.UIElement::INTERNAL_setHorizontalAlignment", (void*)setHorizontalAlignment);
			mono_add_internal_call("FalcoEngine.UIElement::INTERNAL_getVerticalAlignment", (void*)getVerticalAlignment);
			mono_add_internal_call("FalcoEngine.UIElement::INTERNAL_setVerticalAlignment", (void*)setVerticalAlignment);
			mono_add_internal_call("FalcoEngine.UIElement::INTERNAL_getSize", (void*)getSize);
			mono_add_internal_call("FalcoEngine.UIElement::INTERNAL_setSize", (void*)setSize);
			mono_add_internal_call("FalcoEngine.UIElement::get_hovered", (void*)getHovered);
			mono_add_internal_call("FalcoEngine.UIElement::get_canvas", (void*)getCanvas);
			mono_add_internal_call("FalcoEngine.UIElement::INTERNAL_getPosition", (void*)getPosition);
			mono_add_internal_call("FalcoEngine.UIElement::INTERNAL_setPosition", (void*)setPosition);
			mono_add_internal_call("FalcoEngine.UIElement::INTERNAL_getRect", (void*)getRect);
		}

	private:
		//getColor
		static void getColor(MonoObject* this_ptr, API::Color* out_color);

		//setColor
		static void setColor(MonoObject* this_ptr, API::Color* ref_color);

		//getAnchor
		static void getAnchor(MonoObject* this_ptr, API::Vector2* out_value);

		//setAnchor
		static void setAnchor(MonoObject* this_ptr, API::Vector2* ref_value);

		//getHAlignment
		static int getHorizontalAlignment(MonoObject* this_ptr);

		//setHAlignment
		static void setHorizontalAlignment(MonoObject* this_ptr, int value);

		//getVAlignment
		static int getVerticalAlignment(MonoObject* this_ptr);

		//setVAlignment
		static void setVerticalAlignment(MonoObject* this_ptr, int value);

		//getSize
		static void getSize(MonoObject* this_ptr, API::Vector2* out_value);

		//setSize
		static void setSize(MonoObject* this_ptr, API::Vector2* ref_value);

		//isHovered
		static bool getHovered(MonoObject* this_ptr);
		
		//getCanvas
		static MonoObject* getCanvas(MonoObject* this_ptr);

		//getPosition
		static void getPosition(MonoObject* this_ptr, API::Vector3* out_value);

		//setPosition
		static void setPosition(MonoObject* this_ptr, API::Vector3* ref_value);

		//getRect
		static void getRect(MonoObject* this_ptr, API::Rect* out_value);
	};
}