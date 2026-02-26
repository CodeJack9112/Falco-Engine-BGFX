#include "API_Canvas.h"

#include "../Core/APIManager.h"
#include "../Components/Canvas.h"

namespace GX
{
	float API_Canvas::getScale(MonoObject* this_ptr)
	{
		Canvas* canvas = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&canvas));

		return canvas->getZoom();
	}
}