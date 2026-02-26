#include "API_Texture.h"

#include "../Core/Engine.h"
#include "../Core/APIManager.h"
#include "../Classes/StringConverter.h"
#include "../Classes/GUIDGenerator.h"

#include "../Assets/Texture.h"

namespace GX
{
	MonoObject* API_Texture::load(MonoString* path)
	{
		std::string _path = (const char*)mono_string_to_utf8((MonoString*)path);
		_path = CP_SYS(_path);

		Texture* texture = Texture::load(Engine::getSingleton()->getAssetsPath(), _path);

		if (texture != nullptr)
			return texture->getManagedObject();

		return nullptr;
	}

	int API_Texture::getWidth(MonoObject * this_ptr)
	{
		Texture* asset = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->asset_ptr_field, reinterpret_cast<void*>(&asset));

		if (asset != nullptr)
			return asset->getWidth();

		return 0;
	}

	int API_Texture::getHeight(MonoObject * this_ptr)
	{
		Texture* asset = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->asset_ptr_field, reinterpret_cast<void*>(&asset));

		if (asset != nullptr)
			return asset->getHeight();

		return 0;
	}

	MonoObject* API_Texture::fromBytesRGBA8(MonoArray* data, int width, int height)
	{
		int len = mono_array_length(data);
		unsigned char* dt = new unsigned char[len];

		for (int i = 0; i < len; ++i)
			dt[i] = mono_array_get(data, unsigned char, i);

		Texture* texture = Texture::create("runtime/textures/", GUIDGenerator::genGuid(), width, height, 1, Texture::TextureType::Texture2D, bgfx::TextureFormat::RGBA8, dt, len);

		return texture->getManagedObject();
	}
}