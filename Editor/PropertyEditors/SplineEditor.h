#pragma once

#include <map>

#include "ComponentEditor.h"

#include "../Engine/glm/vec3.hpp"
#include "../Engine/glm/mat4x4.hpp"

namespace GX
{
	class Property;
	class Camera;
	class Texture;
	class Transform;
	class Spline;
	struct UndoData;

	class SplineEditor : public ComponentEditor
	{
	public:
		SplineEditor();
		~SplineEditor();

		virtual void init(std::vector<Component*> comps);
		virtual void update();

	private:
		Texture* splinePointTex = nullptr;
		std::string cb = "";
		std::map<Spline*, std::vector<Transform*>> transforms;
		Property* pointsGroup = nullptr;

		std::string cb1 = "";
		std::string cb2 = "";

		std::map<Spline*, glm::mat4x4> prevTransforms;

		UndoData* undoData = nullptr;

		void drawGizmos(int viewId, int viewLayer, Camera* camera);
		void updatePoints();
		void updateGizmo();
		void onChangeTransformPosition(Transform* t, Spline* spline, int index);
		void onGizmoManipulateStart(std::vector<Transform*> nodes);
		void onGizmoManipulateEnd(std::vector<Transform*> nodes);

		void onChangeClosed(bool val);
		void onAddPoint();
		void onPointPopup(int idx, int popup);
		void onChangePoint(int idx, glm::vec3 val);
	};
}