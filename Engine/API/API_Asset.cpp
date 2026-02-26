#include "API_Asset.h"

#include "../Core/Engine.h"
#include "../Core/APIManager.h"
#include "../Classes/StringConverter.h"
#include "../Classes/IO.h"
#include "../Assets/Asset.h"

namespace GX
{
	MonoString* API_Asset::getName(MonoObject* this_ptr)
	{
		Asset* asset = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->asset_ptr_field, reinterpret_cast<void*>(&asset));

		if (asset != nullptr)
		{
			MonoString* str = mono_string_new(APIManager::getSingleton()->getDomain(), CP_UNI(asset->getName()).c_str());
			return str;
		}

		return nullptr;
	}
}