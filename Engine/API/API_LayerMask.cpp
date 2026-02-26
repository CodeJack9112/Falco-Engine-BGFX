#include "API_LayerMask.h"

#include "../Core/APIManager.h"
#include "../Classes/StringConverter.h"
#include "../Core/LayerMask.h"

namespace GX
{
	MonoString* API_LayerMask::LayerToName(int layer)
	{
		std::string name = LayerMask::layerToName(layer);
		MonoString* _name = mono_string_new(APIManager::getSingleton()->getDomain(), CP_UNI(name).c_str());

		return _name;
	}

	int API_LayerMask::NameToLayer(MonoString* name)
	{
		const char* _name = mono_string_to_utf8(name);
		int layer = LayerMask::nameToLayer(CP_SYS(_name));

		return layer;
	}
}