#include "API_AudioListener.h"

#include "../Core/Engine.h"
#include "../Core/APIManager.h"
#include "../Components/AudioListener.h"
#include "../Core/GameObject.h"

namespace GX
{
	float API_AudioListener::getVolume(MonoObject* this_ptr)
	{
		AudioListener* listener = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&listener));

		if (listener != nullptr)
		{
			return listener->getVolume();
		}

		return 0.0f;
	}

	void API_AudioListener::setVolume(MonoObject* this_ptr, float volume)
	{
		AudioListener* listener = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&listener));

		if (listener != nullptr)
		{
			listener->setVolume(volume);
		}
	}

	bool API_AudioListener::getPaused(MonoObject* this_ptr)
	{
		AudioListener* listener = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&listener));

		if (listener != nullptr)
		{
			return listener->getPaused();
		}

		return false;
	}

	void API_AudioListener::setPaused(MonoObject* this_ptr, bool value)
	{
		AudioListener* listener = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&listener));

		if (listener != nullptr)
		{
			listener->setPaused(value);
		}
	}
}