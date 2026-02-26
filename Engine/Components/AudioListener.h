#pragma once

#include "Component.h"
#include <string>

namespace GX
{
	class AudioListener : public Component
	{
	private:
		float volume = 1.0f;

	public:
		AudioListener();
		virtual ~AudioListener();

		float getVolume() { return volume; }
		void setVolume(float value);

		bool getPaused();
		void setPaused(bool value);

		static std::string COMPONENT_TYPE;
		virtual std::string getComponentType();
		virtual Component* onClone();
	};
}