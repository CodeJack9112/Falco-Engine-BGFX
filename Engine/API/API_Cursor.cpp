#include "API_Cursor.h"

#include "../Core/InputManager.h"

namespace GX
{
	bool API_Cursor::getVisible()
	{
		bool visible = InputManager::getSingleton()->getCursorVisible();
		return visible;
	}

	void API_Cursor::setVisible(bool value)
	{
		InputManager::getSingleton()->setCursorVisible(value);
	}

	bool API_Cursor::getLocked()
	{
		bool locked = InputManager::getSingleton()->getCursorLocked();
		return locked;
	}

	void API_Cursor::setLocked(bool locked)
	{
		InputManager::getSingleton()->setCursorLocked(locked);
	}
}