#pragma once

#include <vector>
#include <string>
#include <dear-imgui/imgui.h>

#include "../Engine/glm/vec2.hpp"
#include "../Engine/glm/vec3.hpp"

#include "../Serialization/SProjectData.h"
#include "../Engine/Renderer/Window.h"

namespace GX
{
	class GameObject;
	class Component;
	class Camera;
	class Texture;
	class DialogAbout;
	class HierarchyWindow;
	class AssetsWindow;
	class InspectorWindow;
	class SceneWindow;
	class GameWindow;
	class ConsoleWindow;
	class LightingWindow;
	class NavigationWindow;
	class UIEditorWindow;
	class AnimationEditorWindow;
	class DialogRagdollEditor;
	class DialogProgress;
	class DialogAssetExporter;
	class DialogProjectBuild;
	class DialogCSGStairBuilder;
	class EditorSettings;

	class MainWindow : public Window
	{
		friend class SceneWindow;
		friend class GameWindow;
		friend class HierarchyWindow;
		friend class DialogProjectBuild;

	private:
		void onUI();
		void onPostRender();

		ImFont* stdFont = nullptr;
		ImFont* stdBigFont = nullptr;

		std::string openedSceneName = "";
		std::string openedScenePath = "";

		bool isCompiling = false;
		bool firstFrame = true;
		bool firstRestore = true;

		static bool isPlaying;

		int lastUndoPos = -1;

		glm::vec3 cameraTargetPosition = glm::vec3(0.0f);
		bool focusCameraOnObject = false;

		void updateTitle();

		static std::vector<std::function<void()>> onEndUpdateCallbacks;

		static DialogProjectBuild* projectBuildDialog;
		static DialogAbout* aboutDialog;
		static HierarchyWindow* hierarchyWindow;
		static AssetsWindow* assetsWindow;
		static InspectorWindow* inspectorWindow;
		static SceneWindow* sceneWindow;
		static GameWindow* gameWindow;
		static ConsoleWindow* consoleWindow;
		static NavigationWindow* navigationWindow;
		static LightingWindow* lightingWindow;
		static UIEditorWindow* uiEditorWindow;
		static AnimationEditorWindow* animationEditorWindow;
		static DialogRagdollEditor* ragdollEditor;
		static DialogAssetExporter* assetExporter;
		static DialogCSGStairBuilder* csgStairBuilder;

		static EditorSettings* settings;

		static void updateMainMenu();
		static void updateMainToolbar();

		void loadLastScene();
		void saveLastScene();
		void saveScene(std::string path);
		
		static MainWindow* singleton;
		static int tabSet;

		static std::vector<std::pair<int, int>> screenSizes;

		static std::vector<Texture*> icons;

		static DialogProgress* sceneLoadingProgressDlg;

		static std::string terrainFilePath;
		static bool terrainCreate;
		static bool terrainNested;

		void loadMenuIcons();

		Texture* fileNewSceneTexture = nullptr;
		Texture* fileOpenSceneTexture = nullptr;
		Texture* fileSaveSceneTexture = nullptr;
		Texture* fileSaveAsSceneTexture = nullptr;
		Texture* fileNewProjectTexture = nullptr;
		Texture* fileOpenProjectTexture = nullptr;
		Texture* fileBuildProjectTexture = nullptr;
		Texture* fileExitTexture = nullptr;

		Texture* editUndoTexture = nullptr;
		Texture* editRedoTexture = nullptr;
		Texture* editDeleteObjectTexture = nullptr;
		Texture* editDuplicateObjectTexture = nullptr;
		Texture* editAlignObjectTexture = nullptr;
		Texture* editCopyRenderImageTexture = nullptr;
		Texture* editProjectSettingsTexture = nullptr;

		Texture* objectEmptyTexture = nullptr;
		Texture* objectCameraTexture = nullptr;
		Texture* objectTerrainTexture = nullptr;
		Texture* objectWaterTexture = nullptr;
		Texture* objectAudioSourceTexture = nullptr;
		Texture* objectAudioListenerTexture = nullptr;
		Texture* objectPointLightTexture = nullptr;
		Texture* objectSpotLightTexture = nullptr;
		Texture* objectDirLightTexture = nullptr;
		Texture* objectRagdollTexture = nullptr;
		Texture* objectParticleSystemTexture = nullptr;
		Texture* objectCubeTexture = nullptr;
		Texture* objectSphereTexture = nullptr;
		Texture* objectPlaneTexture = nullptr;
		Texture* objectCapsuleTexture = nullptr;
		Texture* objectCylinderTexture = nullptr;
		Texture* objectPyramidTexture = nullptr;
		Texture* objectConeTexture = nullptr;
		Texture* objectTubeTexture = nullptr;
		Texture* objectTorusTexture = nullptr;
		Texture* objectTeapotTexture = nullptr;
		Texture* objectCanvasTexture = nullptr;
		Texture* objectImageTexture = nullptr;
		Texture* objectTextTexture = nullptr;
		Texture* objectButtonTexture = nullptr;
		Texture* objectMaskTexture = nullptr;
		Texture* objectSplineTexture = nullptr;
		Texture* objectTextInputTexture = nullptr;

