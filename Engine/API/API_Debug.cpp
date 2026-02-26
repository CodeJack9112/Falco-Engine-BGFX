#include "API_Debug.h"

#include <iostream>
#include "../Core/Engine.h"
#include "../Core/Debug.h"
#include "../Classes/StringConverter.h"

namespace GX
{
	void API_Debug::log(MonoString* stringVal)
	{
		const char * _str = (const char*)mono_string_to_utf8((MonoString*)stringVal);
		if (_str != nullptr)
		{
			std::string str = CP_SYS(std::string(_str));

			Debug::log(str);
		}
	}
}