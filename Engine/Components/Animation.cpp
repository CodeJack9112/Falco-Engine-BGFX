#include "Animation.h"

#include <algorithm>

#include "../Engine/Core/Engine.h"
#include "../Engine/Core/GameObject.h"
#include "../Engine/Components/Transform.h"
#include "../Assets/AnimationClip.h"
#include "../Core/APIManager.h"
#include "../Core/Time.h"
#include "../Classes/Hash.h"

namespace GX
{
	std::string Animation::COMPONENT_TYPE = "Animation";

	Animation::Animation() : Component(APIManager::getSingleton()->animation_class)
	{
	}

	Animation::~Animation()
	{
	}

	std::string Animation::getComponentType()
	{
		return COMPONENT_TYPE;
	}

	void Animation::removeAnimationClip(int index)
	{
		if (index < animationClips.size())
			animationClips.erase(animationClips.begin() + index);
	}

	void Animation::saveInitialState()
	{
		if (initialTransforms.size() > 0)
			return;

		std::vector<Transform*>& roots = getGameObject()->getTransform()->getChildren();

		std::vector<Transform*> nstack;
		for (auto it = roots.begin(); it != roots.end(); ++it)
			nstack.push_back(*it);

		while (nstack.size() > 0)
		{
			Transform* child = *nstack.begin();
			nstack.erase(nstack.begin());

			//Save transform
			initialTransforms[child->getGameObject()->getGuidHash()] = child->getLocalTransformMatrix();
			//

			int j = 0;
			for (auto it = child->getChildren().begin(); it != child->getChildren().end(); ++it, ++j)
			{
				Transform* ch = *it;
				nstack.insert(nstack.begin() + j, ch);
			}
		}
	}

	void Animation::loadInitialState()
	{
		std::vector<Transform*>& roots = getGameObject()->getTransform()->getChildren();

		std::vector<Transform*> nstack;
		for (auto it = roots.begin(); it != roots.end(); ++it)
			nstack.push_back(*it);

		while (nstack.size() > 0)
		{
			Transform* child = *nstack.begin();
			nstack.erase(nstack.begin());

			//Load transform
			if (initialTransforms.find(child->getGameObject()->getGuidHash()) != initialTransforms.end())
				child->setLocalTransformMatrix(initialTransforms[child->getGameObject()->getGuidHash()]);
			//

			int j = 0;
			for (auto it = child->getChildren().begin(); it != child->getChildren().end(); ++it, ++j)
			{
				Transform* ch = *it;
				nstack.insert(nstack.begin() + j, ch);
			}
		}

		initialTransforms.clear();
	}

	void Animation::play(int index, float duration)
	{
		if (index >= 0 && index < animationClips.size())
		{
			if (animationClips[index].clip != nullptr)
			{
				saveInitialState();

				if (duration <= 0.0f)
					blendTree.clear();

				transitionDuration = duration;

				if (blendTree.size() > 1)
					blendTree.erase(blendTree.begin() + blendTree.size() - 1);

				BlendState state;
				state.animClip = index;
				state.weight = blendTree.size() == 0 ? 1.0f : 0.0f;
				state.time = animationClips[index].startFrame;
				state.cb = false;

				blendTree.insert(blendTree.begin(), state);
			}
		}
	}

	void Animation::play(std::string name, float duration)
	{
		auto it = std::find_if(animationClips.begin(), animationClips.end(), [=](AnimationClipInfo & inf) -> bool
			{
				return inf.name == name;
			}
		);

		if (it != animationClips.end())
		{
			int pos = std::distance(animationClips.begin(), it);
			play(pos, duration);
		}
	}

	void Animation::stop()
	{
		blendTree.clear();

		loadInitialState();
	}

	bool Animation::isPlaying()
	{
		int n = 0;
		for (auto& b : blendTree)
		{
			if (b.weight > 0 && b.time < animationClips[b.animClip].endFrame - 1)
				++n;
		}

		return n > 0;
	}

	bool Animation::isPlaying(std::string name)
	{
		auto anim = std::find_if(animationClips.begin(), animationClips.end(), [=](AnimationClipInfo& i) -> bool
			{
				return i.name == name;
			}
		);

		if (anim != animationClips.end())
		{
			int idx = std::distance(animationClips.begin(), anim);
			auto it = std::find_if(blendTree.begin(), blendTree.end(), [=](BlendState& state) -> bool
				{
					return state.animClip == idx && state.weight > 0 && state.time < anim->endFrame - 1;
				}
			);

			return it != blendTree.end();
		}

		return false;
	}

	void Animation::onUpdate(float deltaTime)
	{
		updateAnimations(deltaTime);

		float timeScale = Time::getTimeScale();

		if (blendTree.size() > 1)
		{
			for (auto it = blendTree.begin(); it != blendTree.end(); ++it)
			{
				BlendState& state = *it;
				float& weight = state.weight;

				if (it == blendTree.begin())
				{
					weight += (deltaTime / transitionDuration / blendTree.size()) * timeScale;
				}
				else
				{
					weight -= (deltaTime / transitionDuration / blendTree.size()) * timeScale;
				}

				if (weight > 1.0f) weight = 1.0f;
				if (weight < 0.0f) weight = 0.0f;
			}
		}
	}