		Texture* compMeshRendererTexture = nullptr;
		Texture* compDecalRendererTexture = nullptr;
		Texture* compRigidbodyTexture = nullptr;
		Texture* compVehicleTexture = nullptr;
		Texture* compBoxColliderTexture = nullptr;
		Texture* compSphereColliderTexture = nullptr;
		Texture* compCapsuleColliderTexture = nullptr;
		Texture* compMeshColliderTexture = nullptr;
		Texture* compTerrainColliderTexture = nullptr;
		Texture* compFixedJointTexture = nullptr;
		Texture* compHingeJointTexture = nullptr;
		Texture* compConeTwistJointTexture = nullptr;
		Texture* compFreeJointTexture = nullptr;
		Texture* compNavMeshAgentTexture = nullptr;
		Texture* compNavMeshObstacleTexture = nullptr;
		Texture* compAnimationTexture = nullptr;
		Texture* compScriptsGroupTexture = nullptr;
		Texture* compScriptTexture = nullptr;
		Texture* compVideoPlayerTexture = nullptr;

		Texture* assetsExportTexture = nullptr;
		Texture* assetsImportTexture = nullptr;

		Texture* viewGridTexture = nullptr;
		Texture* viewNavMeshTexture = nullptr;

		Texture* aboutTexture = nullptr;

		Texture* sceneWindowTexture = nullptr;
		Texture* inspectorWindowTexture = nullptr;
		Texture* hierarchyWindowTexture = nullptr;
		Texture* assetsWindowTexture = nullptr;
		Texture* uiEditorWindowTexture = nullptr;
		Texture* consoleWindowTexture = nullptr;

		Texture* createObjectTexture = nullptr;

		Texture* csgTexture = nullptr;
		Texture* csgModelTexture = nullptr;
		Texture* csgBrushCubeTexture = nullptr;
		Texture* csgBrushSphereTexture = nullptr;
		Texture* csgBrushConeTexture = nullptr;
		Texture* csgBrushCylinderTexture = nullptr;
		Texture* csgStairTexture = nullptr;

	public:
		MainWindow();
		~MainWindow();

		static MainWindow* getSingleton() { return singleton; }

		bool start();
		virtual void onRestore();
		virtual void onResize(int width, int height);
		virtual bool onClose();

		static void addOnEndUpdateCallback(std::function<void()> callback) { onEndUpdateCallbacks.push_back(callback); }

		bool isScriptsCompiling() { return isCompiling; }

		void openScene(std::string path);
		bool checkSceneIsDirty();

		static void HelpMarker(const char* desc);
		static void MenuItemIcon(Texture* icon);

		static InspectorWindow* getInspectorWindow() { return inspectorWindow; }
		static AssetsWindow* getAssetsWindow() { return assetsWindow; }
		static HierarchyWindow* getHierarchyWindow() { return hierarchyWindow; }
		static ConsoleWindow* getConsoleWindow() { return consoleWindow; }
		static LightingWindow* getLightingWindow() { return lightingWindow; }
		static NavigationWindow* getNavigationWindow() { return navigationWindow; }
		static SceneWindow* getSceneWindow() { return sceneWindow; }
		static GameWindow* getGameWindow() { return gameWindow; }
		static UIEditorWindow* getUIEditorWindow() { return uiEditorWindow; }
		static AnimationEditorWindow* getAnimationEditorWindow() { return animationEditorWindow; }

		static Texture* loadEditorIcon(std::string name);

		static EditorSettings* getSettings() { return settings; }

		static void updateObjectsMenu(bool itemsOnly = true, bool nested = false);
		static void updateComponentsMenu();

		ImFont* getFont() { return stdFont; }
		ImFont* getFontBig() { return stdBigFont; }

		bool isCameraFocusingOnObject() { return focusCameraOnObject; }

		void addGameObjectUndo(std::vector<GameObject*> objects, std::string name);
		void deleteGameObjectUndo(std::vector<GameObject*> objects);
		void addComponentUndo(std::vector<Component*> comps);

		static bool getIsPlaying() { return isPlaying; }

	private:
		void onSceneNew();
		void onSceneOpen();
		bool onSceneSave();
		bool onSceneSaveAs();
		void onProjectOpen();
		void onProjectSettings();
		void onExit();

