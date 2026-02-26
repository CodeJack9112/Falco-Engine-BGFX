#include "AudioListener.h"

#include "../Core/APIManager.h"
#include "../Core/SoundManager.h"

namespace GX
{
	std::string AudioListener::COMPONENT_TYPE = "AudioListener";

	AudioListener::AudioListener() : Component(APIManager::getSingleton()->audiolistener_class)
	{
		SoundManager * mgr = SoundManager::getSingleton();

		if (mgr != nullptr)
		{
			mgr->setListener(this);
		}
	}

	AudioListener::~AudioListener()
	{
		SoundManager * mgr = SoundManager::getSingleton();

		if (mgr != nullptr)
		{
			if (mgr->getListener() == this)
			{
				mgr->setListener(nullptr);
			}
		}
	}

	std::string AudioListener::getComponentType()
	{
		return COMPONENT_TYPE;
	}

	Component* AudioListener::onClone()
	{
		AudioListener* newComponent = new AudioListener();
		newComponent->enabled = enabled;
		newComponent->volume = volume;

		return newComponent;
	}

	void AudioListener::setVolume(float value)
	{
		volume = value;

		SoundManager* mgr = SoundManager::getSingleton();
		mgr->setMasterVolume(value);
	}

	bool AudioListener::getPaused()
	{
		SoundManager* mgr = SoundManager::getSingleton();
		return mgr->getPaused();
	}

	void AudioListener::setPaused(bool value)
	{
		SoundManager* mgr = SoundManager::getSingleton();
		mgr->setPaused(value);
	}
}