#pragma once

#include <vector>
#include <map>
#include <functional>
#include <string>

#include "../glm/glm.hpp"

#include "../Math/Rect.h"
#include "../Math/AxisAlignedBox.h"

namespace GX
{
	class Transform;
	class Camera;

	class Gizmo
	{
	public:
		enum class GizmoType
		{
			GT_SELECT,
			GT_MOVE,
			GT_ROTATE,
			GT_SCALE
		};

		enum class TransformSpace
		{
			TS_LOCAL,
			TS_WORLD
		};

		enum class GizmoAction
		{
			GA_MOVE,
			GA_SCALE,
			GA_ROTATE
		};

		enum class CenterBase
		{
			CB_CENTER,
			CB_PIVOT
		};
		
		typedef std::vector<Transform*> SelectedObjects;

	private:
		Camera * camera = nullptr;
		Rect rect;
		Rect clipRect;
		bool visible = false;
		bool moving = false;
		bool startMoving = false;
		GizmoType gizmoType = GizmoType::GT_MOVE;
		TransformSpace transformSpace = TransformSpace::TS_WORLD;
		CenterBase centerBase = CenterBase::CB_CENTER;
		float snapToGridMoveSize = 0.0f;
		float snapToGridRotateSize = 0.0f;
		float snapToGridScaleSize = 0.0f;
		bool snapToGrid = false;
		bool canManipulate = true;
		bool isViewportHovered = false;
		bool lButtonDown = false;
		bool rButtonDown = false;
		bool mButtonDown = false;

		float* localBounds = nullptr;

		SelectedObjects selectedObjects;
		float dist = 10.0f;

		bool showBounds = false;
		bool showTerrainGrassBounds = false;
		bool showTerrainTreesBounds = false;
		bool showTerrainDetailMeshesBounds = false;

		std::function<void(SelectedObjects& nodes, SelectedObjects& prevObjects, void* userData)> selectCallback = nullptr;
		std::function<void(SelectedObjects& nodes, GizmoAction action)> actionCallback = nullptr;
		std::vector<std::pair<std::string, std::function<void(SelectedObjects& nodes)>>> manipulateCallbacks;
		std::vector<std::pair<std::string, std::function<void(SelectedObjects& nodes)>>> manipulateStartCallbacks;
		std::vector<std::pair<std::string, std::function<void(SelectedObjects& nodes)>>> manipulateEndCallbacks;

		glm::mat4x4 transformMatrix = glm::identity<glm::mat4x4>();
		std::map<Transform*, glm::mat4x4> transformOffsets;

		glm::mat4x4 view = glm::identity<glm::mat4x4>();
		glm::mat4x4 projection = glm::identity<glm::mat4x4>();

		std::vector<Transform*> ignoreObjects;

		bool manipulatorActive = false;
		bool modKeyPressed = false;

		void drawWireframes(int viewId, int viewLayer, Camera * camera);

	public:
		Gizmo();
		~Gizmo();

		void init(Camera* cam);
		void beginFrame();
		void updateView();
		void updateManipulator();
		void updateViewManipulator();
		void setRect(Rect rc) { rect = rc; }
		void setClipRect(Rect rc) { clipRect = rc; }
		void show();
		void hide();
		bool isVisible();
		void setManipulatorActive(bool value) { manipulatorActive = value; }
		bool isModKeyPressed() { return modKeyPressed; }

		glm::vec3 findMidPoint(std::vector<Transform*>& nodes);

		void mouseMove(float x, float y);
		void mouseDown(float x, float y);
		void mouseUp(float x, float y, bool multipleSelection);

		void selectObject(Transform* idObject, bool multipleSelection = false, bool callCallback = true, void* userData = nullptr);
		void selectObjects(SelectedObjects idObjects, void* userData = nullptr, bool callCallback = true, bool updateWireframe = true);

		void setSelectCallback(std::function<void(SelectedObjects& nodes, std::vector<Transform*>& prevNodes, void* userData)> callback) { selectCallback = callback; }
		void setActionCallback(std::function<void(SelectedObjects& nodes, GizmoAction action)> callback) { actionCallback = callback; }
		std::string addManipulateCallback(std::function<void(SelectedObjects& nodes)> callback);
		std::string addManipulateStartCallback(std::function<void(SelectedObjects& nodes)> callback);
		std::string addManipulateEndCallback(std::function<void(SelectedObjects& nodes)> callback);
		void removeManipulateCallback(std::string guid);
		void removeManipulateStartCallback(std::string guid);
		void removeManipulateEndCallback(std::string guid);

		GizmoType getGizmoType() { return gizmoType; }
		TransformSpace getTransformSpace() { return transformSpace; }
		CenterBase getCenterBase() { return centerBase; }
		void setGizmoType(GizmoType type) { gizmoType = type; }
		void setTransformSpace(TransformSpace space) { transformSpace = space; }
		void setCenterBase(CenterBase value);
		void setSnapToGrid(bool snap) { snapToGrid = snap; }
		bool getSnapToGrid() { return snapToGrid; }
		
		float getSnapToGridMoveSize() { return snapToGridMoveSize; }
		void setSnapToGridMoveSize(float value) { snapToGridMoveSize = value; }

		float getSnapToGridRotateSize() { return snapToGridRotateSize; }
		void setSnapToGridRotateSize(float value) { snapToGridRotateSize = value; }
		
		float getSnapToGridScaleSize() { return snapToGridScaleSize; }
		void setSnapToGridScaleSize(float value) { snapToGridScaleSize = value; }

		void setViewportHovered(bool h) { isViewportHovered = h; }
		bool isCanManipulate() { return canManipulate; }
		void updatePosition();
		void getNodesBounds(Transform* root, AxisAlignedBox& box);

		SelectedObjects & getSelectedObjects() { return selectedObjects; }

		void clearSelection();

		void callSelectCallback();

		bool& getShowBounds() { return showBounds; }
		void setShowBounds(bool value) { showBounds = value; }

		bool& getShowTerrainGrassBounds() { return showTerrainGrassBounds; }
		void setShowTerrainGrassBounds(bool value) { showTerrainGrassBounds = value; }

		bool& getShowTerrainTreesBounds() { return showTerrainTreesBounds; }
		void setShowTerrainTreesBounds(bool value) { showTerrainTreesBounds = value; }

		bool& getShowTerrainDetailMeshesBounds() { return showTerrainDetailMeshesBounds; }
		void setShowTerrainDetailMeshesBounds(bool value) { showTerrainDetailMeshesBounds = value; }

		void setIgnoreObject(Transform* t, bool value);
	};
}