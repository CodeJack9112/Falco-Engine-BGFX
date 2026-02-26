#pragma once

#include <string>
#include <functional>

namespace GX
{
	class AnimationClip;

	class DialogAnimationCrop
	{
	public:
		void update();
		void show(AnimationClip* value, std::function<void()> callback);
		void hide();

	private:
		int startFrame = 0;
		int endFrame = 100;
		std::string fileName = "";
		std::string animationName = "New animation";
		AnimationClip* animationClip;

		std::function<void()> completeCallback = nullptr;

		bool visible = false;
	};
}