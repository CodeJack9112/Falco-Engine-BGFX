#include "API_RenderTexture.h"

#include "../Core/APIManager.h"
#include "../Renderer/RenderTexture.h"
#include "../Classes/StringConverter.h"
#include "../Assets/Texture.h"

namespace GX
{
	void API_RenderTexture::ctor(MonoObject* this_ptr, int w, int h, int type)
	{
		RenderTexture* rt = new RenderTexture(w, h, static_cast<RenderTexture::TextureType>(type));

		rt->setManagedObject(this_ptr);
	}

	MonoObject* API_RenderTexture::getColorTexture(MonoObject* this_ptr)
	{
		RenderTexture* object = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->object_ptr_field, reinterpret_cast<void*>(&object));

		if (object != nullptr)
		{
			Texture* tex = object->getColorTexture();
			if (tex != nullptr)
				return tex->getManagedObject();
		}

		return nullptr;
	}

	MonoObject* API_RenderTexture::getDepthTexture(MonoObject* this_ptr)
	{
		RenderTexture* object = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->object_ptr_field, reinterpret_cast<void*>(&object));

		if (object != nullptr)
		{
			Texture* tex = object->getDepthTexture();
			if (tex != nullptr)
				return tex->getManagedObject();
		}

		return nullptr;
	}

	int API_RenderTexture::getWidth(MonoObject* this_ptr)
	{
		RenderTexture* object = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->object_ptr_field, reinterpret_cast<void*>(&object));

		if (object != nullptr)
			return object->getWidth();

		return 0;
	}

	int API_RenderTexture::getHeight(MonoObject* this_ptr)
	{
		RenderTexture* object = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->object_ptr_field, reinterpret_cast<void*>(&object));

		if (object != nullptr)
			return object->getHeight();

		return 0;
	}

	void API_RenderTexture::reset(MonoObject* this_ptr, int w, int h)
	{
		RenderTexture* object = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->object_ptr_field, reinterpret_cast<void*>(&object));

		if (object != nullptr)
		{
			object->reset(w, h);
		}
	}

	void API_RenderTexture::dispose(MonoObject* this_ptr)
	{
		RenderTexture* object = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->object_ptr_field, reinterpret_cast<void*>(&object));

		if (object != nullptr)
			delete object;
	}
}