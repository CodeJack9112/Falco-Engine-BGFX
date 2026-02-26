#include "API_Animation.h"

#include "../Core/Engine.h"
#include "../Core/APIManager.h"
#include "../Components/Animation.h"
#include "../Classes/StringConverter.h"
#include "../Assets/AnimationClip.h"

namespace GX
{
	void API_Animation::play(MonoObject * this_ptr, MonoString * name)
	{
		Animation* animList = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&animList));

		if (animList != nullptr)
		{
			std::string animName = (const char*)mono_string_to_utf8((MonoString*)name);
			animName = CP_SYS(animName);

			animList->play(animName, 0);
		}
	}

	void API_Animation::crossFade(MonoObject* this_ptr, MonoString* name, float duration)
	{
		Animation* animList = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&animList));

		if (animList != nullptr)
		{
			std::string animName = (const char*)mono_string_to_utf8((MonoString*)name);
			animName = CP_SYS(animName);

			animList->play(animName, duration);
		}
	}

	void API_Animation::stop(MonoObject * this_ptr)
	{
		Animation* animList = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&animList));

		if (animList != nullptr)
		{
			animList->stop();
		}
	}

	void API_Animation::setLoop(MonoObject * this_ptr, MonoString * name, bool loop)
	{
		Animation* animList = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&animList));

		if (animList != nullptr)
		{
			std::string animName = (const char*)mono_string_to_utf8((MonoString*)name);
			animName = CP_SYS(animName);

			AnimationClipInfo * inf = animList->getAnimationClip(animName);
			if (inf != nullptr)
				inf->loop = loop;
		}
	}

	bool API_Animation::getLoop(MonoObject * this_ptr, MonoString * name)
	{
		Animation* animList = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&animList));

		if (animList != nullptr)
		{
			std::string animName = (const char*)mono_string_to_utf8((MonoString*)name);
			animName = CP_SYS(animName);

			AnimationClipInfo* inf = animList->getAnimationClip(animName);
			if (inf != nullptr)
				return inf->loop;

			return false;
		}

		return false;
	}

	bool API_Animation::isPlayingByName(MonoObject * this_ptr, MonoString * name)
	{
		Animation* animList = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&animList));

		if (animList != nullptr)
		{
			std::string animName = (const char*)mono_string_to_utf8((MonoString*)name);
			animName = CP_SYS(animName);

			return animList->isPlaying(animName);
		}
		
		return false;
	}

	bool API_Animation::isPlaying(MonoObject * this_ptr)
	{
		Animation* animList = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&animList));

		if (animList != nullptr)
			return animList->isPlaying();

		return false;
	}

	int API_Animation::getNumAnimationClips(MonoObject* this_ptr)
	{
		Animation* animList = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&animList));

		if (animList != nullptr)
			return animList->getAnimationClips().size();

		return 0;
	}

	void API_Animation::getAnimationClipInfo(MonoObject* this_ptr, int index, API::AnimationClipInfo* out_info)
	{
		Animation* animList = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->component_ptr_field, reinterpret_cast<void*>(&animList));

		if (animList != nullptr)
		{
			if (index < animList->getAnimationClips().size())
			{
				AnimationClipInfo* info = animList->getAnimationClip(index);

				out_info->clip = info->clip->getManagedObject();
				out_info->name = mono_string_new(APIManager::getSingleton()->getDomain(), CP_UNI(info->name).c_str());
				out_info->startFrame = info->startFrame;
				out_info->endFrame = info->endFrame;
				out_info->speed = info->speed;
				out_info->loop = info->loop;
			}
		}
	}
}