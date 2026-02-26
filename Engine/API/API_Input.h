#pragma once

#include "API.h"

namespace GX
{
	class API_Input
	{
	public:
		//Register methods
		static void Register()
		{
			mono_add_internal_call("FalcoEngine.Input::INTERNAL_get_cursor_direction", (void*)getCursorDirection);
			mono_add_internal_call("FalcoEngine.Input::INTERNAL_get_cursor_position", (void*)getCursorPosition);
			mono_add_internal_call("FalcoEngine.Input::INTERNAL_get_cursor_relative_position", (void*)getCursorRelativePosition);
			mono_add_internal_call("FalcoEngine.Input::INTERNAL_get_key", (void*)getKey);
			mono_add_internal_call("FalcoEngine.Input::INTERNAL_get_key_down", (void*)getKeyDown);
			mono_add_internal_call("FalcoEngine.Input::INTERNAL_get_key_up", (void*)getKeyUp);
			mono_add_internal_call("FalcoEngine.Input::INTERNAL_get_mouse_button", (void*)getMouseButton);
			mono_add_internal_call("FalcoEngine.Input::INTERNAL_get_mouse_button_down", (void*)getMouseButtonDown);
			mono_add_internal_call("FalcoEngine.Input::INTERNAL_get_mouse_button_up", (void*)getMouseButtonUp);
			mono_add_internal_call("FalcoEngine.Input::INTERNAL_get_mouse_wheel", (void*)getMouseWheel);
		}

	private:
		//Get mouse movement
		static void getCursorDirection(API::Vector2 * out_pos);
		static void getCursorPosition(API::Vector2 * out_pos);
		static void getCursorRelativePosition(API::Vector2 * out_pos);
		static bool getKey(int key);
		static bool getKeyDown(int key);
		static bool getKeyUp(int key);
		static bool getMouseButton(int button);
		static bool getMouseButtonDown(int button);
		static bool getMouseButtonUp(int button);
		static void getMouseWheel(API::Vector2 * out_value);
	};
}