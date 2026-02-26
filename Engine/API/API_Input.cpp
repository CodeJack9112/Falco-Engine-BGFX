#include "API_Input.h"

#include "../Core/InputManager.h"

namespace GX
{
	void API_Input::getCursorDirection(API::Vector2 * out_pos)
	{
		auto dir = InputManager::getSingleton()->getMouseDirection();
		out_pos->x = dir.first;
		out_pos->y = dir.second;
	}

	void API_Input::getCursorPosition(API::Vector2* out_pos)
	{
		auto pos = InputManager::getSingleton()->getMousePosition();
		out_pos->x = pos.first;
		out_pos->y = pos.second;
	}

	void API_Input::getCursorRelativePosition(API::Vector2* out_pos)
	{
		auto relPos = InputManager::getSingleton()->getMouseRelativePosition();
		out_pos->x = relPos.first;
		out_pos->y = relPos.second;
	}

	bool API_Input::getKey(int key)
	{
		return InputManager::getSingleton()->getKey(key);
	}

	bool API_Input::getKeyDown(int key)
	{
		return InputManager::getSingleton()->getKeyDown(key);
	}

	bool API_Input::getKeyUp(int key)
	{
		return InputManager::getSingleton()->getKeyUp(key);
	}

	bool API_Input::getMouseButton(int button)
	{
		return InputManager::getSingleton()->getMouseButton(button);
	}

	bool API_Input::getMouseButtonDown(int button)
	{
		return InputManager::getSingleton()->getMouseButtonDown(button);
	}

	bool API_Input::getMouseButtonUp(int button)
	{
		return InputManager::getSingleton()->getMouseButtonUp(button);
	}

	void API_Input::getMouseWheel(API::Vector2* out_value)
	{
		auto wheel = InputManager::getSingleton()->getMouseWheel();
		out_value->x = wheel.first;
		out_value->y = wheel.second;
	}
}