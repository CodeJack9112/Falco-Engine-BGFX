#include "API_AudioSource.h"

#include "../Core/Engine.h"
#include "../Core/APIManager.h"
#include "../Components/AudioSource.h"
#include "../Core/GameObject.h"
#include "../Classes/StringConverter.h"
#include "../Assets/AudioClip.h"

namespace GX
{
	void API_AudioSource::play(MonoObject * this_ptr)
	{
		AudioSource* audio = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&audio));

		if (audio != nullptr)
			audio->play();
	}

	void API_AudioSource::pause(MonoObject* this_ptr)
	{
		AudioSource* audio = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&audio));

		if (audio != nullptr)
			audio->pause();
	}

	void API_AudioSource::resume(MonoObject* this_ptr)
	{
		AudioSource* audio = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&audio));

		if (audio != nullptr)
			audio->resume();
	}

	void API_AudioSource::stop(MonoObject * this_ptr)
	{
		AudioSource* audio = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&audio));

		if (audio != nullptr)
			audio->stop();
	}

	bool API_AudioSource::isPlaying(MonoObject * this_ptr)
	{
		AudioSource* audio = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&audio));

		if (audio != nullptr)
			return audio->getIsPlaying();

		return false;
	}

	bool API_AudioSource::isPaused(MonoObject* this_ptr)
	{
		AudioSource* audio = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&audio));

		if (audio != nullptr)
			return audio->getIsPaused();

		return false;
	}

	bool API_AudioSource::getLoop(MonoObject * this_ptr)
	{
		AudioSource* audio = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&audio));

		if (audio != nullptr)
			return audio->getLooped();

		return false;
	}

	void API_AudioSource::setLoop(MonoObject * this_ptr, bool loop)
	{
		AudioSource* audio = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&audio));

		if (audio != nullptr)
			audio->setLoop(loop);
	}

	float API_AudioSource::getVolume(MonoObject* this_ptr)
	{
		AudioSource* audio = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&audio));

		if (audio != nullptr)
			return audio->getVolume();

		return 0.0f;
	}

	void API_AudioSource::setVolume(MonoObject* this_ptr, float volume)
	{
		AudioSource* audio = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&audio));

		if (audio != nullptr)
			audio->setVolume(volume);
	}

	float API_AudioSource::getPitch(MonoObject* this_ptr)
	{
		AudioSource* audio = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&audio));

		if (audio != nullptr)
			return audio->getPitch();

		return 0.0f;
	}

	void API_AudioSource::setPitch(MonoObject* this_ptr, float volume)
	{
		AudioSource* audio = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&audio));

		if (audio != nullptr)
			audio->setPitch(volume);
	}

	float API_AudioSource::getMinDistance(MonoObject* this_ptr)
	{
		AudioSource* audio = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&audio));

		if (audio != nullptr)
			return audio->getMinDistance();

		return 0.0f;
	}

	void API_AudioSource::setMinDistance(MonoObject* this_ptr, float value)
	{
		AudioSource* audio = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&audio));

		if (audio != nullptr)
			audio->setMinDistance(value);
	}

	float API_AudioSource::getMaxDistance(MonoObject* this_ptr)
	{
		AudioSource* audio = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&audio));

		if (audio != nullptr)
			return audio->getMaxDistance();

		return 0.0f;
	}

	void API_AudioSource::setMaxDistance(MonoObject* this_ptr, float value)
	{
		AudioSource* audio = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&audio));

		if (audio != nullptr)
			audio->setMaxDistance(value);
	}

	bool API_AudioSource::getIs2D(MonoObject* this_ptr)
	{
		AudioSource* audio = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&audio));

		if (audio != nullptr)
			return audio->getIs2D();

		return false;
	}

	void API_AudioSource::setIs2D(MonoObject* this_ptr, bool value)
	{
		AudioSource* audio = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&audio));

		if (audio != nullptr)
			audio->setIs2D(value);
	}

	MonoObject* API_AudioSource::getAudioClip(MonoObject* this_ptr)
	{
		AudioSource* audio = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&audio));

		if (audio != nullptr)
		{
			AudioClip* audioClip = audio->getAudioClip();

			if (audioClip != nullptr && audioClip->isLoaded())
				return audioClip->getManagedObject();
		}

		return nullptr;
	}

	void API_AudioSource::setAudioClip(MonoObject* this_ptr, MonoObject* value)
	{
		AudioSource* audio = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&audio));

		if (audio != nullptr)
		{
			AudioClip* audioClip = nullptr;
			if (value != nullptr)
				mono_field_get_value(value, APIManager::getSingleton()->asset_ptr_field, reinterpret_cast<void*>(&audioClip));

			audio->setAudioClip(audioClip);
		}
	}

	int API_AudioSource::getTotalLength(MonoObject* this_ptr)
	{
		AudioSource* audio = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&audio));

		if (audio != nullptr)
			return audio->getTotalLength();

		return 0;
	}

	int API_AudioSource::getPlaybackPosition(MonoObject* this_ptr)
	{
		AudioSource* audio = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&audio));

		if (audio != nullptr)
			return audio->getPlaybackPosition();

		return 0;
	}

	void API_AudioSource::setPlaybackPosition(MonoObject* this_ptr, int seconds)
	{
		AudioSource* audio = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&audio));

		if (audio != nullptr)
			audio->setPlaybackPosition(seconds);
	}

	bool API_AudioSource::getPlayOnStart(MonoObject* this_ptr)
	{
		AudioSource* audio = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&audio));

		if (audio != nullptr)
			return audio->getPlayOnStart();

		return false;
	}

	void API_AudioSource::setPlayOnStart(MonoObject* this_ptr, bool value)
	{
		AudioSource* audio = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&audio));

		if (audio != nullptr)
			audio->setPlayOnStart(value);
	}
}