	void Animation::updateAnimations(float deltaTime)
	{
		float timeScale = Time::getTimeScale();
		bool needClear = false;

		std::vector<std::pair<Transform*, glm::mat4x4>> transforms;
		std::vector<std::string> endedAnims;

		for (auto it = blendTree.begin(); it != blendTree.end(); ++it)
		{
			BlendState& state = *it;
			int clip = state.animClip;
			float& time = state.time;
			float weight = state.weight;

			if (clip < 0)
				continue;

			AnimationClipInfo& currentInfo = animationClips[clip];
			AnimationClip* currentClip = currentInfo.clip;

			if (currentClip == nullptr)
				continue;

			if (weight > 0)
			{
				std::vector<AnimationClipNode*>& currentAnimNodes = currentClip->getAnimationClipNodes();
				for (auto it = currentAnimNodes.begin(); it != currentAnimNodes.end(); ++it)
				{
					AnimationClipNode* currentAnimNode = *it;

					size_t nodeNameHash = currentAnimNode->getNameHash();
					GameObject* node = Engine::getSingleton()->findGameObject(nodeNameHash, getGameObject());

					if (node != nullptr)
					{
						Transform* transfrom = node->getTransform();
						glm::mat4x4 matrix = currentAnimNode->getTransformAtTime(time, transfrom);

						if (matrix == glm::identity<glm::mat4x4>())
							matrix = weight * transfrom->getLocalTransformMatrix();
						else
							matrix = weight * matrix;

						auto mt = std::find_if(transforms.begin(), transforms.end(), [=](std::pair<Transform*, glm::mat4x4>& trans) -> bool
							{
								return trans.first == transfrom;
							}
						);

						if (mt != transforms.end())
							mt->second += matrix;
						else
							transforms.push_back(std::make_pair(transfrom, matrix));
					}
				}
			}

			if (time < currentInfo.endFrame - 1)
			{
				time += (deltaTime * currentClip->getFramesPerSecond()) * timeScale * currentInfo.speed;

				if (time > currentInfo.endFrame - 1)
					time = currentInfo.endFrame - 1;
			}
			else
			{
				if (!state.cb)
				{
					endedAnims.push_back(currentInfo.name);
					state.cb = true;
				}

				if (currentInfo.loop)
				{
					time = currentInfo.startFrame;
					state.cb = false;
				}
			}
		}

		for (auto it = transforms.begin(); it != transforms.end(); ++it)
		{
			std::pair<Transform*, glm::mat4x4>& trans = *it;
			trans.first->setLocalTransformMatrix(trans.second);
		}

		transforms.clear();

		if (Engine::getSingleton()->getIsRuntimeMode())
		{
			for (auto& n : endedAnims)
			{
				MonoString* mAnimName = mono_string_new(APIManager::getSingleton()->getDomain(), n.c_str());
				void* args[2] = { managedObject, mAnimName };
				APIManager::getSingleton()->execute(managedObject, "CallOnEnded", args, "Animation,string");
			}
		}

		endedAnims.clear();
	}

	Component* Animation::onClone()
	{
		Animation* newComponent = new Animation();
		newComponent->enabled = enabled;
		newComponent->animationClips = animationClips;
		newComponent->blendTree = blendTree;
		newComponent->initialTransforms = initialTransforms;
		newComponent->transitionDuration = transitionDuration;

		return newComponent;
	}

	void Animation::onRebindObject(std::string oldObj, std::string newObj)
	{
		std::vector<std::map<size_t, glm::mat4x4>::iterator> del;

		for (auto it = initialTransforms.begin(); it != initialTransforms.end(); ++it)
		{
			if (it->first == Hash::getHash(oldObj))
			{
				initialTransforms[Hash::getHash(newObj)] = it->second;
				del.push_back(it);
			}
		}

		for (auto it = del.begin(); it != del.end(); ++it)
		{
			initialTransforms.erase(*it);
		}

		del.clear();
	}

	bool Animation::isEqualsTo(Component* other)
	{
		Animation* otherAnim = (Animation*)other;

		bool equal = otherAnim->getAnimationClips().size() == animationClips.size();

		return equal;
	}

	AnimationClipInfo* Animation::getAnimationClip(int index)
	{
		if (index < animationClips.size())
			return &animationClips[index];

		return nullptr;
	}

	AnimationClipInfo* Animation::getAnimationClip(std::string name)
	{
		auto inf = std::find_if(animationClips.begin(), animationClips.end(), [=](AnimationClipInfo& i) -> bool
			{
				return i.name == name;
			}
		);

		if (inf != animationClips.end())
			return &*inf;

		return nullptr;
	}
}