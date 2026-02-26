#pragma once

#include <string>
#include <map>
#include <functional>
#include <vector>

#include "../glm/mat4x4.hpp"

#include "Component.h"

namespace GX
{
	class GameObject;
	class Camera;
	class Transform;
	class AnimationClip;

	struct AnimationClipInfo
	{
	public:
		AnimationClip* clip = nullptr;
		std::string name = "";
		float speed = 1.0f;
		int startFrame = 0;
		int endFrame = 0;
		bool loop = false;
	};

	class Animation : public Component
	{
		friend class MeshRenderer;

	private:
		struct BlendState
		{
		public:
			BlendState()
			{
				animClip = -1;
				weight = 1.0f;
				time = 0.01f;
			}
			~BlendState() = default;
			int animClip = -1;
			float weight = 1.0f;
			float time = 0.0f;
			bool cb = false;
		};

		std::vector<AnimationClipInfo> animationClips;
		std::map<size_t, glm::mat4x4> initialTransforms; // Guid hash, local transform

		std::vector<BlendState> blendTree;

		float transitionDuration = 0.0f;

		void saveInitialState();
		void loadInitialState();

		void updateAnimations(float deltaTime);

	public:
		Animation();
		virtual ~Animation();

		static std::string COMPONENT_TYPE;
		virtual std::string getComponentType();

		virtual void onUpdate(float deltaTime);
		virtual Component* onClone();
		virtual void onRebindObject(std::string oldObj, std::string newObj);
		virtual bool isEqualsTo(Component* other);

		int getNumAnimationClips() { return (int)animationClips.size(); }
		AnimationClipInfo* getAnimationClip(int index);
		AnimationClipInfo* getAnimationClip(std::string name);
		void addAnimationClip(AnimationClipInfo value) { animationClips.push_back(value); }
		void removeAnimationClip(int index);
		std::vector<AnimationClipInfo>& getAnimationClips() { return animationClips; }

		void play(int index, float duration = 0.0f);
		void play(std::string name, float duration = 0.0f);
		void stop();
		bool isPlaying();
		bool isPlaying(std::string name);
	};
}