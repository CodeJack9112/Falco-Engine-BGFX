#include "API_ParticleSystem.h"

#include "../Core/APIManager.h"

#include "../Components/ParticleSystem.h"

namespace GX
{
	int API_ParticleSystem::getEmittersCount(MonoObject* this_ptr)
	{
		ParticleSystem* comp = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&comp));

		if (comp != nullptr)
			return comp->getEmitters().size();

		return 0;
	}

	MonoObject* API_ParticleSystem::getEmitter(MonoObject* this_ptr, int index)
	{
		ParticleSystem* comp = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&comp));

		if (comp != nullptr)
		{
			if (index < 0 || comp->getEmitters().size() <= index)
				return nullptr;

			MonoClass* emitterClass = APIManager::getSingleton()->particleemitter_class;
			MonoObject* emitterObject = mono_object_new(APIManager::getSingleton()->getDomain(), emitterClass);
			APIManager::getSingleton()->execute(emitterObject, ".ctor");

			MonoClassField* this_field = mono_class_get_field_from_name(emitterClass, "this_ptr");

			ParticleEmitter* em = comp->getEmitters().at(index);
			mono_field_set_value(emitterObject, this_field, reinterpret_cast<void*>(&em));

			return emitterObject;
		}

		return nullptr;
	}

	void API_ParticleSystem::emitterPlay(MonoObject* this_ptr)
	{
		MonoClass* emitterClass = APIManager::getSingleton()->particleemitter_class;
		MonoClassField* this_field = mono_class_get_field_from_name(emitterClass, "this_ptr");

		ParticleEmitter* comp = nullptr;
		mono_field_get_value(this_ptr, this_field, reinterpret_cast<void*>(&comp));

		if (comp == nullptr)
			return;

		comp->play();
	}

	void API_ParticleSystem::emitterStop(MonoObject* this_ptr)
	{
		MonoClass* emitterClass = APIManager::getSingleton()->particleemitter_class;
		MonoClassField* this_field = mono_class_get_field_from_name(emitterClass, "this_ptr");

		ParticleEmitter* comp = nullptr;
		mono_field_get_value(this_ptr, this_field, reinterpret_cast<void*>(&comp));

		if (comp == nullptr)
			return;

		comp->stop();
	}

	bool API_ParticleSystem::emitterGetIsPlaying(MonoObject* this_ptr)
	{
		MonoClass* emitterClass = APIManager::getSingleton()->particleemitter_class;
		MonoClassField* this_field = mono_class_get_field_from_name(emitterClass, "this_ptr");

		ParticleEmitter* comp = nullptr;
		mono_field_get_value(this_ptr, this_field, reinterpret_cast<void*>(&comp));

		if (comp == nullptr)
			return false;

		return comp->getIsPlaying();
	}

	float API_ParticleSystem::emitterGetPlaybackTime(MonoObject* this_ptr)
	{
		MonoClass* emitterClass = APIManager::getSingleton()->particleemitter_class;
		MonoClassField* this_field = mono_class_get_field_from_name(emitterClass, "this_ptr");

		ParticleEmitter* comp = nullptr;
		mono_field_get_value(this_ptr, this_field, reinterpret_cast<void*>(&comp));

		if (comp == nullptr)
			return 0;

		return comp->getPlaybackTime();
	}
}