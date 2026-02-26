#pragma once

#include <string>
#include <vector>
#include <functional>
#include <map>

#include "../Engine/glm/vec2.hpp"
#include "../Engine/glm/vec3.hpp"

namespace GX
{
	class Texture;
	class RenderTexture;
	class Camera;
	class Gizmo;
	class Transform;
	class GameObject;
	class UndoData;

	class SceneWindow
	{
		friend class MainWindow;
		friend class NavigationWindow;

	protected:
		RenderTexture* renderTarget = nullptr;
		RenderTexture* cameraPreviewRenderTarget = nullptr;

		void onResize();

	private:
		bool opened = true;
		std::string dndId = "";
		Gizmo* gizmo = nullptr;

		bool lButtonDown = false;
		bool rButtonDown = false;
		bool mButtonDown = false;
		bool ctrlPressed = false;
		bool shiftPressed = false;
		bool mouseOver = false;

		glm::vec2 position = glm::vec2(0, 0);
		glm::vec2 size = glm::vec2(0, 0);
		glm::vec2 oldSize = glm::vec2(0, 0);
		glm::vec2 prevMousePos = glm::vec2(0, 0);

		std::map<GameObject*, int> layerMem;

		float cameraSpeed1 = 0.0f;
		float cameraSpeed2 = 0.0f;
		float cameraSpeed3 = 0.0f;
		int cameraSpeedPreset = 0;

		float cameraSpeed = 1.0f;
		float cameraSpeedNormal = 1.0f;
		float cameraSpeedFast = 2.0f;

		float gizmoIconsScale = 0.35f;

		bool focused = false;
		bool hovered = false;
		bool wasHovered = false;

		bool showStats = false;
		bool showNavMesh = false;
		bool showGrid = true;

		Texture * cameraIcon = nullptr;
		Texture * viewIcon = nullptr;
		Texture * shadowsIcon = nullptr;
		Texture * gridIcon = nullptr;
		Texture * navMeshIcon = nullptr;

		Texture * lightPointGizmoIcon = nullptr;
		Texture * lightSpotGizmoIcon = nullptr;
		Texture * lightDirGizmoIcon = nullptr;
		Texture * cameraGizmoIcon = nullptr;

		Camera* renderCamera = nullptr;

		Transform* dragObject = nullptr;

		glm::vec3 cameraPivot = glm::vec3(0.0f);

		void dropObject(GameObject * obj);

		void updateToolbar();

		void saveSettings();
		void loadSettings();

		void onGizmoSelect(std::vector<Transform*>& nodes, std::vector<Transform*>& prevNodes, void* userData);
		void selectObjects(std::vector<Transform*>& nodes, void* userData);

		void mouseDown(int x, int y, int mb);
		void mouseUp(int x, int y, int mb);
		void mouseMove(int x, int y);
		void mouseWheel(int x, int y);

		std::function<void()> onMouseHover = nullptr;
		std::function<void()> onMouseExit = nullptr;

		static UndoData* undoData;

		static void onGizmoStartManipulate(std::vector<Transform*>& objects);
		static void onGizmoEndManipulate(std::vector<Transform*>& objects);
		static void onGizmoManipulate(std::vector<Transform*>& objects);

	public:
		SceneWindow();
		~SceneWindow();

		void init();
		void update();
		void show(bool show) { opened = show; }
		bool getVisible() { return opened; }
		void drawGizmos(int viewId, int viewLayer, Camera * camera);

		glm::vec2 getPosition();
		glm::vec2 getSize();
		bool isFocused();
		bool isHovered();

		float getToolbarHeight();
		Camera* getCamera() { return renderCamera; }

		Gizmo* getGizmo() { return gizmo; }

		void setShowGrid(bool value);
		void setShowNavMesh(bool value);
	};
}