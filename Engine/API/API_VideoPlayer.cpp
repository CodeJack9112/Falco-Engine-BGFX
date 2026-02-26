#include "API_VideoPlayer.h"

#include "../Core/Engine.h"
#include "../Core/APIManager.h"
#include "../Components/VideoPlayer.h"
#include "../Core/GameObject.h"
#include "../Classes/StringConverter.h"
#include "../Assets/VideoClip.h"
#include "../Assets/Texture.h"

namespace GX
{
	void API_VideoPlayer::play(MonoObject* this_ptr)
	{
		VideoPlayer* player = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&player));

		if (player != nullptr)
			player->play();
	}

	void API_VideoPlayer::pause(MonoObject* this_ptr)
	{
		VideoPlayer* player = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&player));

		if (player != nullptr)
			player->pause();
	}

	void API_VideoPlayer::resume(MonoObject* this_ptr)
	{
		VideoPlayer* player = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&player));

		if (player != nullptr)
			player->resume();
	}

	void API_VideoPlayer::stop(MonoObject* this_ptr)
	{
		VideoPlayer* player = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&player));

		if (player != nullptr)
			player->stop();
	}

	bool API_VideoPlayer::isPlaying(MonoObject* this_ptr)
	{
		VideoPlayer* player = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&player));

		if (player != nullptr)
			return player->getIsPlaying();

		return false;
	}

	bool API_VideoPlayer::isPaused(MonoObject* this_ptr)
	{
		VideoPlayer* player = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&player));

		if (player != nullptr)
			return player->getIsPaused();

		return false;
	}

	bool API_VideoPlayer::getLoop(MonoObject* this_ptr)
	{
		VideoPlayer* player = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&player));

		if (player != nullptr)
			return player->getLoop();

		return false;
	}

	void API_VideoPlayer::setLoop(MonoObject* this_ptr, bool loop)
	{
		VideoPlayer* player = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&player));

		if (player != nullptr)
			player->setLoop(loop);
	}

	MonoObject* API_VideoPlayer::getVideoClip(MonoObject* this_ptr)
	{
		VideoPlayer* player = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&player));

		if (player != nullptr)
		{
			VideoClip* videoClip = player->getVideoClip();

			if (videoClip != nullptr && videoClip->isLoaded())
				return videoClip->getManagedObject();
		}

		return nullptr;
	}

	void API_VideoPlayer::setVideoClip(MonoObject* this_ptr, MonoObject* value)
	{
		VideoPlayer* player = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&player));

		if (player != nullptr)
		{
			VideoClip* videoClip = nullptr;
			if (value != nullptr)
				mono_field_get_value(value, APIManager::getSingleton()->asset_ptr_field, reinterpret_cast<void*>(&videoClip));

			player->setVideoClip(videoClip);
		}
	}

	int API_VideoPlayer::getTotalLength(MonoObject* this_ptr)
	{
		VideoPlayer* player = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&player));

		if (player != nullptr)
			return player->getTotalLength();

		return 0;
	}

	int API_VideoPlayer::getPlaybackPosition(MonoObject* this_ptr)
	{
		VideoPlayer* player = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&player));

		if (player != nullptr)
			return player->getPlaybackPosition();

		return 0;
	}

	void API_VideoPlayer::setPlaybackPosition(MonoObject* this_ptr, int seconds)
	{
		VideoPlayer* player = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&player));

		if (player != nullptr)
			player->setPlaybackPosition(seconds);
	}

	MonoObject* API_VideoPlayer::getTexture(MonoObject* this_ptr)
	{
		VideoPlayer* player = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&player));

		if (player != nullptr)
		{
			Texture* texture = player->getTexture();

			if (texture != nullptr && texture->isLoaded())
				return texture->getManagedObject();
		}

		return nullptr;
	}

	bool API_VideoPlayer::getPlayOnStart(MonoObject* this_ptr)
	{
		VideoPlayer* player = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&player));

		if (player != nullptr)
			return player->getPlayOnStart();

		return false;
	}

	void API_VideoPlayer::setPlayOnStart(MonoObject* this_ptr, bool value)
	{
		VideoPlayer* player = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&player));

		if (player != nullptr)
			player->setPlayOnStart(value);
	}
}