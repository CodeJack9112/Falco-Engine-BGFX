#pragma once

#include "ComponentEditor.h"

#include "../Engine/Renderer/Color.h"
#include "../Engine/glm/vec2.hpp"

namespace GX
{
	class Property;

	class CameraEditor : public ComponentEditor
	{
	public:
		CameraEditor();
		~CameraEditor();

		virtual void init(std::vector<Component*> comps);

	private:
		void onChangeClearFlags(Property* prop, int val);
		void onChangeCullingMask(Property* prop, int val);
		void onChangeClearColor(Property* prop, Color val);
		void onChangeDepth(Property* prop, int val);
		void onChangeFov(Property* prop, float val);
		void onChangeClipPlanes(Property* prop, glm::vec2 val);
		void onChangeViewportPos(Property* prop, glm::vec2 val);
		void onChangeViewportSize(Property* prop, glm::vec2 val);
		void onChangeProjectionType(Property* prop, int val);
		void onChangeOrthographicSize(Property* prop, float val);
		void onChangeOcclusionCulling(Property* prop, bool val);
	};
}