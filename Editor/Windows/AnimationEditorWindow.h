#pragma once

#include <string>
#include <vector>
#include <algorithm>

#include "../Engine/glm/vec3.hpp"
#include "../Engine/glm/gtc/quaternion.hpp"

namespace GX
{
	class AnimationClip;
	class AnimationClipNode;
	class Texture;

	class TimeVector3;
	class TimeQuaternion;

	class AnimationEditorWindow
	{
	private:
		bool opened = true;
		bool focused = false;

		AnimationClip* openedAnimation = nullptr;
		AnimationClipNode* openedAnimationNode = nullptr;

		std::string fileName = "";

		float panel1Size = 0.0f;
		float panel2Size = 0.0f;
		float wndPrevSize = 0.0f;

		int currentFrame = 0;
		float timelineScroll = 0;
		int dragStartTime = -1;

		bool mouseDownTimeline = false;
		bool mouseDownPos = false;
		bool mouseDownRot = false;
		bool mouseDownScl = false;

		int renameNode = -1;
		int deleteNode = -1;

		TimeVector3* dragFramePos = nullptr;
		TimeVector3* selFramePos = nullptr;

		TimeQuaternion* dragFrameRot = nullptr;
		TimeQuaternion* selFrameRot = nullptr;

		TimeVector3* dragFrameScl = nullptr;
		TimeVector3* selFrameScl = nullptr;

		glm::vec3 selFramePosVal = glm::vec3(0.0f);
		glm::vec3 selFrameRotVal = glm::vec3(0.0f);
		glm::vec3 selFrameSclVal = glm::vec3(0.0f);

		Texture* addKeyframeTex = nullptr;
		Texture* pasteTex = nullptr;
		Texture* saveTex = nullptr;

		void updateEditor();

		void addPositionUndo(glm::vec3 val);
		void addRotationUndo(glm::quat val);
		void addScaleUndo(glm::vec3 val);

	public:
		AnimationEditorWindow();
		~AnimationEditorWindow();

		void init();
		void update();
		void show(bool show) { opened = show; }
		bool getVisible() { return opened; }
		bool isFocused() { return focused; }

		AnimationClip* getOpenedAnimation() { return openedAnimation; }

		void openAnimation(AnimationClip* anim);
		void closeAnimation();
	};
}