		//Edit
		void onUndo();
		void onRedo();
		void onDelete();
		void onDuplicate();
		void onAlignWithView();
		void onCopyRenderImage();

		void onExportScene();
		void onExportSelected();
		void exportObjects(std::vector<GameObject*>& objects);

		void createPrimitive(std::string name, bool nested = false);
		GameObject* createEmptyObject(std::string name, bool nested = false, bool addUndo = true);
		GameObject* createUIObject(std::string name, bool nested = false);

		bool checkHasComponent(GameObject* obj, std::string compType, bool showMessage);

		//Create object
		void onCreateEmpty(bool nested = false);
		void onCreateAudioSource(bool nested = false);
		void onCreateAudioListener(bool nested = false);
		void onCreateDecal(bool nested = false);
		void onCreateCamera(bool nested = false);
		void onCreateTerrain(bool nested = false);
		void onCreatePointLight(bool nested = false);
		void onCreateSpotLight(bool nested = false);
		void onCreateDirectionalLight(bool nested = false);
		void onCreateRagdoll();
		void onCreateParticleSystem(bool nested = false);
		void onCreateWater(bool nested = false);
		void onCreateCube(bool nested = false);
		void onCreateSphere(bool nested = false);
		void onCreatePlane(bool nested = false);
		void onCreateCapsule(bool nested = false);
		void onCreateCylinder(bool nested = false);
		void onCreateTeapot(bool nested = false);
		void onCreateCone(bool nested = false);
		void onCreatePyramid(bool nested = false);
		void onCreateTorus(bool nested = false);
		void onCreateTube(bool nested = false);
		void onCreateCanvas(bool nested = false);
		void onCreateImage(bool nested = false);
		void onCreateText(bool nested = false);
		void onCreateButton(bool nested = false);
		void onCreateTextInput(bool nested = false);
		void onCreateMask(bool nested = false);
		void onCreateSpline(bool nested = false);
		void onCreateVideoPlayer(bool nested = false);
		void onCreateCSGModel(bool nested = false);
		void onCreateCSGBrushCube(bool nested = false);
		void onCreateCSGBrushSphere(bool nested = false);
		void onCreateCSGBrushCone(bool nested = false);
		void onCreateCSGBrushCylinder(bool nested = false);
		void onCreateCSGStair(bool nested = false);

		//Add component
		void onAddCamera(bool addUndo = true);
		void onAddMeshRenderer(bool addUndo = true);
		void onAddDecalRenderer(bool addUndo = true);
		void onAddLight(bool addUndo = true);
		void onAddAudioSource(bool addUndo = true);
		void onAddAudioListener(bool addUndo = true);
		void onAddRigidbody(bool addUndo = true);
		void onAddBoxCollider(bool addUndo = true);
		void onAddSphereCollider(bool addUndo = true);
		void onAddCapsuleCollider(bool addUndo = true);
		void onAddMeshCollider(bool addUndo = true);
		void onAddTerrainCollider(bool addUndo = true);
		void onAddFixedJoint(bool addUndo = true);
		void onAddHingeJoint(bool addUndo = true);
		void onAddConeTwistJoint(bool addUndo = true);
		void onAddFreeJoint(bool addUndo = true);
		void onAddVehicle(bool addUndo = true);
		void onAddNavMeshAgent(bool addUndo = true);
		void onAddNavMeshObstacle(bool addUndo = true);
		void onAddAnimationList(bool addUndo = true);
		void onAddTerrain(bool create = false, bool nested = false);
		void onAddParticleSystem(bool addUndo = true);
		void onAddWater(bool addUndo = true);
		void onAddCanvas(bool addUndo = true);
		void onAddImage(bool addUndo = true);
		void onAddText(bool addUndo = true);
		void onAddButton(bool addUndo = true);
		void onAddTextInput(bool addUndo = true);
		void onAddMask(bool addUndo = true);
		void onAddSpline(bool addUndo = true);
		void onAddVideoPlayer(bool addUndo = true);
		void onAddCSGModel(bool addUndo = true);
		void onAddCSGBrush(bool addUndo = true, int brushType = 0);

		void onAddScript(std::string className, bool addUndo = true);

		//Assets
		void onExportPackage();
		void onImportPackage();

		//Help
		void onAbout();

		//Gizmo
		void onGizmoSelect();
		void onGizmoMove();
		void onGizmoRotate();
		void onGizmoScale();
		void onGizmoLocal();
		void onGizmoWorld();
		void onGizmoSnapToGrid();
		void onGizmoCenter();
		void onGizmoPivot();

		//Play
		void onPlay();
	};
}