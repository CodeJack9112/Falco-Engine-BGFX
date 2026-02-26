#include "MainWindow.h"

#include <imgui.h>
#include <dear-imgui/imgui_internal.h>
#include <dear-imgui//misc/cpp/imgui_stdlib.h>

#include <bx/math.h>
#include <bgfx/bgfx.h>

#include <boost/algorithm/string.hpp>
#include <boost/process.hpp>

#include "../Serialization/EditorSettings.h"
#include "../Classes/Toast.h"
#include "../Classes/TreeView.h"
#include "../Classes/TreeNode.h"
#include "../Classes/Undo.h"
#include "../Classes/LightmapBaker.h"

#include "../Dialogs/DialogProjectBuild.h"
#include "../Dialogs/DialogAbout.h"
#include "../Dialogs/DialogRagdollEditor.h"
#include "../Dialogs/DialogProgress.h"
#include "../Dialogs/DialogAssetExporter.h"
#include "../Dialogs/DialogCSGStairBuilder.h"

#include "AssetsWindow.h"
#include "ConsoleWindow.h"
#include "HierarchyWindow.h"
#include "InspectorWindow.h"
#include "LightingWindow.h"
#include "NavigationWindow.h"
#include "SceneWindow.h"
#include "GameWindow.h"
#include "UIEditorWindow.h"
#include "AnimationEditorWindow.h"

#include "../PropertyEditors/ProjectSettingsEditor.h"
#include "../PropertyEditors/TerrainEditor.h"
#include "../PropertyEditors/ObjectEditor.h"
#include "../PropertyEditors/CubemapEditor.h"

#include "../Engine/Core/Engine.h"
#include "../Engine/Core/APIManager.h"
#include "../Engine/Core/Debug.h"
#include "../Engine/Core/Time.h"
#include "../Engine/Classes/Helpers.h"
#include "../Engine/Classes/IO.h"
#include "../Engine/Core/InputManager.h"
#include "../Engine/Renderer/Renderer.h"
#include "../Engine/Renderer/RenderTexture.h"
#include "../Engine/Renderer/Primitives.h"
#include "../Engine/Renderer/BatchedGeometry.h"
#include "../Engine/Renderer/CSGGeometry.h"
#include "../Engine/Core/GameObject.h"
#include "../Engine/Assets/Model3DLoader.h"
#include "../Engine/Assets/Mesh.h"
#include "../Engine/Assets/Texture.h"
#include "../Engine/Assets/Scene.h"

#include "../Engine/Components/MeshRenderer.h"
#include "../Engine/Components/Camera.h"
#include "../Engine/Components/Transform.h"
#include "../Engine/Components/Light.h"
#include "../Engine/Components/Animation.h"
#include "../Engine/Components/RigidBody.h"
#include "../Engine/Components/BoxCollider.h"
#include "../Engine/Components/CapsuleCollider.h"
#include "../Engine/Components/FixedJoint.h"
#include "../Engine/Components/FreeJoint.h"
#include "../Engine/Components/HingeJoint.h"
#include "../Engine/Components/ConeTwistJoint.h"
#include "../Engine/Components/MeshCollider.h"
#include "../Engine/Components/SphereCollider.h"
#include "../Engine/Components/TerrainCollider.h"
#include "../Engine/Components/Vehicle.h"
#include "../Engine/Components/AudioListener.h"
#include "../Engine/Components/AudioSource.h"
#include "../Engine/Components/NavMeshAgent.h"
#include "../Engine/Components/NavMeshObstacle.h"
#include "../Engine/Components/Terrain.h"
#include "../Engine/Components/ParticleSystem.h"
#include "../Engine/Components/Water.h"
#include "../Engine/Components/Canvas.h"
#include "../Engine/Components/Image.h"
#include "../Engine/Components/Text.h"
#include "../Engine/Components/Button.h"
#include "../Engine/Components/TextInput.h"
#include "../Engine/Components/Mask.h"
#include "../Engine/Components/Spline.h"
#include "../Engine/Components/VideoPlayer.h"
#include "../Engine/Components/MonoScript.h"
#include "../Engine/Components/CSGModel.h"
#include "../Engine/Components/CSGBrush.h"
#include "../Engine/Components/DecalRenderer.h"

#include "../Engine/UI/ImGUIWidgets.h"
#include "../Engine/Gizmo/Gizmo.h"
#include "../Engine/Classes/StringConverter.h"
#include "../Engine/Math/Mathf.h"

#include "../Engine/OcclusionCulling/CullingThreadpool.h"

#ifdef _WIN32
#include <shellapi.h>
#include <combaseapi.h>
#include <shobjidl_core.h>
#else
#define FREEIMAGE_LIB
#include "../FreeImage/include/FreeImage.h"
#undef FREEIMAGE_LIB
#endif

#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>

namespace GX
{
	MainWindow* MainWindow::singleton = nullptr;

	DialogProjectBuild* MainWindow::projectBuildDialog = nullptr;
	DialogAbout* MainWindow::aboutDialog = nullptr;
	HierarchyWindow* MainWindow::hierarchyWindow = nullptr;
	AssetsWindow* MainWindow::assetsWindow = nullptr;
	InspectorWindow* MainWindow::inspectorWindow = nullptr;
	SceneWindow* MainWindow::sceneWindow = nullptr;
	GameWindow* MainWindow::gameWindow = nullptr;
	ConsoleWindow* MainWindow::consoleWindow = nullptr;
	LightingWindow* MainWindow::lightingWindow = nullptr;
	NavigationWindow* MainWindow::navigationWindow = nullptr;
	UIEditorWindow* MainWindow::uiEditorWindow = nullptr;
	AnimationEditorWindow* MainWindow::animationEditorWindow = nullptr;
	
	DialogRagdollEditor* MainWindow::ragdollEditor = nullptr;
	DialogProgress* MainWindow::sceneLoadingProgressDlg = nullptr;
	DialogAssetExporter* MainWindow::assetExporter = nullptr;
	DialogCSGStairBuilder* MainWindow::csgStairBuilder = nullptr;

	EditorSettings* MainWindow::settings = nullptr;

	int MainWindow::tabSet = 3;
	std::vector<std::pair<int, int>> MainWindow::screenSizes;
	std::vector<std::function<void()>> MainWindow::onEndUpdateCallbacks;
	std::vector<Texture*> MainWindow::icons;

	std::string MainWindow::terrainFilePath = "";
	bool MainWindow::terrainCreate = false;
	bool MainWindow::terrainNested = false;

	bool MainWindow::isPlaying = false;

	static glm::vec3 camPos = glm::vec3(0, 0, 0);
	static glm::highp_quat camRot = glm::identity<glm::highp_quat>();

	MainWindow::MainWindow()
	{
		singleton = this;

		projectBuildDialog = new DialogProjectBuild();
		aboutDialog = new DialogAbout();
		hierarchyWindow = new HierarchyWindow();
		assetsWindow = new AssetsWindow();
		inspectorWindow = new InspectorWindow();
		sceneWindow = new SceneWindow();
		gameWindow = new GameWindow();
		consoleWindow = new ConsoleWindow();
		lightingWindow = new LightingWindow();
		navigationWindow = new NavigationWindow();
		uiEditorWindow = new UIEditorWindow();
		animationEditorWindow = new AnimationEditorWindow();
		ragdollEditor = new DialogRagdollEditor();
		assetExporter = new DialogAssetExporter();
		csgStairBuilder = new DialogCSGStairBuilder();

		sceneLoadingProgressDlg = new DialogProgress();

		settings = new EditorSettings();

		Undo::setOnUndoAddedCallback([=]() { updateTitle(); });
	}

	MainWindow::~MainWindow()
	{
		delete projectBuildDialog;
		delete aboutDialog;
		delete hierarchyWindow;
		delete assetsWindow;
		delete inspectorWindow;
		delete sceneWindow;
		delete gameWindow;
		delete consoleWindow;
		delete lightingWindow;
		delete navigationWindow;
		delete uiEditorWindow;
		delete animationEditorWindow;
		delete ragdollEditor;
		delete assetExporter;
		delete csgStairBuilder;

		delete sceneLoadingProgressDlg;

		delete settings;
	}

	bool MainWindow::start()
	{
		Engine::getSingleton()->setIsRuntimeMode(false);

		std::string renderer = bgfx::getRendererName(bgfx::getRendererType());
		init("Falco Engine [" + renderer + "]", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 800, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED);

		settings->load(Engine::getSingleton()->getSettingsPath() + "Editor.settings");

		Renderer::getSingleton()->setUIClearColor(Color(0.25, 0.25, 0.25, 1.0));

		APIManager::getSingleton()->setBeginCompileCallback([=]()
			{
				isCompiling = true;
				Renderer::getSingleton()->renderFrame();
				Renderer::getSingleton()->renderFrame();
			}
		);
		APIManager::getSingleton()->setEndCompileCallback([=]()
			{
				isCompiling = false;
				inspectorWindow->updateCurrentEditor();
			}
		);

		ImGuiIO& io = ImGui::GetIO(); (void)io;

		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigWindowsMoveFromTitleBarOnly = true;

		ImGui::GetStyle().WindowRounding = 0;
		ImGui::GetStyle().ColorButtonPosition = ImGuiDir_Left;
		ImGui::GetStyle().IndentSpacing = 15;
		ImGui::GetStyle().FrameRounding = 4;
		ImGui::GetStyle().GrabRounding = 3;
		ImGui::GetStyle().ItemSpacing = ImVec2(8, 2);

		//Set style
		ImVec4* colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_WindowBg] = ImVec4(0.16f, 0.16f, 0.16f, 0.94f);
		colors[ImGuiCol_ChildBg] = ImVec4(0.14f, 0.14f, 0.14f, 0.00f);
		colors[ImGuiCol_PopupBg] = ImVec4(0.15f, 0.15f, 0.15f, 0.94f);
		colors[ImGuiCol_Border] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
		colors[ImGuiCol_BorderShadow] = ImVec4(0.11f, 0.11f, 0.11f, 0.00f);
		colors[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.10f, 0.10f, 0.54f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.27f, 0.27f, 0.27f, 0.67f);
		colors[ImGuiCol_TitleBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.23f);
		colors[ImGuiCol_Button] = ImVec4(0.42f, 0.42f, 0.42f, 0.47f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.44f, 0.44f, 0.44f, 0.80f);
		colors[ImGuiCol_Separator] = ImVec4(0.30f, 0.30f, 0.30f, 0.50f);
		colors[ImGuiCol_TabActive] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
		colors[ImGuiCol_TabUnfocused] = ImVec4(0.19f, 0.19f, 0.19f, 0.94f);
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.31f, 0.31f, 0.31f, 0.94f);

		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.32f, 0.32f, 0.32f, 0.40f);
		colors[ImGuiCol_CheckMark] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
		colors[ImGuiCol_SliderGrab] = ImVec4(0.33f, 0.33f, 0.33f, 1.00f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.45f, 0.45f, 0.45f, 1.00f);
		colors[ImGuiCol_Header] = ImVec4(0.36f, 0.36f, 0.36f, 0.31f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.64f, 0.64f, 0.64f, 1.00f);
		colors[ImGuiCol_SeparatorHovered] = ImVec4(0.46f, 0.46f, 0.46f, 0.78f);
		colors[ImGuiCol_SeparatorActive] = ImVec4(0.45f, 0.45f, 0.45f, 1.00f);
		colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_Tab] = ImVec4(0.26f, 0.26f, 0.26f, 0.86f);
		colors[ImGuiCol_TabHovered] = ImVec4(0.40f, 0.40f, 0.40f, 0.80f);
		colors[ImGuiCol_DockingPreview] = ImVec4(0.84f, 0.84f, 0.84f, 0.70f);
		colors[ImGuiCol_TextSelectedBg] = ImVec4(0.72f, 0.72f, 0.72f, 0.35f);
		colors[ImGuiCol_NavHighlight] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
		//

		//Load fonts
		stdFont = io.Fonts->AddFontFromFileTTF(std::string(Helper::ExePath() + "Editor/Fonts/Roboto-Regular.ttf").c_str(), 14.0f, 0, io.Fonts->GetGlyphRangesCyrillic());
		stdBigFont = io.Fonts->AddFontFromFileTTF(std::string(Helper::ExePath() + "Editor/Fonts/Roboto-Regular.ttf").c_str(), 25.0f, 0, io.Fonts->GetGlyphRangesCyrillic());
		//

		//Load icons
		icons.push_back(loadEditorIcon("Toolbar/new.png"));				//0
		icons.push_back(loadEditorIcon("Toolbar/open.png"));			//1
		icons.push_back(loadEditorIcon("Toolbar/save.png"));			//2
		icons.push_back(loadEditorIcon("Toolbar/undo.png"));			//3
		icons.push_back(loadEditorIcon("Toolbar/redo.png"));			//4
		icons.push_back(loadEditorIcon("Toolbar/select.png"));			//5
		icons.push_back(loadEditorIcon("Toolbar/move.png"));			//6
		icons.push_back(loadEditorIcon("Toolbar/rotate.png"));			//7
		icons.push_back(loadEditorIcon("Toolbar/scale.png"));			//8
		icons.push_back(loadEditorIcon("Toolbar/local.png"));			//9
		icons.push_back(loadEditorIcon("Toolbar/world.png"));			//10
		icons.push_back(loadEditorIcon("Toolbar/magnet.png"));			//11
		icons.push_back(loadEditorIcon("Toolbar/play.png"));			//12
		icons.push_back(loadEditorIcon("Toolbar/center.png"));			//13
		icons.push_back(loadEditorIcon("Toolbar/pivot.png"));			//14
		icons.push_back(loadEditorIcon("Toolbar/arrow_down.png"));		//15
		icons.push_back(loadEditorIcon("Toolbar/stop.png"));			//16

		loadMenuIcons();
		//

		//Init windows
		Toast::init();
		sceneWindow->init();
		gameWindow->init();
		consoleWindow->init();
		inspectorWindow->init();
		hierarchyWindow->init();
		assetsWindow->init();
		lightingWindow->init();
		navigationWindow->init();
		uiEditorWindow->init();
		animationEditorWindow->init();
		//

		//Get display modes
	#ifdef _WIN32
		DEVMODE dm = { 0 };
		dm.dmSize = sizeof(dm);

		for (int iModeNum = 0; EnumDisplaySettings(NULL, iModeNum, &dm) != 0; iModeNum++)
		{
			screenSizes.push_back(std::make_pair(dm.dmPelsWidth, dm.dmPelsHeight));
		}
	#else
		screenSizes.push_back({ 640, 480 });
		screenSizes.push_back({ 800, 600 });
		screenSizes.push_back({ 1024, 768 });
		screenSizes.push_back({ 1280, 800 });
		screenSizes.push_back({ 1366, 768 });
		screenSizes.push_back({ 1280, 1024 });
		screenSizes.push_back({ 1920, 1080 });

		int modes = SDL_GetNumDisplayModes(0);
		int displaysCount = SDL_GetNumVideoDisplays();

		for (int j = 0; j < displaysCount; ++j)
		{
			for (int i = 0; i < modes; ++i)
			{
				SDL_DisplayMode mode = { SDL_PIXELFORMAT_UNKNOWN, 0, 0, 0, 0 };
				if (SDL_GetDisplayMode(j, i, &mode) == 0)
				{
					screenSizes.push_back({ mode.w, mode.h });
				}
			}
		}
	#endif

		auto end = std::unique(screenSizes.begin(), screenSizes.end());
		screenSizes.erase(end, screenSizes.end());
		sort(screenSizes.begin(), screenSizes.end(), [](const std::pair<int, int>& a, const std::pair<int, int>& b) -> bool { return a.first < b.first; });

		hierarchyWindow->refreshHierarchy();
		//

		sceneWindow->onResize();
		gameWindow->onResize();

		//Show windows
		sceneWindow->show(settings->sceneWindowVisible);
		consoleWindow->show(settings->consoleWindowVisible);
		inspectorWindow->show(settings->inspectorWindowVisible);
		hierarchyWindow->show(settings->hierarchyWindowVisible);
		assetsWindow->show(settings->assetsWindowVisible);
		lightingWindow->show(settings->lightingWindowVisible);
		navigationWindow->show(settings->navigationWindowVisible);
		uiEditorWindow->show(settings->uiEditorWindowVisible);
		animationEditorWindow->show(settings->animationEditorWindowVisible);
		//

		sceneWindow->getGizmo()->setSnapToGrid(settings->snapToGrid);
		sceneWindow->getGizmo()->setSnapToGridMoveSize(settings->snapToGridMoveSize);
		sceneWindow->getGizmo()->setSnapToGridRotateSize(settings->snapToGridRotateSize);
		sceneWindow->getGizmo()->setSnapToGridScaleSize(settings->snapToGridScaleSize);
		sceneWindow->getGizmo()->setCenterBase(settings->gizmoCenterBase == 0 ? Gizmo::CenterBase::CB_CENTER : Gizmo::CenterBase::CB_PIVOT);
		sceneWindow->getGizmo()->setGizmoType(static_cast<Gizmo::GizmoType>(settings->gizmoType));
		sceneWindow->getGizmo()->setTransformSpace(static_cast<Gizmo::TransformSpace>(settings->gizmoTransformSpace));

		Camera* cam = sceneWindow->getCamera();
		cam->getGameObject()->getTransform()->setPosition(settings->cameraPosition.getValue());
		cam->getGameObject()->getTransform()->setRotation(settings->cameraRotation.getValue());

		addOnEndUpdateCallback([=]()
			{
				addOnEndUpdateCallback([=]()
					{
						loadLastScene();
					}
				);
			}
		);

		//Run loop
		Renderer::getSingleton()->setUICallback([=]() { onUI(); });
		std::string cb = Renderer::getSingleton()->addPostRenderCallback([=]() { onPostRender(); });
		bool closedByUser = run();
		//

		Renderer::getSingleton()->removePostRenderCallback(cb);

		//Save settings
		settings->sceneWindowVisible = sceneWindow->getVisible();
		settings->consoleWindowVisible = consoleWindow->getVisible();
		settings->inspectorWindowVisible = inspectorWindow->getVisible();
		settings->hierarchyWindowVisible = hierarchyWindow->getVisible();
		settings->assetsWindowVisible = assetsWindow->getVisible();
		settings->lightingWindowVisible = lightingWindow->getVisible();
		settings->navigationWindowVisible = navigationWindow->getVisible();
		settings->uiEditorWindowVisible = uiEditorWindow->getVisible();
		settings->animationEditorWindowVisible = animationEditorWindow->getVisible();
		settings->cameraPosition = SVector3(camPos.x, camPos.y, camPos.z);
		settings->cameraRotation = SQuaternion(camRot.x, camRot.y, camRot.z, camRot.w);

		settings->giBake = LightmapBaker::getGIBake();
		settings->giBounces = LightmapBaker::getGIBounces();
		settings->giIntensity = LightmapBaker::getGIIntensity();
		settings->giQuality = static_cast<int>(LightmapBaker::getGIQuality());
		settings->lightmapSize = LightmapBaker::getLightmapSize();

		settings->save();
		//

		return false;
	}

	void MainWindow::onRestore()
	{
		if (!firstRestore)
		{
			assetsWindow->reloadFiles();
			assetsWindow->reloadChangedFiles();
		}

		firstRestore = false;
	}

	void MainWindow::onResize(int width, int height)
	{
		//sceneWindow->onResize();
	}

	void MainWindow::onUI()
	{
		ImGui::PushFont(stdFont);

		bool open = true;
		ImGuiID dockspaceID = ImGui::GetID("HUB_DockSpace");

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::SetNextWindowPos(ImVec2(0, 58));
		ImGui::SetNextWindowSize(ImVec2(getWidth(), getHeight() - 58));
		ImGui::Begin("Main", &open, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground);
		ImGui::PopStyleVar(3);

		if (ImGui::DockBuilderGetNode(dockspaceID) == nullptr)
		{
			ImGui::DockBuilderRemoveNode(dockspaceID);
			ImGui::DockBuilderAddNode(dockspaceID, ImGuiDockNodeFlags_DockSpace);
			ImGui::DockBuilderSetNodeSize(dockspaceID, ImVec2(getWidth(), getHeight() - 58));

			ImGuiID dock_main_id = dockspaceID;
			ImGuiID dock_scene_id;
			ImGuiID dock_right_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.35f, nullptr, &dock_scene_id);
			ImGuiID dock_down_id = ImGui::DockBuilderSplitNode(dock_scene_id, ImGuiDir_Down, 0.15f, nullptr, &dock_scene_id);
			ImGuiID dock_prop_id;
			ImGuiID dock_prop_down_id;
			dock_right_id = ImGui::DockBuilderSplitNode(dock_right_id, ImGuiDir_Right, 0.55f, nullptr, &dock_prop_id);
			ImGuiID dock_right_down_id = ImGui::DockBuilderSplitNode(dock_right_id, ImGuiDir_Down, 0.4f, nullptr, &dock_right_id);
			ImGuiID dock_right_down_2_id = ImGui::DockBuilderSplitNode(dock_right_down_id, ImGuiDir_Down, 0.3f, nullptr, &dock_right_down_id);
			ImGuiID dock_prop_up_id = ImGui::DockBuilderSplitNode(dock_prop_id, ImGuiDir_Up, 0.5f, nullptr, &dock_prop_down_id);

			ImGui::DockBuilderDockWindow("Scene", dock_scene_id);
			ImGui::DockBuilderDockWindow("UI Editor", dock_scene_id);
			ImGui::DockBuilderDockWindow("Game", dock_scene_id);
			ImGui::DockBuilderDockWindow("Inspector", dock_right_id);
			ImGui::DockBuilderDockWindow("Lighting", dock_right_id);
			ImGui::DockBuilderDockWindow("Navigation", dock_right_id);
			ImGui::DockBuilderDockWindow("Terrain", dock_right_down_id);
			ImGui::DockBuilderDockWindow("Preview", dock_right_down_2_id);
			ImGui::DockBuilderDockWindow("Hierarchy", dock_prop_up_id);
			ImGui::DockBuilderDockWindow("Assets", dock_prop_down_id);
			ImGui::DockBuilderDockWindow("Console", dock_down_id);

			ImGui::DockBuilderFinish(dockspaceID);

			ImGui::LoadIniSettingsFromDisk((Helper::ExePath() + "imgui.ini").c_str());
		}

		ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_NoWindowMenuButton);
		ImGui::End();

		//ImGui::ShowStyleEditor();

		updateMainMenu();
		updateMainToolbar();

		hierarchyWindow->update();
		assetsWindow->update();
		inspectorWindow->update();
		lightingWindow->update();
		navigationWindow->update();
		sceneWindow->update();
		consoleWindow->update();
		gameWindow->update();
		uiEditorWindow->update();
		animationEditorWindow->update();
		aboutDialog->update();
		ragdollEditor->update();
		assetExporter->update();
		projectBuildDialog->update();
		csgStairBuilder->update();

		//sceneLoadingProgressDlg->update();

		if (tabSet > 0)
		{
			tabSet -= 1;

			if (ImGuiWindow* window = ImGui::FindWindowByName("Inspector"))
			{
				if (window->DockIsActive)
				{
					ImGuiID tab_id = ImHashStr("Inspector");
					ImGui::KeepAliveID(tab_id);
					window->DockNode->TabBar->NextSelectedTabId = tab_id;
				}
			}
			if (ImGuiWindow* window = ImGui::FindWindowByName("Scene"))
			{
				if (window->DockIsActive)
				{
					ImGuiID tab_id = ImHashStr("Scene");
					ImGui::KeepAliveID(tab_id);
					window->DockNode->TabBar->NextSelectedTabId = tab_id;
				}
			}
		}

		Toast::update();

		ImGui::PopFont();

		if (isCompiling)
		{
			int _w = Renderer::getSingleton()->getWidth();
			int _h = Renderer::getSingleton()->getHeight();

			ImGui::SetNextWindowPos(ImVec2(0, 0));
			ImGui::SetNextWindowSize(ImVec2(_w, _h));
			ImGui::SetNextWindowBgAlpha(0.5f);
			ImGui::SetNextWindowFocus();
			ImGui::Begin("Compiling scripts bg", &isCompiling, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
			ImGui::End();

			ImGui::PushFont(stdBigFont);
			int sw = 260;
			int sh = 80;
			std::string cmptext = "COMPILING SCRIPTS...";
			ImGui::SetNextWindowPos(ImVec2(_w * 0.5f - sw * 0.5f, _h * 0.5f - sh * 0.5f));
			ImGui::SetNextWindowSize(ImVec2(sw, sh));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10);
			ImGui::SetNextWindowFocus();
			ImGui::Begin("Compiling scripts", &isCompiling, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
			ImVec2 rg = ImGui::GetContentRegionAvail();
			ImVec2 ts = ImGui::CalcTextSize(cmptext.c_str());
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (rg.x * 0.5f) - (ts.x * 0.5f) - ImGui::GetScrollX());
			ImGui::SetCursorPosY(27);
			ImGui::Text(cmptext.c_str());
			ImGui::End();
			ImGui::PopStyleVar();
			ImGui::PopFont();
		}

		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_NoMouse)
			ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;

		if (firstFrame)
			firstFrame = false;
	}

	void MainWindow::loadMenuIcons()
	{
		fileNewSceneTexture = loadEditorIcon("Toolbar/new.png");
		fileOpenSceneTexture = loadEditorIcon("Toolbar/open.png");
		fileSaveSceneTexture = loadEditorIcon("Toolbar/save.png");
		fileSaveAsSceneTexture = loadEditorIcon("Toolbar/save.png");
		fileNewProjectTexture = loadEditorIcon("Toolbar/new.png");
		fileOpenProjectTexture = loadEditorIcon("Toolbar/open.png");
		fileBuildProjectTexture = loadEditorIcon("Menu/build_project.png");
		fileExitTexture = loadEditorIcon("Menu/exit.png");

		editUndoTexture = loadEditorIcon("Toolbar/undo.png");
		editRedoTexture = loadEditorIcon("Toolbar/redo.png");
		editDeleteObjectTexture = loadEditorIcon("Menu/delete_object.png");
		editDuplicateObjectTexture = loadEditorIcon("Menu/duplicate_object.png");
		editAlignObjectTexture = loadEditorIcon("Menu/align_with_view.png");
		editCopyRenderImageTexture = loadEditorIcon("Menu/copy_render_image.png");
		editProjectSettingsTexture = loadEditorIcon("Menu/project_settings.png");

		objectEmptyTexture = loadEditorIcon("Hierarchy/empty.png");
		objectCameraTexture = loadEditorIcon("Hierarchy/camera.png");
		objectTerrainTexture = loadEditorIcon("Inspector/terrain.png");
		objectWaterTexture = loadEditorIcon("Inspector/water.png");
		objectAudioSourceTexture = loadEditorIcon("Inspector/audio_source.png");
		objectAudioListenerTexture = loadEditorIcon("Inspector/audio_listener.png");
		objectPointLightTexture = loadEditorIcon("Menu/point_light.png");
		objectSpotLightTexture = loadEditorIcon("Menu/spot_light.png");
		objectDirLightTexture = loadEditorIcon("Menu/dir_light.png");
		objectRagdollTexture = loadEditorIcon("Menu/ragdoll.png");
		objectParticleSystemTexture = loadEditorIcon("Inspector/particle_system.png");
		objectCubeTexture = loadEditorIcon("Menu/cube.png");
		objectSphereTexture = loadEditorIcon("Menu/sphere.png");
		objectPlaneTexture = loadEditorIcon("Menu/plane.png");
		objectCapsuleTexture = loadEditorIcon("Menu/capsule.png");
		objectCylinderTexture = loadEditorIcon("Menu/cylinder.png");
		objectPyramidTexture = loadEditorIcon("Menu/pyramid.png");
		objectConeTexture = loadEditorIcon("Menu/cone.png");
		objectTubeTexture = loadEditorIcon("Menu/cylinder.png");
		objectTorusTexture = loadEditorIcon("Menu/torus.png");
		objectTeapotTexture = loadEditorIcon("Menu/teapot.png");
		objectCanvasTexture = loadEditorIcon("Hierarchy/canvas.png");
		objectImageTexture = loadEditorIcon("Hierarchy/image.png");
		objectTextTexture = loadEditorIcon("Hierarchy/text.png");
		objectButtonTexture = loadEditorIcon("Hierarchy/button.png");
		objectMaskTexture = loadEditorIcon("Hierarchy/mask.png");
		objectSplineTexture = loadEditorIcon("Inspector/spline.png");
		objectTextInputTexture = loadEditorIcon("Hierarchy/text_input.png");

		compMeshRendererTexture = loadEditorIcon("Hierarchy/mesh_renderer.png");
		compDecalRendererTexture = loadEditorIcon("Hierarchy/decal_renderer.png");
		compRigidbodyTexture = loadEditorIcon("Inspector/rigidbody.png");
		compVehicleTexture = loadEditorIcon("Inspector/vehicle.png");
		compBoxColliderTexture = loadEditorIcon("Inspector/box_collider.png");
		compSphereColliderTexture = loadEditorIcon("Inspector/sphere_collider.png");
		compCapsuleColliderTexture = loadEditorIcon("Inspector/capsule_collider.png");
		compMeshColliderTexture = loadEditorIcon("Inspector/mesh_collider.png");
		compTerrainColliderTexture = loadEditorIcon("Inspector/terrain_collider.png");
		compFixedJointTexture = loadEditorIcon("Inspector/fixed_joint.png");
		compHingeJointTexture = loadEditorIcon("Inspector/hinge_joint.png");
		compConeTwistJointTexture = loadEditorIcon("Inspector/fixed_joint.png");
		compFreeJointTexture = loadEditorIcon("Inspector/free_joint.png");
		compNavMeshAgentTexture = loadEditorIcon("Inspector/navmesh_agent.png");
		compNavMeshObstacleTexture = loadEditorIcon("Inspector/navmesh_obstacle.png");
		compScriptsGroupTexture = loadEditorIcon("Inspector/scripts.png");
		compScriptTexture = loadEditorIcon("Assets/cs.png");
		compAnimationTexture = loadEditorIcon("Inspector/animation.png");
		compVideoPlayerTexture = loadEditorIcon("Inspector/video_player.png");

		assetsExportTexture = loadEditorIcon("Menu/export.png");
		assetsImportTexture = loadEditorIcon("Menu/import.png");

		viewGridTexture = loadEditorIcon("Toolbar/grid.png");
		viewNavMeshTexture = loadEditorIcon("Toolbar/navmesh.png");

		aboutTexture = loadEditorIcon("Menu/about.png");

		sceneWindowTexture = loadEditorIcon("Menu/scene.png");
		inspectorWindowTexture = loadEditorIcon("Menu/inspector.png");
		hierarchyWindowTexture = loadEditorIcon("Menu/hierarchy.png");
		assetsWindowTexture = loadEditorIcon("Menu/assets.png");
		uiEditorWindowTexture = loadEditorIcon("Menu/ui_editor.png");
		consoleWindowTexture = loadEditorIcon("Menu/console.png");

		createObjectTexture = loadEditorIcon("Toolbar/add.png");

		csgTexture = loadEditorIcon("CSG/csg.png");
		csgModelTexture = loadEditorIcon("CSG/csg_model.png");
		csgBrushCubeTexture = loadEditorIcon("CSG/brush_cube.png");
		csgBrushSphereTexture = loadEditorIcon("CSG/brush_sphere.png");
		csgBrushConeTexture = loadEditorIcon("CSG/brush_cone.png");
		csgBrushCylinderTexture = loadEditorIcon("CSG/brush_cylinder.png");
		csgStairTexture = loadEditorIcon("CSG/stair.png");
	}

	void MainWindow::MenuItemIcon(Texture* icon)
	{
		ImGui::Image((void*)icon->getHandle().idx, ImVec2(16, 16), ImVec2(0, 1), ImVec2(1, 0)); ImGui::SameLine();
	}

	void MainWindow::updateMainMenu()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 4));

		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				MenuItemIcon(singleton->fileNewSceneTexture);
				if (ImGui::MenuItem("New Scene", "Ctrl+N", false, !isPlaying)) { addOnEndUpdateCallback([=]() { singleton->onSceneNew(); }); }
				MenuItemIcon(singleton->fileOpenSceneTexture);
				if (ImGui::MenuItem("Open Scene...", "Ctrl+O", false, !isPlaying)) { addOnEndUpdateCallback([=]() { singleton->onSceneOpen(); }); }
				MenuItemIcon(singleton->fileSaveSceneTexture);
				if (ImGui::MenuItem("Save Scene", "Ctrl+S", false, !isPlaying)) { addOnEndUpdateCallback([=]() { singleton->onSceneSave(); }); }
				MenuItemIcon(singleton->fileSaveAsSceneTexture);
				if (ImGui::MenuItem("Save Scene As...", "Ctrl+Shift+S", false, !isPlaying)) { addOnEndUpdateCallback([=]() { singleton->onSceneSaveAs(); }); }
				ImGui::Separator();
				MenuItemIcon(singleton->fileNewProjectTexture);
				if (ImGui::MenuItem("New Project", "", false, !isPlaying)) { addOnEndUpdateCallback([=]() { singleton->onProjectOpen(); }); }
				MenuItemIcon(singleton->fileOpenProjectTexture);
				if (ImGui::MenuItem("Open Project...", "", false, !isPlaying)) { addOnEndUpdateCallback([=]() { singleton->onProjectOpen(); }); }
				ImGui::Separator();
				MenuItemIcon(singleton->fileBuildProjectTexture);
				if (ImGui::MenuItem("Build Project", "", false, !isPlaying)) { projectBuildDialog->show(); }
				ImGui::Separator();
				MenuItemIcon(singleton->assetsExportTexture);
				if (ImGui::BeginMenu("Export..."))
				{
					if (ImGui::MenuItem("Export scene...", "", false, !isPlaying)) { addOnEndUpdateCallback([=]() { singleton->onExportScene(); }); }
					if (ImGui::MenuItem("Export selected...", "", false, !isPlaying)) { addOnEndUpdateCallback([=]() { singleton->onExportSelected(); }); }
					ImGui::EndMenu();
				}
				ImGui::Separator();
				MenuItemIcon(singleton->fileExitTexture);
				if (ImGui::MenuItem("Exit", "")) { addOnEndUpdateCallback([=]() { singleton->onExit(); }); }
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Edit"))
			{
				std::string undoMenu = "Undo";
				std::string redoMenu = "Redo";

				if (Undo::isUndoAvailable())
					undoMenu += " \"" + Undo::getLastUndoName() + "\"";

				if (Undo::isRedoAvailable())
					redoMenu += " \"" + Undo::getLastRedoName() + "\"";

				MenuItemIcon(singleton->editUndoTexture);
				if (ImGui::MenuItem(undoMenu.c_str(), "Ctrl+Z", false, Undo::isUndoAvailable())) { addOnEndUpdateCallback([=]() { singleton->onUndo(); }); }
				MenuItemIcon(singleton->editRedoTexture);
				if (ImGui::MenuItem(redoMenu.c_str(), "Ctrl+Shift+Z", false, Undo::isRedoAvailable())) { addOnEndUpdateCallback([=]() { singleton->onRedo(); }); }
				ImGui::Separator();
				MenuItemIcon(singleton->editDeleteObjectTexture);
				if (ImGui::MenuItem("Delete Object(s)", "Del")) { addOnEndUpdateCallback([=]() { singleton->onDelete(); }); }
				MenuItemIcon(singleton->editDuplicateObjectTexture);
				if (ImGui::MenuItem("Duplicate Object(s)", "Ctrl+D")) { addOnEndUpdateCallback([=]() { singleton->onDuplicate(); }); }
				MenuItemIcon(singleton->editAlignObjectTexture);
				if (ImGui::MenuItem("Align With View", "Ctrl+Shift+A")) { addOnEndUpdateCallback([=]() { singleton->onAlignWithView(); }); }
				ImGui::Separator();
				MenuItemIcon(singleton->editCopyRenderImageTexture);
				if (ImGui::MenuItem("Copy Render Image", "Ctrl+Shift+C")) { addOnEndUpdateCallback([=]() { singleton->onCopyRenderImage(); }); }
				ImGui::Separator();
				MenuItemIcon(singleton->editProjectSettingsTexture);
				if (ImGui::MenuItem("Project Settings", "")) { addOnEndUpdateCallback([=]() { singleton->onProjectSettings(); }); }

				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Create Object"))
			{
				updateObjectsMenu();

				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Add Component"))
			{
				updateComponentsMenu();

				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Assets"))
			{
				MenuItemIcon(singleton->assetsExportTexture);
				if (ImGui::MenuItem("Export Package", "")) { addOnEndUpdateCallback([=]() { singleton->onExportPackage(); }); }
				MenuItemIcon(singleton->assetsImportTexture);
				if (ImGui::MenuItem("Import Package", "")) { addOnEndUpdateCallback([=]() { singleton->onImportPackage(); }); }

				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("View"))
			{
				MenuItemIcon(singleton->viewGridTexture);
				if (ImGui::MenuItem("Show Grid", "", sceneWindow->showGrid)) { addOnEndUpdateCallback([=]() { sceneWindow->setShowGrid(!sceneWindow->showGrid); }); }
				MenuItemIcon(singleton->viewNavMeshTexture);
				if (ImGui::MenuItem("Show NavMesh", "", sceneWindow->showNavMesh)) { addOnEndUpdateCallback([=]() { sceneWindow->setShowNavMesh(!sceneWindow->showNavMesh); }); }

				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Window"))
			{
				MenuItemIcon(singleton->sceneWindowTexture);
				if (ImGui::MenuItem("Scene", "", sceneWindow->getVisible())) { sceneWindow->show(!sceneWindow->getVisible()); }
				MenuItemIcon(singleton->inspectorWindowTexture);
				if (ImGui::MenuItem("Inspector", "", inspectorWindow->getVisible())) { inspectorWindow->show(!inspectorWindow->getVisible()); }
				MenuItemIcon(singleton->hierarchyWindowTexture);
				if (ImGui::MenuItem("Hierarchy", "", hierarchyWindow->getVisible())) { hierarchyWindow->show(!hierarchyWindow->getVisible()); }
				MenuItemIcon(singleton->assetsWindowTexture);
				if (ImGui::MenuItem("Assets", "", assetsWindow->getVisible())) { assetsWindow->show(!assetsWindow->getVisible()); }
				MenuItemIcon(singleton->objectPointLightTexture);
				if (ImGui::MenuItem("Lighting", "", lightingWindow->getVisible())) { lightingWindow->show(!lightingWindow->getVisible()); }
				MenuItemIcon(singleton->viewNavMeshTexture);
				if (ImGui::MenuItem("Navigation", "", navigationWindow->getVisible())) { navigationWindow->show(!navigationWindow->getVisible()); }
				MenuItemIcon(singleton->uiEditorWindowTexture);
				if (ImGui::MenuItem("UI Editor", "", uiEditorWindow->getVisible())) { uiEditorWindow->show(!uiEditorWindow->getVisible()); }
				MenuItemIcon(singleton->compAnimationTexture);
				if (ImGui::MenuItem("Animation Editor", "", animationEditorWindow->getVisible())) { animationEditorWindow->show(!animationEditorWindow->getVisible()); }
				ImGui::Separator();
				MenuItemIcon(singleton->consoleWindowTexture);
				if (ImGui::MenuItem("Console", "", consoleWindow->getVisible())) { consoleWindow->show(!consoleWindow->getVisible()); }

				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Help"))
			{
				MenuItemIcon(singleton->aboutTexture);
				if (ImGui::MenuItem("About Falco Engine", "")) { singleton->onAbout(); }

				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}

		ImGui::PopStyleVar();
	}

	void MainWindow::updateComponentsMenu()
	{
		MenuItemIcon(singleton->compMeshRendererTexture);
		if (ImGui::BeginMenu("Rendering"))
		{
			MenuItemIcon(singleton->objectCameraTexture);
			if (ImGui::MenuItem("Camera", "")) { addOnEndUpdateCallback([=]() { singleton->onAddCamera(); }); }
			MenuItemIcon(singleton->compMeshRendererTexture);
			if (ImGui::MenuItem("Mesh Renderer", "")) { addOnEndUpdateCallback([=]() { singleton->onAddMeshRenderer(); }); }
			MenuItemIcon(singleton->compDecalRendererTexture);
			if (ImGui::MenuItem("Decal Renderer", "")) { addOnEndUpdateCallback([=]() { singleton->onAddDecalRenderer(); }); }
			MenuItemIcon(singleton->objectPointLightTexture);
			if (ImGui::MenuItem("Light", "")) { addOnEndUpdateCallback([=]() { singleton->onAddLight(); }); }
			MenuItemIcon(singleton->objectTerrainTexture);
			if (ImGui::MenuItem("Terrain", "")) { addOnEndUpdateCallback([=]() { singleton->onAddTerrain(); }); }
			MenuItemIcon(singleton->objectWaterTexture);
			if (ImGui::MenuItem("Water", "")) { addOnEndUpdateCallback([=]() { singleton->onAddWater(); }); }
			ImGui::EndMenu();
		}
		MenuItemIcon(singleton->objectAudioSourceTexture);
		if (ImGui::BeginMenu("Audio"))
		{
			MenuItemIcon(singleton->objectAudioSourceTexture);
			if (ImGui::MenuItem("Audio Source", "")) { addOnEndUpdateCallback([=]() { singleton->onAddAudioSource(); }); }
			MenuItemIcon(singleton->objectAudioListenerTexture);
			if (ImGui::MenuItem("Audio Listener", "")) { addOnEndUpdateCallback([=]() { singleton->onAddAudioListener(); }); }
			ImGui::EndMenu();
		}
		MenuItemIcon(singleton->compRigidbodyTexture);
		if (ImGui::BeginMenu("Physics"))
		{
			MenuItemIcon(singleton->compRigidbodyTexture);
			if (ImGui::MenuItem("Rigidbody", "")) { addOnEndUpdateCallback([=]() { singleton->onAddRigidbody(); }); }
			MenuItemIcon(singleton->compVehicleTexture);
			if (ImGui::MenuItem("Vehicle", "")) { addOnEndUpdateCallback([=]() { singleton->onAddVehicle(); }); }
			ImGui::Separator();
			MenuItemIcon(singleton->compBoxColliderTexture);
			if (ImGui::MenuItem("Box Collider", "")) { addOnEndUpdateCallback([=]() { singleton->onAddBoxCollider(); }); }
			MenuItemIcon(singleton->compSphereColliderTexture);
			if (ImGui::MenuItem("Sphere Collider", "")) { addOnEndUpdateCallback([=]() { singleton->onAddSphereCollider(); }); }
			MenuItemIcon(singleton->compCapsuleColliderTexture);
			if (ImGui::MenuItem("Capsule Collider", "")) { addOnEndUpdateCallback([=]() { singleton->onAddCapsuleCollider(); }); }
			MenuItemIcon(singleton->compMeshColliderTexture);
			if (ImGui::MenuItem("Mesh Collider", "")) { addOnEndUpdateCallback([=]() { singleton->onAddMeshCollider(); }); }
			MenuItemIcon(singleton->compTerrainColliderTexture);
			if (ImGui::MenuItem("Terrain Collider", "")) { addOnEndUpdateCallback([=]() { singleton->onAddTerrainCollider(); }); }
			ImGui::Separator();
			MenuItemIcon(singleton->compFixedJointTexture);
			if (ImGui::MenuItem("Fixed Joint", "")) { addOnEndUpdateCallback([=]() { singleton->onAddFixedJoint(); }); }
			MenuItemIcon(singleton->compHingeJointTexture);
			if (ImGui::MenuItem("Hinge Joint", "")) { addOnEndUpdateCallback([=]() { singleton->onAddHingeJoint(); }); }
			MenuItemIcon(singleton->compConeTwistJointTexture);
			if (ImGui::MenuItem("Cone Twist Joint", "")) { addOnEndUpdateCallback([=]() { singleton->onAddConeTwistJoint(); }); }
			MenuItemIcon(singleton->compFreeJointTexture);
			if (ImGui::MenuItem("Free Joint", "")) { addOnEndUpdateCallback([=]() { singleton->onAddFreeJoint(); }); }
			ImGui::EndMenu();
		}
		MenuItemIcon(singleton->compNavMeshAgentTexture);
		if (ImGui::BeginMenu("Navigation"))
		{
			MenuItemIcon(singleton->compNavMeshAgentTexture);
			if (ImGui::MenuItem("NavMesh Agent", "")) { addOnEndUpdateCallback([=]() { singleton->onAddNavMeshAgent(); }); }
			MenuItemIcon(singleton->compNavMeshObstacleTexture);
			if (ImGui::MenuItem("NavMesh Obstacle", "")) { addOnEndUpdateCallback([=]() { singleton->onAddNavMeshObstacle(); }); }
			ImGui::EndMenu();
		}
		MenuItemIcon(singleton->objectCanvasTexture);
		if (ImGui::BeginMenu("UI"))
		{
			MenuItemIcon(singleton->objectCanvasTexture);
			if (ImGui::MenuItem("Canvas", "")) { addOnEndUpdateCallback([=]() { singleton->onAddCanvas(); }); }
			ImGui::Separator();
			MenuItemIcon(singleton->objectImageTexture);
			if (ImGui::MenuItem("Image", "")) { addOnEndUpdateCallback([=]() { singleton->onAddImage(); }); }
			MenuItemIcon(singleton->objectTextTexture);
			if (ImGui::MenuItem("Text", "")) { addOnEndUpdateCallback([=]() { singleton->onAddText(); }); }
			MenuItemIcon(singleton->objectButtonTexture);
			if (ImGui::MenuItem("Button", "")) { addOnEndUpdateCallback([=]() { singleton->onAddButton(); }); }
			MenuItemIcon(singleton->objectTextInputTexture);
			if (ImGui::MenuItem("Text Input", "")) { addOnEndUpdateCallback([=]() { singleton->onAddTextInput(); }); }
			ImGui::Separator();
			MenuItemIcon(singleton->objectMaskTexture);
			if (ImGui::MenuItem("Mask", "")) { addOnEndUpdateCallback([=]() { singleton->onAddMask(); }); }
			ImGui::EndMenu();
		}
		MenuItemIcon(singleton->objectParticleSystemTexture);
		if (ImGui::BeginMenu("Effects"))
		{
			MenuItemIcon(singleton->objectParticleSystemTexture);
			if (ImGui::MenuItem("Particle System", "")) { addOnEndUpdateCallback([=]() { singleton->onAddParticleSystem(); }); }
			ImGui::EndMenu();
		}
		MenuItemIcon(singleton->csgTexture);
		if (ImGui::BeginMenu("CSG"))
		{
			MenuItemIcon(singleton->csgModelTexture);
			if (ImGui::MenuItem("Model", "")) { addOnEndUpdateCallback([=]() { singleton->onAddCSGModel(); }); }
			ImGui::Separator();
			MenuItemIcon(singleton->csgBrushCubeTexture);
			if (ImGui::MenuItem("Cube Brush", "")) { addOnEndUpdateCallback([=]() { singleton->onAddCSGBrush(static_cast<int>(CSGBrush::BrushType::Cube)); }); }
			MenuItemIcon(singleton->csgBrushSphereTexture);
			if (ImGui::MenuItem("Sphere Brush", "")) { addOnEndUpdateCallback([=] { singleton->onAddCSGBrush(static_cast<int>(CSGBrush::BrushType::Sphere)); }); }
			MenuItemIcon(singleton->csgBrushConeTexture);
			if (ImGui::MenuItem("Cone Brush", "")) { addOnEndUpdateCallback([=] { singleton->onAddCSGBrush(static_cast<int>(CSGBrush::BrushType::Cone)); }); }
			MenuItemIcon(singleton->csgBrushCylinderTexture);
			if (ImGui::MenuItem("Cylinder Brush", "")) { addOnEndUpdateCallback([=] { singleton->onAddCSGBrush(static_cast<int>(CSGBrush::BrushType::Cylinder)); }); }
			ImGui::EndMenu();
		}
		MenuItemIcon(singleton->compAnimationTexture);
		if (ImGui::BeginMenu("Misc"))
		{
			MenuItemIcon(singleton->compAnimationTexture);
			if (ImGui::MenuItem("Animation", "")) { addOnEndUpdateCallback([=]() { singleton->onAddAnimationList(); }); }
			MenuItemIcon(singleton->objectSplineTexture);
			if (ImGui::MenuItem("Spline", "")) { addOnEndUpdateCallback([=]() { singleton->onAddSpline(); }); }
			MenuItemIcon(singleton->compVideoPlayerTexture);
			if (ImGui::MenuItem("Video Player", "")) { addOnEndUpdateCallback([=]() { singleton->onAddVideoPlayer(); }); }
			ImGui::EndMenu();
		}
		MenuItemIcon(singleton->compScriptsGroupTexture);
		if (ImGui::BeginMenu("Scripts"))
		{
			auto& scripts = APIManager::getSingleton()->getBehaviourClasses();

			for (auto& script : scripts)
			{
				std::string scriptName = mono_class_get_name(script);
				MenuItemIcon(singleton->compScriptTexture);
				if (ImGui::MenuItem(scriptName.c_str(), "")) { addOnEndUpdateCallback([=]() { singleton->onAddScript(scriptName); }); }
			}

			ImGui::EndMenu();
		}
	}

	Texture* MainWindow::loadEditorIcon(std::string name)
	{
		Texture* tex = Texture::load(Helper::ExePath(), "Editor/Icons/" + name, false, Texture::CompressionMethod::None, true);

		if (tex != nullptr && tex->isLoaded())
			return tex;

		return nullptr;
	}

	void MainWindow::updateObjectsMenu(bool itemsOnly, bool nested)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 4));

		bool contextMenu = false;

		if (itemsOnly)
			contextMenu = true;
		else
		{
			MenuItemIcon(singleton->createObjectTexture);
			contextMenu = ImGui::BeginMenu("Create");
		}

		if (contextMenu)
		{
			MenuItemIcon(singleton->objectEmptyTexture);
			if (ImGui::MenuItem("Empty", "")) { addOnEndUpdateCallback([=] { singleton->onCreateEmpty(nested); }); }
			ImGui::Separator();
			MenuItemIcon(singleton->compMeshRendererTexture);
			if (ImGui::BeginMenu("Rendering"))
			{
				MenuItemIcon(singleton->compDecalRendererTexture);
				if (ImGui::MenuItem("Decal", "")) { addOnEndUpdateCallback([=] { singleton->onCreateDecal(nested); }); }
				MenuItemIcon(singleton->objectCameraTexture);
				if (ImGui::MenuItem("Camera", "")) { addOnEndUpdateCallback([=] { singleton->onCreateCamera(nested); }); }
				MenuItemIcon(singleton->objectTerrainTexture);
				if (ImGui::MenuItem("Terrain", "")) { addOnEndUpdateCallback([=] { singleton->onCreateTerrain(nested); }); }
				MenuItemIcon(singleton->objectWaterTexture);
				if (ImGui::MenuItem("Water", "")) { addOnEndUpdateCallback([=] { singleton->onCreateWater(nested); }); }
				ImGui::EndMenu();
			}
			MenuItemIcon(singleton->objectAudioSourceTexture);
			if (ImGui::BeginMenu("Audio"))
			{
				MenuItemIcon(singleton->objectAudioSourceTexture);
				if (ImGui::MenuItem("Audio Source", "")) { addOnEndUpdateCallback([=] { singleton->onCreateAudioSource(nested); }); }
				MenuItemIcon(singleton->objectAudioListenerTexture);
				if (ImGui::MenuItem("Audio Listener", "")) { addOnEndUpdateCallback([=] { singleton->onCreateAudioListener(nested); }); }
				ImGui::EndMenu();
			}
			MenuItemIcon(singleton->objectPointLightTexture);
			if (ImGui::BeginMenu("Light"))
			{
				MenuItemIcon(singleton->objectPointLightTexture);
				if (ImGui::MenuItem("Point Light", "")) { addOnEndUpdateCallback([=] { singleton->onCreatePointLight(nested); }); }
				MenuItemIcon(singleton->objectSpotLightTexture);
				if (ImGui::MenuItem("Spot Light", "")) { addOnEndUpdateCallback([=] { singleton->onCreateSpotLight(nested); }); }
				MenuItemIcon(singleton->objectDirLightTexture);
				if (ImGui::MenuItem("Directional Light", "")) { addOnEndUpdateCallback([=] { singleton->onCreateDirectionalLight(nested); }); }
				ImGui::EndMenu();
			}
			MenuItemIcon(singleton->objectRagdollTexture);
			if (ImGui::BeginMenu("Physics"))
			{
				MenuItemIcon(singleton->objectRagdollTexture);
				if (ImGui::MenuItem("Ragdoll", "")) { addOnEndUpdateCallback([=] { singleton->onCreateRagdoll(); }); }
				ImGui::EndMenu();
			}
			MenuItemIcon(singleton->objectParticleSystemTexture);
			if (ImGui::BeginMenu("Effects"))
			{
				MenuItemIcon(singleton->objectParticleSystemTexture);
				if (ImGui::MenuItem("Particle System", "")) { addOnEndUpdateCallback([=] { singleton->onCreateParticleSystem(nested); }); }
				ImGui::EndMenu();
			}
			MenuItemIcon(singleton->objectCubeTexture);
			if (ImGui::BeginMenu("Primitives"))
			{
				MenuItemIcon(singleton->objectCubeTexture);
				if (ImGui::MenuItem("Cube", "")) { addOnEndUpdateCallback([=] { singleton->onCreateCube(nested); }); }
				MenuItemIcon(singleton->objectSphereTexture);
				if (ImGui::MenuItem("Sphere", "")) { addOnEndUpdateCallback([=] { singleton->onCreateSphere(nested); }); }
				MenuItemIcon(singleton->objectPlaneTexture);
				if (ImGui::MenuItem("Plane", "")) { addOnEndUpdateCallback([=] { singleton->onCreatePlane(nested); }); }
				MenuItemIcon(singleton->objectCapsuleTexture);
				if (ImGui::MenuItem("Capsule", "")) { addOnEndUpdateCallback([=] { singleton->onCreateCapsule(nested); }); }
				MenuItemIcon(singleton->objectCylinderTexture);
				if (ImGui::MenuItem("Cylinder", "")) { addOnEndUpdateCallback([=] { singleton->onCreateCylinder(nested); }); }
				MenuItemIcon(singleton->objectPyramidTexture);
				if (ImGui::MenuItem("Pyramid", "")) { addOnEndUpdateCallback([=] { singleton->onCreatePyramid(nested); }); }
				MenuItemIcon(singleton->objectConeTexture);
				if (ImGui::MenuItem("Cone", "")) { addOnEndUpdateCallback([=] { singleton->onCreateCone(nested); }); }
				MenuItemIcon(singleton->objectTubeTexture);
				if (ImGui::MenuItem("Tube", "")) { addOnEndUpdateCallback([=] { singleton->onCreateTube(nested); }); }
				MenuItemIcon(singleton->objectTorusTexture);
				if (ImGui::MenuItem("Torus", "")) { addOnEndUpdateCallback([=] { singleton->onCreateTorus(nested); }); }
				MenuItemIcon(singleton->objectTeapotTexture);
				if (ImGui::MenuItem("Teapot", "")) { addOnEndUpdateCallback([=] { singleton->onCreateTeapot(nested); }); }
				ImGui::EndMenu();
			}
			MenuItemIcon(singleton->objectCanvasTexture);
			if (ImGui::BeginMenu("UI"))
			{
				MenuItemIcon(singleton->objectCanvasTexture);
				if (ImGui::MenuItem("Canvas", "")) { addOnEndUpdateCallback([=] { singleton->onCreateCanvas(nested); }); }
				ImGui::Separator();
				MenuItemIcon(singleton->objectImageTexture);
				if (ImGui::MenuItem("Image", "")) { addOnEndUpdateCallback([=] { singleton->onCreateImage(nested); }); }
				MenuItemIcon(singleton->objectTextTexture);
				if (ImGui::MenuItem("Text", "")) { addOnEndUpdateCallback([=] { singleton->onCreateText(nested); }); }
				MenuItemIcon(singleton->objectButtonTexture);
				if (ImGui::MenuItem("Button", "")) { addOnEndUpdateCallback([=] { singleton->onCreateButton(nested); }); }
				MenuItemIcon(singleton->objectTextInputTexture);
				if (ImGui::MenuItem("Text Input", "")) { addOnEndUpdateCallback([=] { singleton->onCreateTextInput(nested); }); }
				ImGui::Separator();
				MenuItemIcon(singleton->objectMaskTexture);
				if (ImGui::MenuItem("Mask", "")) { addOnEndUpdateCallback([=] { singleton->onCreateMask(nested); }); }
				ImGui::EndMenu();
			}
			MenuItemIcon(singleton->csgTexture);
			if (ImGui::BeginMenu("CSG"))
			{
				MenuItemIcon(singleton->csgModelTexture);
				if (ImGui::MenuItem("Model", "")) { addOnEndUpdateCallback([=] { singleton->onCreateCSGModel(nested); }); }
				ImGui::Separator();
				MenuItemIcon(singleton->csgBrushCubeTexture);
				if (ImGui::MenuItem("Cube Brush", "")) { addOnEndUpdateCallback([=] { singleton->onCreateCSGBrushCube(nested); }); }
				MenuItemIcon(singleton->csgBrushSphereTexture);
				if (ImGui::MenuItem("Sphere Brush", "")) { addOnEndUpdateCallback([=] { singleton->onCreateCSGBrushSphere(nested); }); }
				MenuItemIcon(singleton->csgBrushConeTexture);
				if (ImGui::MenuItem("Cone Brush", "")) { addOnEndUpdateCallback([=] { singleton->onCreateCSGBrushCone(nested); }); }
				MenuItemIcon(singleton->csgBrushCylinderTexture);
				if (ImGui::MenuItem("Cylinder Brush", "")) { addOnEndUpdateCallback([=] { singleton->onCreateCSGBrushCylinder(nested); }); }
				MenuItemIcon(singleton->csgStairTexture);
				if (ImGui::MenuItem("Stair", "")) { addOnEndUpdateCallback([=] { singleton->onCreateCSGStair(nested); }); }
				ImGui::EndMenu();
			}
			MenuItemIcon(singleton->objectSplineTexture);
			if (ImGui::BeginMenu("Misc"))
			{
				MenuItemIcon(singleton->objectSplineTexture);
				if (ImGui::MenuItem("Spline", "")) { addOnEndUpdateCallback([=] { singleton->onCreateSpline(nested); }); }
				MenuItemIcon(singleton->compVideoPlayerTexture);
				if (ImGui::MenuItem("Video Player", "")) { addOnEndUpdateCallback([=] { singleton->onCreateVideoPlayer(nested); }); }
				ImGui::EndMenu();
			}

			if (!itemsOnly)
				ImGui::EndMenu();
		}

		ImGui::PopStyleVar();
	}

	void MainWindow::HelpMarker(const char* desc)
	{
		if (ImGui::IsItemHovered() && GImGui->HoveredIdTimer > 0.5f)
		{
			ImGui::BeginTooltip();
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextUnformatted(desc);
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
	}

	void MainWindow::updateMainToolbar()
	{
		bool open = true;
		glm::vec2 ws = glm::vec2(singleton->getWidth(), singleton->getHeight());
		ImVec2 pos = ImVec2(0, 20);
		ImVec2 size = ImVec2(ws.x, 38);

		bool snap = sceneWindow->getGizmo()->getSnapToGrid();
		bool gizmoSelect = sceneWindow->getGizmo()->getGizmoType() == Gizmo::GizmoType::GT_SELECT;
		bool gizmoMove = sceneWindow->getGizmo()->getGizmoType() == Gizmo::GizmoType::GT_MOVE;
		bool gizmoRotate = sceneWindow->getGizmo()->getGizmoType() == Gizmo::GizmoType::GT_ROTATE;
		bool gizmoScale = sceneWindow->getGizmo()->getGizmoType() == Gizmo::GizmoType::GT_SCALE;
		bool gizmoLocal = sceneWindow->getGizmo()->getTransformSpace() == Gizmo::TransformSpace::TS_LOCAL;
		bool gizmoWorld = sceneWindow->getGizmo()->getTransformSpace() == Gizmo::TransformSpace::TS_WORLD;
		bool gizmoCenter = sceneWindow->getGizmo()->getCenterBase() == Gizmo::CenterBase::CB_CENTER;
		bool gizmoPivot = sceneWindow->getGizmo()->getCenterBase() == Gizmo::CenterBase::CB_PIVOT;

		ImVec4 col3 = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
		ImVec4 col4 = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
		ImVec4 col5 = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
		ImVec4 col6 = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
		ImVec4 col7 = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
		ImVec4 col8 = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
		ImVec4 col9 = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
		ImVec4 col10 = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
		ImVec4 col11 = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
		ImVec4 col12 = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
		ImVec4 col13 = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
		ImVec4 col14 = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

		if (snap) col3 = ImVec4(1, 1, 1, 1);
		if (gizmoSelect) col4 = ImVec4(1, 1, 1, 1);
		if (gizmoMove) col5 = ImVec4(1, 1, 1, 1);
		if (gizmoRotate) col6 = ImVec4(1, 1, 1, 1);
		if (gizmoScale) col7 = ImVec4(1, 1, 1, 1);
		if (gizmoLocal) col8 = ImVec4(1, 1, 1, 1);
		if (gizmoWorld) col9 = ImVec4(1, 1, 1, 1);
		if (gizmoCenter) col10 = ImVec4(1, 1, 1, 1);
		if (gizmoPivot) col11 = ImVec4(1, 1, 1, 1);
		if (isPlaying) col14 = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);

		if (Undo::isUndoAvailable()) col12 = ImVec4(1, 1, 1, 1);
		if (Undo::isRedoAvailable()) col13 = ImVec4(1, 1, 1, 1);

		ImGui::SetNextWindowPos(pos);
		ImGui::SetNextWindowSize(size);

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 4));

		if (ImGui::Begin("Tools", &open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDocking))
		{
			if (ImGui::ImageButton((void*)icons[0]->getHandle().idx, ImVec2(16, 16), ImVec2(0, 1), ImVec2(1, 0), -1, ImVec4(0, 0, 0, 0), col14)) { addOnEndUpdateCallback([=]() { singleton->onSceneNew(); }); } // New
			ImGui::SameLine(); HelpMarker("New scene");

			if (ImGui::ImageButton((void*)icons[1]->getHandle().idx, ImVec2(16, 16), ImVec2(0, 1), ImVec2(1, 0), -1, ImVec4(0, 0, 0, 0), col14)) { addOnEndUpdateCallback([=]() { singleton->onSceneOpen(); }); } // Open
			ImGui::SameLine(); HelpMarker("Open scene");
			if (ImGui::ImageButton((void*)icons[2]->getHandle().idx, ImVec2(16, 16), ImVec2(0, 1), ImVec2(1, 0), -1, ImVec4(0, 0, 0, 0), col14)) { addOnEndUpdateCallback([=]() { singleton->onSceneSave(); }); } // Save
			ImGui::SameLine(); HelpMarker("Save scene");

			ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
			ImGui::SameLine();

			std::string undoMenu = "Undo";
			std::string redoMenu = "Redo";

			if (Undo::isUndoAvailable())
				undoMenu += " \"" + Undo::getLastUndoName() + "\"";

			if (Undo::isRedoAvailable())
				redoMenu += " \"" + Undo::getLastRedoName() + "\"";

			if (ImGui::ImageButton((void*)icons[3]->getHandle().idx, ImVec2(16, 16), ImVec2(0, 1), ImVec2(1, 0), -1, ImVec4(0, 0, 0, 0), col12)) { addOnEndUpdateCallback([=]() { singleton->onUndo(); }); } // Undo
			ImGui::SameLine(); HelpMarker(undoMenu.c_str());
			if (ImGui::ImageButton((void*)icons[4]->getHandle().idx, ImVec2(16, 16), ImVec2(0, 1), ImVec2(1, 0), -1, ImVec4(0, 0, 0, 0), col13)) { addOnEndUpdateCallback([=]() { singleton->onRedo(); }); } // Redo
			ImGui::SameLine(); HelpMarker(redoMenu.c_str());

			ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
			ImGui::SameLine();

			if (ImGui::ImageButton((void*)icons[5]->getHandle().idx, ImVec2(16, 16), ImVec2(0, 1), ImVec2(1, 0), -1, ImVec4(0, 0, 0, 0), col4)) { singleton->onGizmoSelect(); } // Select
			ImGui::SameLine(); HelpMarker("Operation: Select");
			if (ImGui::ImageButton((void*)icons[6]->getHandle().idx, ImVec2(16, 16), ImVec2(0, 1), ImVec2(1, 0), -1, ImVec4(0, 0, 0, 0), col5)) { singleton->onGizmoMove(); } // Move
			ImGui::SameLine(); HelpMarker("Operation: Move");
			if (ImGui::ImageButton((void*)icons[7]->getHandle().idx, ImVec2(16, 16), ImVec2(0, 1), ImVec2(1, 0), -1, ImVec4(0, 0, 0, 0), col6)) { singleton->onGizmoRotate(); } // Rotate
			ImGui::SameLine(); HelpMarker("Operation: Rotate");
			if (ImGui::ImageButton((void*)icons[8]->getHandle().idx, ImVec2(16, 16), ImVec2(0, 1), ImVec2(1, 0), -1, ImVec4(0, 0, 0, 0), col7)) { singleton->onGizmoScale(); } // Scale
			ImGui::SameLine(); HelpMarker("Operation: Scale");

			ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
			ImGui::SameLine();

			if (ImGui::ImageButton((void*)icons[13]->getHandle().idx, ImVec2(16, 16), ImVec2(0, 1), ImVec2(1, 0), -1, ImVec4(0, 0, 0, 0), col10)) { singleton->onGizmoCenter(); } // Center
			ImGui::SameLine(); HelpMarker("Pivot: Bounding box center");
			if (ImGui::ImageButton((void*)icons[14]->getHandle().idx, ImVec2(16, 16), ImVec2(0, 1), ImVec2(1, 0), -1, ImVec4(0, 0, 0, 0), col11)) { singleton->onGizmoPivot(); } // Pivot
			ImGui::SameLine(); HelpMarker("Pivot: Local pivot");

			ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
			ImGui::SameLine();

			if (ImGui::ImageButton((void*)icons[9]->getHandle().idx, ImVec2(16, 16), ImVec2(0, 1), ImVec2(1, 0), -1, ImVec4(0, 0, 0, 0), col8)) { singleton->onGizmoLocal(); } // Local
			ImGui::SameLine(); HelpMarker("Transform space: Local");
			if (ImGui::ImageButton((void*)icons[10]->getHandle().idx, ImVec2(16, 16), ImVec2(0, 1), ImVec2(1, 0), -1, ImVec4(0, 0, 0, 0), col9)) { singleton->onGizmoWorld(); } // World
			ImGui::SameLine(); HelpMarker("Transform space: World");

			ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
			ImGui::SameLine();

			if (ImGui::ImageButton((void*)icons[11]->getHandle().idx, ImVec2(16, 16), ImVec2(0, 1), ImVec2(1, 0), -1, ImVec4(0, 0, 0, 0), col3)) { singleton->onGizmoSnapToGrid(); } // Snap to grid
			ImGui::SameLine(); HelpMarker("Snap to grid");
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 7);
			if (ImGui::ImageButtonWithID((void*)icons[15]->getHandle().idx, ImVec2(8, 16), ImGui::GetCurrentWindow()->GetIDNoKeepAlive("magnetValues"), ImVec2(0, 1), ImVec2(1, 0)))
			{
				ImGui::OpenPopup("magnet_values_popup");
			}

			if (ImGui::BeginPopup("magnet_values_popup"))
			{
				Gizmo* gizmo = sceneWindow->getGizmo();
				float step1 = gizmo->getSnapToGridMoveSize();
				float step2 = gizmo->getSnapToGridRotateSize();
				float step3 = gizmo->getSnapToGridScaleSize();
				int uiStep1 = settings->snapToGridUIMoveSize;
				int uiStep2 = settings->snapToGridUIRotateSize;

				ImGui::Text("Position step");
				if (ImGui::InputFloat("##position_step", &step1, 0.125f, 0.25f, "%.3f"))
				{
					settings->snapToGridMoveSize = step1;
					settings->save();

					sceneWindow->getGizmo()->setSnapToGridMoveSize(step1);
				}

				ImGui::Text("Rotation step");
				if (ImGui::InputFloat("##rotation_step", &step2, 5.0f, 15.0f, "%.3f"))
				{
					settings->snapToGridRotateSize = step2;
					settings->save();

					sceneWindow->getGizmo()->setSnapToGridRotateSize(step2);
				}

				ImGui::Text("Scale step");
				if (ImGui::InputFloat("##scale_step", &step3, 0.1f, 0.2f, "%.3f"))
				{
					settings->snapToGridScaleSize = step3;
					settings->save();

					sceneWindow->getGizmo()->setSnapToGridScaleSize(step3);
				}

				ImGui::Separator();

				ImGui::Text("UI position/size step");
				if (ImGui::InputInt("##ui_position_step", &uiStep1, 1, 2))
				{
					settings->snapToGridUIMoveSize = uiStep1;
					settings->save();
				}

				ImGui::Text("UI rotation step");
				if (ImGui::InputInt("##ui_rotation_step", &uiStep2, 1, 2))
				{
					settings->snapToGridUIRotateSize = uiStep2;
					settings->save();
				}

				ImGui::EndPopup();
			}

			ImGui::SameLine();

			ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
			ImGui::SameLine();

			void* texPlayStop = (void*)icons[12]->getHandle().idx;
			if (isPlaying)
				texPlayStop = (void*)icons[16]->getHandle().idx;

			if (ImGui::ImageButton(texPlayStop, ImVec2(16, 16), ImVec2(0, 1), ImVec2(1, 0))) { singleton->onPlay(); } // Play or stop
			if (!isPlaying)
				HelpMarker("Play");
			else
				HelpMarker("Stop");

			ImGui::SameLine();
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 7);
			if (ImGui::ImageButtonWithID((void*)icons[15]->getHandle().idx, ImVec2(8, 16), ImGui::GetCurrentWindow()->GetIDNoKeepAlive("screenSizes"), ImVec2(0, 1), ImVec2(1, 0)))
			{
				ImGui::OpenPopup("screen_resolutions_popup");
			}

			if (ImGui::BeginPopup("screen_resolutions_popup"))
			{
				if (ImGui::MenuItem("Standalone", "", settings->standalone, !isPlaying))
				{
					settings->standalone = !settings->standalone;
					settings->save();
				}

				if (ImGui::MenuItem("Save on play", "", settings->saveOnPlay, !isPlaying))
				{
					settings->saveOnPlay = !settings->saveOnPlay;
					settings->save();
				}

				if (ImGui::BeginMenu("Screen size", settings->standalone && !isPlaying))
				{
					if (ImGui::MenuItem("Fullscreen", "", settings->runFullscreen, settings->standalone && !isPlaying))
					{
						settings->runFullscreen = !settings->runFullscreen;
						settings->save();
					}

					ImGui::Separator();

					for (auto it = screenSizes.begin(); it != screenSizes.end(); ++it)
					{
						std::string res = std::to_string(it->first) + "x" + std::to_string(it->second);

						if (ImGui::MenuItem(res.c_str(), "", settings->runScreenWidth == it->first && settings->runScreenHeight == it->second, settings->standalone && !isPlaying))
						{
							settings->runScreenWidth = it->first;
							settings->runScreenHeight = it->second;
							settings->save();
						}
					}

					ImGui::EndMenu();
				}

				ImGui::EndPopup();
			}

			ImGui::SameLine();
		}

		ImGui::End();

		ImGui::PopStyleVar();
	}

	void MainWindow::onPostRender()
	{
		camPos = sceneWindow->getCamera()->getGameObject()->getTransform()->getPosition();
		camRot = sceneWindow->getCamera()->getGameObject()->getTransform()->getRotation();

		if (onEndUpdateCallbacks.size() > 0)
		{
			std::vector<std::function<void()>> funcs = onEndUpdateCallbacks;
			onEndUpdateCallbacks.clear();

			for (auto it = funcs.begin(); it != funcs.end(); ++it)
				(*it)();

			funcs.clear();
		}

		if (!ImGui::GetIO().WantCaptureKeyboard && !ImGui::GetIO().WantTextInput)
		{
			if (!InputManager::getSingleton()->getMouseButton(0) && !InputManager::getSingleton()->getMouseButton(1))
			{
				if (InputManager::getSingleton()->getKey(SDL_SCANCODE_LCTRL) && !InputManager::getSingleton()->getKey(SDL_SCANCODE_LSHIFT))
				{
					if (InputManager::getSingleton()->getKeyDown(SDL_SCANCODE_N))
						onSceneNew();

					if (InputManager::getSingleton()->getKeyDown(SDL_SCANCODE_S))
						onSceneSave();

					if (InputManager::getSingleton()->getKeyDown(SDL_SCANCODE_O))
						onSceneOpen();

					if (InputManager::getSingleton()->getKeyDown(SDL_SCANCODE_Z))
						onUndo();

					if (InputManager::getSingleton()->getKeyDown(SDL_SCANCODE_Y))
						onRedo();
				}

				if (InputManager::getSingleton()->getKey(SDL_SCANCODE_LCTRL) && InputManager::getSingleton()->getKey(SDL_SCANCODE_LSHIFT))
				{
					if (InputManager::getSingleton()->getKeyDown(SDL_SCANCODE_S))
						onSceneSaveAs();

					if (InputManager::getSingleton()->getKeyDown(SDL_SCANCODE_Z))
						onRedo();

					if (InputManager::getSingleton()->getKeyDown(SDL_SCANCODE_C))
						onCopyRenderImage();
				}

				if (!InputManager::getSingleton()->getKey(SDL_SCANCODE_LCTRL) && !InputManager::getSingleton()->getKey(SDL_SCANCODE_LSHIFT))
				{
					if (InputManager::getSingleton()->getKeyDown(SDL_SCANCODE_F5))
						onPlay();
				}

				if (sceneWindow->isFocused() || hierarchyWindow->isFocused() || sceneWindow->isHovered())
				{
					if (!InputManager::getSingleton()->getKey(SDL_SCANCODE_LCTRL) && !InputManager::getSingleton()->getKey(SDL_SCANCODE_LSHIFT))
					{
						if (InputManager::getSingleton()->getKeyDown(SDL_SCANCODE_F))
						{
							Gizmo* gizmo = sceneWindow->gizmo;
							auto& vec = gizmo->getSelectedObjects();

							if (vec.size() > 0)
							{
								Transform* first = vec[0];

								AxisAlignedBox box;
								for (auto& node : vec)
									gizmo->getNodesBounds(node, box);

								glm::vec3 sz = box.getSize();
								float mx = 1.0f;
								if (sz.x != INFINITY)
									mx = std::max(mx, sz.x);
								if (sz.y != INFINITY)
									mx = std::max(mx, sz.y);
								if (sz.z != INFINITY)
									mx = std::max(mx, sz.z);

								glm::vec3 objsCenter = box.getCenter();

								Transform* camTrans = sceneWindow->renderCamera->getTransform();

								cameraTargetPosition = objsCenter + (-camTrans->getForward() * mx);
								focusCameraOnObject = true;
							}
						}
					}
				}
			}
		}

		if (focusCameraOnObject)
		{
			Transform* camTrans = sceneWindow->renderCamera->getTransform();
			glm::vec3 posInterpolate = Mathf::lerp(camTrans->getPosition(), cameraTargetPosition, Time::getDeltaTime() * 10.0f);

			if (glm::distance(camTrans->getPosition(), cameraTargetPosition) >= 0.1f)
				camTrans->setPosition(posInterpolate);
			else
				focusCameraOnObject = false;
		}
	}

	void MainWindow::updateTitle()
	{
		std::string renderer = bgfx::getRendererName(bgfx::getRendererType());
		std::string title = "";
		std::string changed = "";

		if (lastUndoPos != Undo::getCurrentPosition())
			changed = " *";

		if (!openedSceneName.empty())
			title = "Falco Engine - " + openedSceneName + changed + " [" + renderer + "]";
		else
			title = "Falco Engine - New Scene.scene" + changed + " [" + renderer + "]";

		SDL_SetWindowTitle((SDL_Window*)getSdlWindow(), title.c_str());
	}

	void MainWindow::loadLastScene()
	{
		if (!settings->lastOpenedScene.empty())
		{
			if (IO::FileExists(Engine::getSingleton()->getAssetsPath() + settings->lastOpenedScene))
				openScene(settings->lastOpenedScene);
			else
				Scene::clear();
		}
		else
			Scene::clear();
	}

	void MainWindow::saveLastScene()
	{
		settings->lastOpenedScene = openedScenePath;
		settings->save();
	}

	void MainWindow::openScene(std::string path)
	{
		Undo::clearUndo();
		lastUndoPos = Undo::getCurrentPosition();

		lightingWindow->onSceneLoaded();

		ObjectEditor::resetBufferObjects();

		inspectorWindow->setEditor(nullptr);
		sceneWindow->getGizmo()->clearSelection();
		hierarchyWindow->getTreeView()->clear();
		uiEditorWindow->clearSelection();

		std::string fileName = IO::GetFileNameWithExt(path);

		openedSceneName = fileName;
		openedScenePath = path;

		updateTitle();

		sceneLoadingProgressDlg->show();
		sceneLoadingProgressDlg->setTitle("Loading scene - " + path);
		Scene::load(Engine::getSingleton()->getAssetsPath(), path, [=](float progress, std::string status)
			{
				sceneLoadingProgressDlg->setStatusText(status);
				sceneLoadingProgressDlg->setProgress(progress);
			}
		);
		sceneLoadingProgressDlg->hide();
		hierarchyWindow->refreshHierarchy();

		saveLastScene();

		/*std::vector<Camera*>& cameras = Renderer::getSingleton()->getCameras();
		for (auto it = cameras.begin(); it != cameras.end(); ++it)
		{
			(*it)->setRenderTarget(sceneWindow->renderTarget);
		}*/
	}

	void MainWindow::saveScene(std::string path)
	{
		if (isPlaying)
			return;

		lastUndoPos = Undo::getCurrentPosition();

		std::string fileName = IO::GetFileNameWithExt(path);

		openedSceneName = fileName;
		openedScenePath = path;

		updateTitle();

		Scene::save(Engine::getSingleton()->getAssetsPath(), path);
		saveLastScene();

		Toast::showMessage("\"" + path + "\" saved", TI_SAVE);

		assetsWindow->reloadFiles();
	}

	bool MainWindow::checkSceneIsDirty()
	{
		bool ret = true;

		if (lastUndoPos != Undo::getCurrentPosition())
		{
#ifdef _WIN32
			int result = MessageBoxA(0, "Current scene has unsaved changes.\nDo you want to save current scene?", "Unsaved changes", MB_YESNOCANCEL | MB_ICONQUESTION);
			if (result == IDYES)
			{
				if (!Engine::getSingleton()->getIsRuntimeMode())
				{
					return onSceneSave();
				}
				else
				{
					MessageBoxA(0, "Can not save during play mode!", "Unsaved changes", MB_OK | MB_ICONERROR);
					return false;
				}
			}
			if (result == IDNO)
				return true;
			if (result == IDCANCEL)
				return false;
#else
			char filename[1024];
			FILE *f = popen("zenity --info --text 'Current scene has unsaved changes.\nDo you want to save current scene?' --title 'Unsaved changes' --no-wrap \
							--ok-label Yes \
							--extra-button No \
							--extra-button Cancel", "r");
			fgets(filename, 1024, f);
			int r = pclose(f);

			if (std::string(filename).rfind("No", 0) != std::string::npos)
				return true;
			else if (std::string(filename).rfind("Cancel") != std::string::npos)
				return false;
			else if (r != 0)
				return false;
			else
			{
				if (!Engine::getSingleton()->getIsRuntimeMode())
				{
					return onSceneSave();
				}
				else
				{
					FILE* f = popen("zenity --error --text 'Can not save during play mode!' --title 'Unsaved changes' --no-wrap \
							--ok-label OK", "r");
					int r = pclose(f);

					return false;
				}
			}
#endif
		}

		return ret;
	}

	void MainWindow::onSceneNew()
	{
		if (isPlaying)
			return;

		if (!checkSceneIsDirty())
			return;

		Undo::clearUndo();
		lastUndoPos = Undo::getCurrentPosition();

		openedSceneName = "";
		openedScenePath = "";

		updateTitle();

		inspectorWindow->setEditor(nullptr);
		sceneWindow->getGizmo()->clearSelection();
		hierarchyWindow->getTreeView()->clear();

		Scene::clear();

		Transform* camT = sceneWindow->getCamera()->getTransform();
		camT->setPosition(glm::vec3(0, 2, 0));
		camT->setRotation(glm::identity<glm::highp_quat>());
	}

	void MainWindow::onSceneOpen()
	{
		if (isPlaying)
			return;

		if (!checkSceneIsDirty())
			return;

	#ifdef _WIN32
		IFileDialog* pfd;
		if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd))))
		{
			DWORD dwOptions;
			if (SUCCEEDED(pfd->GetOptions(&dwOptions)))
			{
				pfd->SetOptions(dwOptions);
			}

			COMDLG_FILTERSPEC ComDlgFS[1] = { {L"Scene files", L"*.scene"} };
			pfd->SetFileTypes(1, ComDlgFS);

			if (SUCCEEDED(pfd->Show(NULL)))
			{
				IShellItem* psi;
				if (SUCCEEDED(pfd->GetResult(&psi)))
				{
					LPWSTR g_path = NULL;
					LPWSTR g_name = NULL;

					if (!SUCCEEDED(psi->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &g_path)))
					{
						MessageBox(NULL, "SIGDN_DESKTOPABSOLUTEPARSING failed", NULL, NULL);
					}
					else
					{
						if (!SUCCEEDED(psi->GetDisplayName(SIGDN_NORMALDISPLAY, &g_name)))
						{
							MessageBox(NULL, "SIGDN_NORMALDISPLAY failed", NULL, NULL);
						}
						else
						{
							std::string fileName = StringConvert::ws2s(g_name);
							std::string filePath = StringConvert::ws2s(g_path);

							filePath = IO::ReplaceBackSlashes(filePath);

							if (filePath.find(Engine::getSingleton()->getAssetsPath()) == std::string::npos)
							{
								MessageBoxA(0, "Select location inside project assets folder!", "Error", MB_OK | MB_ICONERROR);
							}
							else
							{
								filePath = IO::RemovePart(filePath, Engine::getSingleton()->getAssetsPath());

								addOnEndUpdateCallback([=]()
									{
										openScene(filePath);
									}
								);
							}
						}
					}

					CoTaskMemFree(g_path);
					CoTaskMemFree(g_name);

					psi->Release();
				}
			}
			pfd->Release();
		}
	#else
			char filename[1024];
			FILE *f = popen("zenity --file-selection --file-filter='Scene files | *.scene'", "r");
			fgets(filename, 1024, f);
			int r = pclose(f);

			if (r == 0)
			{
				std::string filePath = filename;
				int idx = filePath.find("\n");
				if (idx != std::string::npos)
					filePath = filePath.substr(0, idx);

				filePath = IO::ReplaceBackSlashes(filePath);

				if (filePath.find(Engine::getSingleton()->getAssetsPath()) == std::string::npos)
				{
					FILE *ff = popen("zenity --icon-name='dialog-warning' --no-wrap --info --title='Error' --text='Select location inside project assets folder!'", "r");
					pclose(ff);
				}
				else
				{
					filePath = IO::RemovePart(filePath, Engine::getSingleton()->getAssetsPath());

					addOnEndUpdateCallback([=]()
						{
							openScene(filePath);
						}
					);
				}
			}
	#endif
	}

	bool MainWindow::onSceneSave()
	{
		if (isPlaying)
			return false;

		if (openedScenePath.empty())
		{
			return onSceneSaveAs();
		}
		else
		{
			saveScene(openedScenePath);
		}

		return true;
	}

	bool MainWindow::onSceneSaveAs()
	{
		if (isPlaying)
			return false;

		bool saved = false;

	#ifdef _WIN32
		IFileDialog* pfd;
		if (SUCCEEDED(CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd))))
		{
			DWORD dwOptions;
			if (SUCCEEDED(pfd->GetOptions(&dwOptions)))
			{
				pfd->SetOptions(dwOptions);
			}

			COMDLG_FILTERSPEC ComDlgFS[1] = { {L"Scene files", L"*.scene"} };
			pfd->SetFileTypes(1, ComDlgFS);

			if (SUCCEEDED(pfd->Show(NULL)))
			{
				IShellItem* psi;
				if (SUCCEEDED(pfd->GetResult(&psi)))
				{
					LPWSTR g_path = NULL;
					LPWSTR g_name = NULL;

					if (!SUCCEEDED(psi->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &g_path)))
					{
						MessageBox(NULL, "SIGDN_DESKTOPABSOLUTEPARSING failed", NULL, NULL);
					}
					else
					{
						if (!SUCCEEDED(psi->GetDisplayName(SIGDN_NORMALDISPLAY, &g_name)))
						{
							MessageBox(NULL, "SIGDN_NORMALDISPLAY failed", NULL, NULL);
						}
						else
						{
							std::string fileName = StringConvert::ws2s(g_name);
							std::string filePath = StringConvert::ws2s(g_path);

							filePath = IO::ReplaceBackSlashes(filePath);

							if (filePath.find(".scene") == std::string::npos)
								filePath += ".scene";

							if (filePath.find(Engine::getSingleton()->getAssetsPath()) == std::string::npos)
							{
								MessageBoxA(0, "Select location inside project assets folder!", "Error", MB_OK | MB_ICONERROR);
							}
							else
							{
								filePath = IO::RemovePart(filePath, Engine::getSingleton()->getAssetsPath());

								//addOnEndUpdateCallback([=]()
								//	{
								saveScene(filePath);
								saved = true;
								//	}
								//);
							}
						}
					}

					CoTaskMemFree(g_path);
					CoTaskMemFree(g_name);

					psi->Release();
				}
			}
			pfd->Release();
		}
	#else
			char filename[1024];
			FILE *f = popen("zenity --file-selection --save --file-filter='Scene files | *.scene'", "r");
			fgets(filename, 1024, f);
			int r = pclose(f);

			if (r == 0)
			{
				std::string filePath = filename;
				int idx = filePath.find("\n");
				if (idx != std::string::npos)
					filePath = filePath.substr(0, idx);

				filePath = IO::ReplaceBackSlashes(filePath);

				if (filePath.find(".scene") == std::string::npos)
					filePath += ".scene";

				if (filePath.find(Engine::getSingleton()->getAssetsPath()) == std::string::npos)
				{
					FILE *ff = popen("zenity --icon-name='dialog-warning' --no-wrap --info --title='Error' --text='Select location inside project assets folder!'", "r");
					pclose(ff);
				}
				else
				{
					filePath = IO::RemovePart(filePath, Engine::getSingleton()->getAssetsPath());

					saveScene(filePath);
					saved = true;
				}
			}
	#endif

		return saved;
	}

	void MainWindow::onProjectOpen()
	{
		if (isPlaying)
			return;

		if (closeByUser())
		{
			std::string path = Helper::ExeName();
	#ifdef _WIN32
			::ShellExecute(NULL, LPCSTR("open"), LPCSTR(path.c_str()), NULL, NULL, SW_RESTORE);
	#else
			if (fork() == 0)
				system(path.c_str());
	#endif
		}
	}

	void MainWindow::onProjectSettings()
	{
		ProjectSettingsEditor* editor = new ProjectSettingsEditor();
		editor->init();
		inspectorWindow->setEditor(editor);
	}

	void MainWindow::onExit()
	{
		if (!checkSceneIsDirty())
			return;

		Engine::getSingleton()->quit();
	}

	bool MainWindow::onClose()
	{
		std::string path = Helper::ExePath() + "imgui.ini";
		ImGui::SaveIniSettingsToDisk(path.c_str());

		return checkSceneIsDirty();
	}

	void MainWindow::onUndo()
	{
		Undo::doUndo();
		updateTitle();
	}

	void MainWindow::onRedo()
	{
		Undo::doRedo();
		singleton->updateTitle();
	}

	void MainWindow::onDelete()
	{
		if (!sceneWindow->isFocused() &&
			!hierarchyWindow->isFocused() &&
			!uiEditorWindow->isFocused() &&
			!assetsWindow->isFocused())
			return;

		if (inspectorWindow->isFocused())
			return;

		if (animationEditorWindow->isFocused())
			return;

		std::vector<Transform*> selected = sceneWindow->gizmo->getSelectedObjects();
		std::vector<Transform*> del;

		sceneWindow->gizmo->selectObject(nullptr);
		uiEditorWindow->clearSelection();
		inspectorWindow->setEditor(nullptr);

		for (auto it = selected.begin(); it != selected.end(); ++it)
		{
			Transform* cur = *it;
			Transform* par = cur->getParent();
			bool found = false;
			while (!found && par != nullptr)
			{
				found = std::find(selected.begin(), selected.end(), par) != selected.end();
				par = par->getParent();
			}

			if (found) del.push_back(cur);
		}

		for (auto it = del.begin(); it != del.end(); ++it)
		{
			auto i = std::find(selected.begin(), selected.end(), *it);
			if (i != selected.end())
				selected.erase(i);
		}

		del.clear();

		std::vector<GameObject*> delObjs;
		for (auto& s : selected)
			delObjs.push_back(s->getGameObject());

		deleteGameObjectUndo(delObjs);
		delObjs.clear();

		TreeView* tv = hierarchyWindow->getTreeView();
		
		bool updBatches = false;
		bool updCsg = false;

		std::vector<Component*> comps;

		for (auto it = selected.begin(); it != selected.end(); ++it)
		{
			Transform* cur = *it;

			Transform* parent = cur->getParent();

			std::vector<Transform*> nstack;
			nstack.push_back(cur);

			while (nstack.size() > 0)
			{
				Transform* child = *nstack.begin();
				nstack.erase(nstack.begin());

				//
				GameObject* _obj = child->getGameObject();
				auto& comps = _obj->getComponents();
				for (auto& comp : comps)
					comp->onDetach();
				//

				int j = 0;
				auto& children = child->getChildren();
				for (auto it = children.begin(); it != children.end(); ++it, ++j)
				{
					Transform* ch = *it;
					nstack.insert(nstack.begin() + j, ch);
				}
			}

			if (parent != nullptr)
			{
				auto& children = parent->getChildren();
				auto it = std::find(children.begin(), children.end(), cur);
				if (it != children.end())
					children.erase(it);
			}
			else
			{
				auto& children = Engine::getSingleton()->getRootTransforms();
				auto it = std::find(children.begin(), children.end(), cur);
				if (it != children.end())
					children.erase(it);
			}

			TreeNode* nd = tv->getNodeByName(cur->getGameObject()->getGuid());
			if (nd != nullptr)
				nd->setVisible(false);

			if (!updBatches || !updCsg)
			{
				cur->iterateChildren([&updBatches, &updCsg, &comps](Transform* child) -> bool
					{
						if (child->getGameObject() != nullptr)
						{
							if (child->getGameObject()->getBatchingStatic())
								updBatches = true;

							CSGBrush* brush = (CSGBrush*)child->getGameObject()->getComponent(CSGBrush::COMPONENT_TYPE);
							CSGModel* model = (CSGModel*)child->getGameObject()->getComponent(CSGModel::COMPONENT_TYPE);
							if (brush != nullptr)
							{
								if (child->getGameObject()->getActive() && brush->getEnabled())
								{
									comps.push_back(brush);
									updCsg = true;
								}
							}
							else if (model != nullptr)
							{
								if (child->getGameObject()->getActive() && model->getEnabled())
								{
									comps.push_back(model);
									updCsg = true;
								}
							}
						}

						return true;
					}
				);
			}
		}

		if (updBatches)
			MainWindow::addOnEndUpdateCallback([=]() { BatchedGeometry::getSingleton()->rebuild(); });

		if (updCsg)
			MainWindow::addOnEndUpdateCallback([=]() { CSGGeometry::getSingleton()->rebuild(comps); });
	}

	void MainWindow::onDuplicate()
	{
		std::vector<Transform*> cloned;
		std::vector<GameObject*> clonedObjs;
		std::vector<Transform*>& selected = sceneWindow->gizmo->getSelectedObjects();

		bool updBatches = false;
		bool updCsg = false;

		std::vector<Component*> comps;

		for (auto it = selected.begin(); it != selected.end(); ++it)
		{
			Transform* cur = *it;
			Transform* par = cur->getParent();
			bool found = false;
			while (!found && par != nullptr)
			{
				found = std::find(selected.begin(), selected.end(), par) != selected.end();
				par = par->getParent();
			}

			if (found) continue;

			GameObject* obj = cur->getGameObject();
			if (obj != nullptr)
			{
				GameObject* newObj = obj->clone();
				cloned.push_back(newObj->getTransform());
				clonedObjs.push_back(newObj);
			}

			if (!updBatches || !updCsg)
			{
				cur->iterateChildren([&updBatches, &updCsg, &comps](Transform* child) -> bool
					{
						if (child->getGameObject() != nullptr)
						{
							if (child->getGameObject()->getBatchingStatic())
								updBatches = true;

							CSGBrush* brush = (CSGBrush*)child->getGameObject()->getComponent(CSGBrush::COMPONENT_TYPE);
							CSGModel* model = (CSGModel*)child->getGameObject()->getComponent(CSGModel::COMPONENT_TYPE);
							if (brush != nullptr)
							{
								if (child->getGameObject()->getActive() && brush->getEnabled())
								{
									comps.push_back(brush);
									updCsg = true;
								}
							}
							else if (model != nullptr)
							{
								if (child->getGameObject()->getActive() && model->getEnabled())
								{
									comps.push_back(model);
									updCsg = true;
								}
							}
						}

						return true;
					}
				);
			}
		}

		for (auto it = cloned.begin(); it != cloned.end(); ++it)
			hierarchyWindow->insertNode(*it, (*it)->getParent());

		if (sceneWindow->gizmo->getSelectedObjects().size() > 0)
			sceneWindow->gizmo->selectObject(nullptr);

		addGameObjectUndo(clonedObjs, "Duplicate object(s)");

		sceneWindow->gizmo->selectObjects(cloned);
		cloned.clear();
		clonedObjs.clear();

		if (updBatches)
			MainWindow::addOnEndUpdateCallback([=]() { BatchedGeometry::getSingleton()->rebuild(); });

		if (updCsg)
			MainWindow::addOnEndUpdateCallback([=]() { CSGGeometry::getSingleton()->rebuild(comps); });
	}

	void MainWindow::onAlignWithView()
	{
		if (sceneWindow->gizmo->getSelectedObjects().size() == 0)
			return;

		Transform* node = sceneWindow->gizmo->getSelectedObjects()[0];

		if (node != nullptr)
		{
			glm::vec3 pos = sceneWindow->renderCamera->getGameObject()->getTransform()->getPosition();
			glm::highp_quat rot = sceneWindow->renderCamera->getGameObject()->getTransform()->getRotation();

			node->setPosition(pos);
			node->setRotation(rot);

			inspectorWindow->updateCurrentEditor();
		}
	}

	#ifdef _WIN32
	bool BitmapToClipboard(HBITMAP hBM, HWND hWnd)
	{
		OpenClipboard(NULL);

		DIBSECTION ds;
		::GetObject(hBM, sizeof(DIBSECTION), &ds);
		
		ds.dsBmih.biCompression = BI_RGB;
		HDC hdc = ::GetDC(NULL);
		HBITMAP hbitmap_ddb = ::CreateDIBitmap(
			hdc, &ds.dsBmih, CBM_INIT, ds.dsBm.bmBits, (BITMAPINFO*)&ds.dsBmih, DIB_RGB_COLORS);
		::ReleaseDC(NULL, hdc);

		EmptyClipboard();
		SetClipboardData(CF_BITMAP, hbitmap_ddb);
		CloseClipboard();

		return true;
	}

	HBITMAP TextureToBitmap(uint8_t* data, int width, int height)
	{
		HWND hwnd = MainWindow::getSingleton()->getHwnd();
		HDC hDC = GetDC(hwnd);
		HDC hDCMem = CreateCompatibleDC(hDC);

		unsigned char* lpBitmapBits;

		BITMAPINFO bi;
		ZeroMemory(&bi, sizeof(BITMAPINFO));
		bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bi.bmiHeader.biWidth = width;
		bi.bmiHeader.biHeight = height;
		bi.bmiHeader.biPlanes = 1;
		bi.bmiHeader.biBitCount = 32;

		HBITMAP bitmap = ::CreateDIBSection(hDCMem, &bi, DIB_RGB_COLORS, (VOID**)&lpBitmapBits, NULL, 0);

		BitBlt(hDCMem, 0, 0, width, height, hDC, 0, 0, SRCCOPY);

		int pitch = 4 * width; // 4 bytes per pixel but if not 32 bit, round pitch up to multiple of 4
		int index;
		for (int x = 0; x < width; x++)
		{
			for (int y = 0; y < height; y++)
			{
				index = y * pitch;
				index += x * 4;
				lpBitmapBits[index + 0] = data[index + 0];
				lpBitmapBits[index + 1] = data[index + 1];
				lpBitmapBits[index + 2] = data[index + 2]; 
			}
		}

		BitBlt(hDC, 0, 0, width, height, hDCMem, 0, 0, SRCCOPY);

		DeleteDC(hDCMem);

		return bitmap;
	}
	#endif

	void MainWindow::onCopyRenderImage()
	{
		//Get image
		const bgfx::Caps* caps = bgfx::getCaps();
		if ((BGFX_CAPS_TEXTURE_BLIT | BGFX_CAPS_TEXTURE_READ_BACK) != (caps->supported & (BGFX_CAPS_TEXTURE_BLIT | BGFX_CAPS_TEXTURE_READ_BACK)))
		{
			consoleWindow->log("Texture read back is not supported", LogMessageType::LMT_ERROR);
			return;
		}

		RenderTexture* rt = sceneWindow->renderTarget;

		int w = rt->getWidth();
		int h = rt->getHeight();
		
		bgfx::TextureHandle m_rb = bgfx::createTexture2D(w, h, false, 1, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_BLIT_DST | BGFX_TEXTURE_READ_BACK);
		bgfx::TextureHandle bb = rt->getColorTextureHandle();

		Renderer::getSingleton()->frame();

		int viewId = Renderer::getSingleton()->getNumViewsUsed() + 1;

		uint8_t* data = new uint8_t[w * h * 4];
		memset(data, 0, (w * h * 4) * sizeof(uint8_t));

		bgfx::blit(viewId, m_rb, 0, 0, bb);
		bgfx::readTexture(m_rb, data);

		Renderer::getSingleton()->frame();
		Renderer::getSingleton()->frame();

		bgfx::destroy(m_rb);

	#ifdef _WIN32
		//Copy to clipboard
		HBITMAP bmp = TextureToBitmap(data, w, h);

		delete[] data;

		BitmapToClipboard(bmp, getHwnd());
		DeleteObject(bmp);

		//Message
		Toast::showMessage("Render image copied to clipboard", ToastIcon::TI_CLIPBOARD_ADD);
	#else
		FIBITMAP* imagen = FreeImage_ConvertFromRawBits(data, w, h, w * 4, 32, 0xFF0000, 0x00FF00, 0x0000FF, false);

		std::string dirPath = Engine::getSingleton()->getRootPath() + "Temp/";

		std::string fileName = dirPath + "render_image.png";
		FreeImage_Save(FREE_IMAGE_FORMAT::FIF_PNG, imagen, fileName.c_str(), PNG_Z_NO_COMPRESSION);
		FreeImage_Unload(imagen);
		delete[] data;

		std::string command = "xclip -selection clip -t image/png \"" + fileName + "\"";
		int r = system(command.c_str());
		
		if (r != 0)
		{
			//Message
			Toast::showMessage("Render image saved to \"" + fileName + "\"", ToastIcon::TI_SAVE);
			Toast::showMessage("Install xclip to copy to clipboard", ToastIcon::TI_WARNING);
		}
		else
		{
			Toast::showMessage("Render image copied to clipboard", ToastIcon::TI_CLIPBOARD_ADD);
		}
	#endif
	}

	Canvas* findCanvas()
	{
		Canvas* canvas = nullptr;

		std::vector<GameObject*> objects = Engine::getSingleton()->getGameObjects();
		for (auto it = objects.begin(); it != objects.end(); ++it)
		{
			Component* comp = (*it)->getComponent(Canvas::COMPONENT_TYPE);
			if (comp != nullptr)
			{
				canvas = (Canvas*)comp;
				break;
			}
		}

		return canvas;
	}

	void MainWindow::onExportScene()
	{
		auto& allObjects = Engine::getSingleton()->getRootTransforms();
		std::vector<GameObject*> objects;
		for (auto obj : allObjects)
			objects.push_back(obj->getGameObject());

		exportObjects(objects);
	}

	void MainWindow::onExportSelected()
	{
		auto& allObjects = sceneWindow->gizmo->getSelectedObjects();
		std::vector<GameObject*> objects;
		for (auto obj : allObjects)
		{
			bool exclude = false;
			Transform* parent = obj->getParent();
			while (parent != nullptr)
			{
				if (std::find(allObjects.begin(), allObjects.end(), parent) != allObjects.end())
					exclude = true;

				if (exclude)
					break;

				parent = parent->getParent();
			}

			if (!exclude)
				objects.push_back(obj->getGameObject());
		}

		exportObjects(objects);
	}
	
	void MainWindow::exportObjects(std::vector<GameObject*>& objects)
	{
#ifdef _WIN32
		IFileDialog* pfd;
		if (SUCCEEDED(CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd))))
		{
			DWORD dwOptions;
			if (SUCCEEDED(pfd->GetOptions(&dwOptions)))
			{
				pfd->SetOptions(dwOptions);
			}

			COMDLG_FILTERSPEC ComDlgFS[2] = { {L"FBX", L"*.fbx"}, {L"OBJ", L"*.obj"}};
			pfd->SetFileTypes(2, ComDlgFS);

			if (SUCCEEDED(pfd->Show(NULL)))
			{
				IShellItem* psi;
				if (SUCCEEDED(pfd->GetResult(&psi)))
				{
					LPWSTR g_path = NULL;
					LPWSTR g_name = NULL;

					if (!SUCCEEDED(psi->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &g_path)))
					{
						MessageBox(NULL, "SIGDN_DESKTOPABSOLUTEPARSING failed", NULL, NULL);
					}
					else
					{
						if (!SUCCEEDED(psi->GetDisplayName(SIGDN_NORMALDISPLAY, &g_name)))
						{
							MessageBox(NULL, "SIGDN_NORMALDISPLAY failed", NULL, NULL);
						}
						else
						{
							std::string fileName = StringConvert::ws2s(g_name);
							std::string filePath = StringConvert::ws2s(g_path);

							filePath = IO::ReplaceBackSlashes(filePath);

							UINT fmt = 0;

							pfd->GetFileTypeIndex(&fmt);

							if (fmt == 1)
							{
								if (filePath.find(".fbx") == std::string::npos)
									filePath += ".fbx";
							}
							else if (fmt == 2)
							{
								if (filePath.find(".obj") == std::string::npos)
									filePath += ".obj";
							}

							Model3DLoader::exportObjects(filePath, objects, fmt - 1);
						}
					}

					CoTaskMemFree(g_path);
					CoTaskMemFree(g_name);

					psi->Release();
				}
			}
			pfd->Release();
		}
#else
		char filename[1024];
		FILE* f = popen("zenity --file-selection --save --file-filter='FBX | *.fbx' --file-filter='OBJ | *.obj'", "r");
		fgets(filename, 1024, f);
		int r = pclose(f);

		if (r == 0)
		{
			std::string filePath = filename;
			int idx = filePath.find("\n");
			if (idx != std::string::npos)
				filePath = filePath.substr(0, idx);

			filePath = IO::ReplaceBackSlashes(filePath);

			std::string ext = IO::GetFileExtension(filePath);
			int fmt = 0;
			if (ext == "fbx")
				fmt = 0;
			else if (ext == "obj")
				fmt = 1;

			Model3DLoader::exportObjects(filePath, objects, fmt);
		}
#endif
	}

	void MainWindow::addGameObjectUndo(std::vector<GameObject*> objects, std::string name)
	{
		//Undo
		UndoData* undoData = Undo::addUndo(name);
		undoData->objectData.resize(1);
		undoData->intData.resize(1);

		undoData->undoAction = [=](UndoData* data)
		{
			bool updBatches = false;
			bool updCsg = false;

			std::vector<Component*> comps;

			for (auto& d : data->objectData[0])
			{
				GameObject* obj = (GameObject*)d;
				Transform* trans = obj->getTransform();
				Transform* parent = trans->getParent();

				std::vector<Transform*> nstack;
				nstack.push_back(trans);

				while (nstack.size() > 0)
				{
					Transform* child = *nstack.begin();
					nstack.erase(nstack.begin());

					//
					GameObject* _obj = child->getGameObject();
					auto& comps = _obj->getComponents();
					for (auto& comp : comps)
						comp->onDetach();
					//

					int j = 0;
					auto& children = child->getChildren();
					for (auto it = children.begin(); it != children.end(); ++it, ++j)
					{
						Transform* ch = *it;
						nstack.insert(nstack.begin() + j, ch);
					}
				}

				if (parent != nullptr)
				{
					auto& children = parent->getChildren();
					auto it = std::find(children.begin(), children.end(), trans);
					if (it != children.end())
						children.erase(it);
				}
				else
				{
					auto& children = Engine::getSingleton()->getRootTransforms();
					auto it = std::find(children.begin(), children.end(), trans);
					if (it != children.end())
						children.erase(it);
				}

				TreeView* tv = hierarchyWindow->getTreeView();
				TreeNode* nd = tv->getNodeByName(obj->getGuid());
				if (nd != nullptr)
					nd->setVisible(false);

				if (!updBatches || !updCsg)
				{
					trans->iterateChildren([&updBatches, &updCsg, &comps](Transform* child) -> bool
						{
							if (child->getGameObject() != nullptr)
							{
								if (child->getGameObject()->getBatchingStatic())
									updBatches = true;

								CSGBrush* csgBrush = (CSGBrush*)child->getGameObject()->getComponent(CSGBrush::COMPONENT_TYPE);
								CSGModel* model = (CSGModel*)child->getGameObject()->getComponent(CSGModel::COMPONENT_TYPE);
								if (csgBrush != nullptr)
								{
									comps.push_back(csgBrush);
									updCsg = true;
								}
								else if (model != nullptr)
								{
									if (child->getGameObject()->getActive() && model->getEnabled())
									{
										comps.push_back(model);
										updCsg = true;
									}
								}
							}

							return true;
						}
					);
				}
			}

			Engine::getSingleton()->markGameObjectsOutdated();

			if (updBatches)
				MainWindow::addOnEndUpdateCallback([=]() { BatchedGeometry::getSingleton()->rebuild(); });

			if (updCsg)
				MainWindow::addOnEndUpdateCallback([=]() { CSGGeometry::getSingleton()->rebuild(comps); });
		};

		undoData->redoAction = [=](UndoData* data)
		{
			bool updBatches = false;
			bool updCsg = false;

			std::vector<Component*> comps;

			for (auto& d : data->objectData[0])
			{
				GameObject* obj = (GameObject*)d;
				Transform* trans = obj->getTransform();
				Transform* parent = trans->getParent();
				int idx = data->intData[0][obj];

				if (parent != nullptr)
				{
					auto& children = parent->getChildren();
					if (idx > children.size() - 1)
						idx = children.size() - 1;
					children.insert(children.begin() + idx, trans);
				}
				else
				{
					auto& children = Engine::getSingleton()->getRootTransforms();
					if (idx > children.size() - 1)
						idx = children.size() - 1;
					children.insert(children.begin() + idx, trans);
				}

				std::vector<Transform*> nstack;
				nstack.push_back(trans);

				while (nstack.size() > 0)
				{
					Transform* child = *nstack.begin();
					nstack.erase(nstack.begin());

					//
					GameObject* _obj = child->getGameObject();
					auto& comps = _obj->getComponents();
					for (auto& comp : comps)
						comp->onAttach();
					//

					int j = 0;
					auto& children = child->getChildren();
					for (auto it = children.begin(); it != children.end(); ++it, ++j)
					{
						Transform* ch = *it;
						nstack.insert(nstack.begin() + j, ch);
					}
				}

				TreeView* tv = hierarchyWindow->getTreeView();
				TreeNode* nd = tv->getNodeByName(obj->getGuid());
				if (nd != nullptr)
					nd->setVisible(true);

				if (!updBatches || !updCsg)
				{
					trans->iterateChildren([&updBatches, &updCsg, &comps](Transform* child) -> bool
						{
							if (child->getGameObject() != nullptr)
							{
								if (child->getGameObject()->getBatchingStatic())
									updBatches = true;

								CSGBrush* csgBrush = (CSGBrush*)child->getGameObject()->getComponent(CSGBrush::COMPONENT_TYPE);
								CSGModel* model = (CSGModel*)child->getGameObject()->getComponent(CSGModel::COMPONENT_TYPE);
								if (csgBrush != nullptr)
								{
									comps.push_back(csgBrush);
									updCsg = true;
								}
								else if (model != nullptr)
								{
									if (child->getGameObject()->getActive() && model->getEnabled())
									{
										comps.push_back(model);
										updCsg = true;
									}
								}
							}

							return true;
						}
					);
				}
			}

			Engine::getSingleton()->markGameObjectsOutdated();

			if (updBatches)
				MainWindow::addOnEndUpdateCallback([=]() { BatchedGeometry::getSingleton()->rebuild(); });

			if (updCsg)
				MainWindow::addOnEndUpdateCallback([=]() { CSGGeometry::getSingleton()->rebuild(comps); });
		};

		undoData->destroyAction = [=](UndoData* data)
		{
			std::vector<GameObject*> del;
			for (auto& d : data->objectData[0])
			{
				if (Undo::getObjectDeleted(d))
					continue;

				GameObject* obj = (GameObject*)d;
				Transform* trans = obj->getTransform();
				Transform* parent = trans->getParent();
				int idx = data->intData[0][obj];

				if (parent != nullptr)
				{
					auto& children = parent->getChildren();
					auto it = std::find(children.begin(), children.end(), trans);
					if (it == children.end())
					{
						children.insert(children.begin() + idx, trans);
						del.push_back(obj);
					}
				}
				else
				{
					auto& children = Engine::getSingleton()->getRootTransforms();
					auto it = std::find(children.begin(), children.end(), trans);
					if (it == children.end())
					{
						//children.insert(children.begin() + idx, trans);
						children.push_back(trans);
						del.push_back(obj);
					}
				}
			}

			TreeView* tv = hierarchyWindow->getTreeView();
			for (auto& d : del)
			{
				TreeNode* nd = tv->getNodeByName(d->getGuid());
				if (nd != nullptr)
					tv->deleteNode(nd);

				Engine::getSingleton()->destroyGameObject(d);
				Undo::setObjectDeleted(d, true);
			}

			del.clear();
		};
		//

		for (auto& obj : objects)
		{
			undoData->objectData[0].push_back(obj);

			Transform* trans = obj->getTransform();
			Transform* parent = trans->getParent();

			if (parent != nullptr)
			{
				auto& children = parent->getChildren();
				auto it = std::find(children.begin(), children.end(), trans);
				undoData->intData[0][obj] = std::distance(children.begin(), it);
			}
			else
			{
				auto& children = Engine::getSingleton()->getRootTransforms();
				auto it = std::find(children.begin(), children.end(), trans);
				undoData->intData[0][obj] = std::distance(children.begin(), it);
			}
		}

		std::sort(undoData->objectData[0].begin(), undoData->objectData[0].end(), [=](void* a, void* b) -> bool
			{
				return undoData->intData[0][a] < undoData->intData[0][b];
			}
		);

		Engine::getSingleton()->markGameObjectsOutdated();
	}

	void MainWindow::deleteGameObjectUndo(std::vector<GameObject*> objects)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Delete object(s)");
		undoData->objectData.resize(1);
		undoData->intData.resize(1);

		undoData->undoAction = [=](UndoData* data)
		{
			bool updBatches = false;
			bool updCsg = false;

			std::vector<Component*> comps;

			for (auto& d : data->objectData[0])
			{
				GameObject* obj = (GameObject*)d;
				Transform* trans = obj->getTransform();
				Transform* parent = trans->getParent();
				int idx = data->intData[0][obj];

				if (parent != nullptr)
				{
					auto& children = parent->getChildren();
					children.insert(children.begin() + idx, trans);
				}
				else
				{
					auto& children = Engine::getSingleton()->getRootTransforms();
					children.insert(children.begin() + idx, trans);
				}

				std::vector<Transform*> nstack;
				nstack.push_back(trans);

				while (nstack.size() > 0)
				{
					Transform* child = *nstack.begin();
					nstack.erase(nstack.begin());

					//
					GameObject* _obj = child->getGameObject();
					auto& comps = _obj->getComponents();
					for (auto& comp : comps)
						comp->onAttach();
					//

					int j = 0;
					auto& children = child->getChildren();
					for (auto it = children.begin(); it != children.end(); ++it, ++j)
					{
						Transform* ch = *it;
						nstack.insert(nstack.begin() + j, ch);
					}
				}

				TreeView* tv = hierarchyWindow->getTreeView();
				TreeNode* nd = tv->getNodeByName(obj->getGuid());
				if (nd != nullptr)
					nd->setVisible(true);

				if (!updBatches || !updCsg)
				{
					trans->iterateChildren([&updBatches, &updCsg, &comps](Transform* child) -> bool
						{
							if (child->getGameObject() != nullptr)
							{
								if (child->getGameObject()->getBatchingStatic())
									updBatches = true;

								CSGBrush* brush = (CSGBrush*)child->getGameObject()->getComponent(CSGBrush::COMPONENT_TYPE);
								CSGModel* model = (CSGModel*)child->getGameObject()->getComponent(CSGModel::COMPONENT_TYPE);
								if (brush != nullptr)
								{
									if (child->getGameObject()->getActive() && brush->getEnabled())
									{
										comps.push_back(brush);
										updCsg = true;
									}
								}
								else if (model != nullptr)
								{
									if (child->getGameObject()->getActive() && model->getEnabled())
									{
										comps.push_back(model);
										updCsg = true;
									}
								}
							}

							return true;
						}
					);
				}
			}

			Engine::getSingleton()->markGameObjectsOutdated();

			if (updBatches)
				MainWindow::addOnEndUpdateCallback([=]() { BatchedGeometry::getSingleton()->rebuild(); });

			if (updCsg)
				MainWindow::addOnEndUpdateCallback([=]() { CSGGeometry::getSingleton()->rebuild(comps); });
		};

		undoData->redoAction = [=](UndoData* data)
		{
			bool updBatches = false;
			bool updCsg = false;

			std::vector<Component*> comps;

			for (auto& d : data->objectData[0])
			{
				GameObject* obj = (GameObject*)d;
				Transform* trans = obj->getTransform();
				Transform* parent = trans->getParent();

				std::vector<Transform*> nstack;
				nstack.push_back(trans);

				while (nstack.size() > 0)
				{
					Transform* child = *nstack.begin();
					nstack.erase(nstack.begin());

					//
					GameObject* _obj = child->getGameObject();
					auto& comps = _obj->getComponents();
					for (auto& comp : comps)
						comp->onDetach();
					//

					int j = 0;
					auto& children = child->getChildren();
					for (auto it = children.begin(); it != children.end(); ++it, ++j)
					{
						Transform* ch = *it;
						nstack.insert(nstack.begin() + j, ch);
					}
				}

				if (parent != nullptr)
				{
					auto& children = parent->getChildren();
					auto it = std::find(children.begin(), children.end(), trans);
					if (it != children.end())
						children.erase(it);
				}
				else
				{
					auto& children = Engine::getSingleton()->getRootTransforms();
					auto it = std::find(children.begin(), children.end(), trans);
					if (it != children.end())
						children.erase(it);
				}

				TreeView* tv = hierarchyWindow->getTreeView();
				TreeNode* nd = tv->getNodeByName(obj->getGuid());
				if (nd != nullptr)
					nd->setVisible(false);

				if (!updBatches || !updCsg)
				{
					trans->iterateChildren([&updBatches, &updCsg, &comps](Transform* child) -> bool
						{
							if (child->getGameObject() != nullptr)
							{
								if (child->getGameObject()->getBatchingStatic())
									updBatches = true;

								CSGBrush* brush = (CSGBrush*)child->getGameObject()->getComponent(CSGBrush::COMPONENT_TYPE);
								CSGModel* model = (CSGModel*)child->getGameObject()->getComponent(CSGModel::COMPONENT_TYPE);
								if (brush != nullptr)
								{
									if (child->getGameObject()->getActive() && brush->getEnabled())
									{
										comps.push_back(brush);
										updCsg = true;
									}
								}
								else if (model != nullptr)
								{
									if (child->getGameObject()->getActive() && model->getEnabled())
									{
										comps.push_back(model);
										updCsg = true;
									}
								}
							}

							return true;
						}
					);
				}
			}

			Engine::getSingleton()->markGameObjectsOutdated();

			if (updBatches)
				MainWindow::addOnEndUpdateCallback([=]() { BatchedGeometry::getSingleton()->rebuild(); });

			if (updCsg)
				MainWindow::addOnEndUpdateCallback([=]() { CSGGeometry::getSingleton()->rebuild(comps); });
		};

		undoData->destroyAction = [=](UndoData* data)
		{
			std::vector<GameObject*> del;
			for (auto& d : data->objectData[0])
			{
				if (Undo::getObjectDeleted(d))
					continue;

				GameObject* obj = (GameObject*)d;
				Transform* trans = obj->getTransform();
				Transform* parent = trans->getParent();
				int idx = data->intData[0][obj];

				if (parent != nullptr)
				{
					auto& children = parent->getChildren();
					auto it = std::find(children.begin(), children.end(), trans);
					if (it == children.end())
					{
						children.insert(children.begin() + idx, trans);
						del.push_back(obj);
					}
				}
				else
				{
					auto& children = Engine::getSingleton()->getRootTransforms();
					auto it = std::find(children.begin(), children.end(), trans);
					if (it == children.end())
					{
						children.insert(children.begin() + idx, trans);
						del.push_back(obj);
					}
				}
			}

			TreeView* tv = hierarchyWindow->getTreeView();
			for (auto& d : del)
			{
				TreeNode* nd = tv->getNodeByName(d->getGuid());
				if (nd != nullptr)
					tv->deleteNode(nd);

				Engine::getSingleton()->destroyGameObject(d);
				Undo::setObjectDeleted(d, true);
			}

			del.clear();
		};
		//

		for (auto& obj : objects)
		{
			undoData->objectData[0].push_back(obj);

			Transform* trans = obj->getTransform();
			Transform* parent = trans->getParent();

			if (parent != nullptr)
			{
				auto& children = parent->getChildren();
				auto it = std::find(children.begin(), children.end(), trans);
				undoData->intData[0][obj] = std::distance(children.begin(), it);
			}
			else
			{
				auto& children = Engine::getSingleton()->getRootTransforms();
				auto it = std::find(children.begin(), children.end(), trans);
				undoData->intData[0][obj] = std::distance(children.begin(), it);
			}
		}

		std::sort(undoData->objectData[0].begin(), undoData->objectData[0].end(), [=](void* a, void* b) -> bool
			{
				return undoData->intData[0][a] < undoData->intData[0][b];
			}
		);

		Engine::getSingleton()->markGameObjectsOutdated();
	}

	void MainWindow::createPrimitive(std::string name, bool nested)
	{
		if (TerrainEditor::isEditModeActive())
			getInspectorWindow()->setEditor(nullptr);

		Gizmo* gizmo = sceneWindow->getGizmo();

		Transform* par = nullptr;
		if (nested)
		{
			auto& sel = gizmo->getSelectedObjects();
			if (sel.size() > 0)
				par = sel[0];
		}

		GameObject* obj = Model3DLoader::load3DModel(Engine::getSingleton()->getBuiltinResourcesPath(), name);
		
		Transform* child = obj->getTransform()->getChild(0);
		child->setParent(par);
		GameObject* obj1 = child->getGameObject();
		Engine::getSingleton()->destroyGameObject(obj);
		obj = obj1;

		MeshRenderer* rend = (MeshRenderer*)obj->getComponent(MeshRenderer::COMPONENT_TYPE);
		if (rend != nullptr)
		{
			if (rend->getSharedMaterialsCount() > 0)
				rend->setSharedMaterial(0, Renderer::getDefaultStandardMaterial());
		}

		if (par != nullptr)
		{
			obj->getTransform()->setLocalPosition(glm::vec3(0, 0, 0));
		}
		else
		{
			Camera* cam = sceneWindow->getCamera();
			Transform* camTransform = cam->getGameObject()->getTransform();
			obj->getTransform()->setPosition(camTransform->getPosition() + camTransform->getForward() * 7.0f);
		}

		hierarchyWindow->insertNode(obj->getTransform(), par);

		if (gizmo->getSelectedObjects().size() > 0)
			gizmo->selectObject(nullptr);

		addGameObjectUndo({ obj }, "Add primitive");

		gizmo->selectObject(obj->getTransform());
	}

	GameObject* MainWindow::createEmptyObject(std::string name, bool nested, bool addUndo)
	{
		if (TerrainEditor::isEditModeActive())
			getInspectorWindow()->setEditor(nullptr);

		Gizmo* gizmo = sceneWindow->getGizmo();

		Transform* par = nullptr;
		if (nested)
		{
			auto& sel = gizmo->getSelectedObjects();
			if (sel.size() > 0)
				par = sel[0];
		}

		GameObject* obj = Engine::getSingleton()->createGameObject();
		obj->setName(name);
		if (par != nullptr)
		{
			obj->getTransform()->setParent(par);
			obj->getTransform()->setLocalPosition(glm::vec3(0, 0, 0));
		}
		else
		{
			Camera* cam = sceneWindow->getCamera();
			Transform* camTransform = cam->getGameObject()->getTransform();
			obj->getTransform()->setPosition(camTransform->getPosition() + camTransform->getForward() * 7.0f);
		}

		hierarchyWindow->insertNode(obj->getTransform(), par);

		if (gizmo->getSelectedObjects().size() > 0)
			gizmo->selectObject(nullptr);

		if (addUndo)
		{
			addGameObjectUndo({ obj }, "Add object");
			gizmo->selectObject(obj->getTransform());
		}

		return obj;
	}

	GameObject* MainWindow::createUIObject(std::string name, bool nested)
	{
		if (TerrainEditor::isEditModeActive())
			getInspectorWindow()->setEditor(nullptr);

		Gizmo* gizmo = sceneWindow->getGizmo();

		Transform* par = nullptr;
		if (nested)
		{
			auto& sel = gizmo->getSelectedObjects();
			if (sel.size() > 0)
				par = sel[0];
		}

		GameObject* obj = Engine::getSingleton()->createGameObject();
		obj->setName(name);
		if (par != nullptr)
			obj->getTransform()->setParent(par);
		obj->getTransform()->setLocalPosition(glm::vec3(0, 0, 0));

		if (par == nullptr)
		{
			Canvas* canvas = findCanvas();
			Transform* canvasTransform = nullptr;
			if (canvas != nullptr)
			{
				glm::vec2 refSize = canvas->getRefScreenSize();
				canvasTransform = canvas->getGameObject()->getTransform();
				obj->getTransform()->setParent(canvasTransform);
				obj->getTransform()->setPosition(glm::vec3(refSize / 2.0f, 0.0f));
			}

			hierarchyWindow->insertNode(obj->getTransform(), canvasTransform);
		}
		else
		{
			hierarchyWindow->insertNode(obj->getTransform(), par);
		}

		if (gizmo->getSelectedObjects().size() > 0)
			gizmo->selectObject(nullptr);

		addGameObjectUndo({ obj }, "Add UI object");

		gizmo->selectObject(obj->getTransform());

		return obj;
	}

	void MainWindow::onCreateEmpty(bool nested)
	{
		createEmptyObject("GameObject", nested);
	}

	void MainWindow::onCreateAudioSource(bool nested)
	{
		createEmptyObject("AudioSource", nested);
		onAddAudioSource(false);
	}

	void MainWindow::onCreateAudioListener(bool nested)
	{
		createEmptyObject("AudioListener", nested);
		onAddAudioListener(false);
	}

	void MainWindow::onCreateDecal(bool nested)
	{
		createEmptyObject("Decal", nested);
		onAddDecalRenderer(false);
	}

	void MainWindow::onCreateCamera(bool nested)
	{
		createEmptyObject("Camera", nested);
		onAddCamera(false);
	}

	void MainWindow::onCreateTerrain(bool nested)
	{
		terrainCreate = true;
		terrainNested = nested;
		sceneWindow->gizmo->selectObject(nullptr, false, false);
		onAddTerrain(terrainCreate, nested);
	}

	void MainWindow::onCreatePointLight(bool nested)
	{
		GameObject* obj = createEmptyObject("Point Light", nested);
		onAddLight(false);

		((Light*)obj->getComponent(Light::COMPONENT_TYPE))->setLightType(LightType::Point);
	}

	void MainWindow::onCreateSpotLight(bool nested)
	{
		GameObject* obj = createEmptyObject("Spot Light", nested);
		onAddLight(false);

		((Light*)obj->getComponent(Light::COMPONENT_TYPE))->setLightType(LightType::Spot);
	}

	void MainWindow::onCreateDirectionalLight(bool nested)
	{
		GameObject* obj = createEmptyObject("Directional Light", nested);
		onAddLight(false);

		((Light*)obj->getComponent(Light::COMPONENT_TYPE))->setLightType(LightType::Directional);
	}

	void MainWindow::onCreateParticleSystem(bool nested)
	{
		createEmptyObject("Particle System", nested);
		onAddParticleSystem(false);
	}

	void MainWindow::onCreateWater(bool nested)
	{
		createEmptyObject("Water", nested);
		onAddWater(false);
	}

	void MainWindow::onCreateCanvas(bool nested)
	{
		createEmptyObject("Canvas", nested);
		onAddCanvas(false);
	}

	void MainWindow::onCreateImage(bool nested)
	{
		GameObject* obj = createUIObject("Image", nested);
		onAddImage(false);

		addOnEndUpdateCallback([=]()
			{
				uiEditorWindow->selectObject((Image*)obj->getComponent(Image::COMPONENT_TYPE), false, false);
			}
		);
	}

	void MainWindow::onCreateText(bool nested)
	{
		GameObject* obj = createUIObject("Text", nested);
		onAddText(false);

		addOnEndUpdateCallback([=]()
			{
				uiEditorWindow->selectObject((Text*)obj->getComponent(Text::COMPONENT_TYPE), false, false);
			}
		);
	}

	void MainWindow::onCreateButton(bool nested)
	{
		GameObject* obj = createUIObject("Button", nested);
		onAddButton(false);

		addOnEndUpdateCallback([=]()
			{
				uiEditorWindow->selectObject((Button*)obj->getComponent(Button::COMPONENT_TYPE), false, false);
			}
		);
	}

	void MainWindow::onCreateTextInput(bool nested)
	{
		GameObject* obj = createUIObject("Text Input", nested);
		onAddTextInput(false);

		addOnEndUpdateCallback([=]()
			{
				uiEditorWindow->selectObject((TextInput*)obj->getComponent(TextInput::COMPONENT_TYPE), false, false);
			}
		);
	}

	void MainWindow::onCreateMask(bool nested)
	{
		GameObject* obj = createUIObject("Mask", nested);
		onAddMask(false);

		addOnEndUpdateCallback([=]()
			{
				uiEditorWindow->selectObject((Mask*)obj->getComponent(Mask::COMPONENT_TYPE), false, false);
			}
		);
	}

	void MainWindow::onCreateSpline(bool nested)
	{
		GameObject* obj = createEmptyObject("Spline", nested);
		onAddSpline(false);
	}

	void MainWindow::onCreateVideoPlayer(bool nested)
	{
		GameObject* obj = createEmptyObject("Video Player", nested);
		onAddVideoPlayer(false);
	}

	void MainWindow::onCreateCSGModel(bool nested)
	{
		GameObject* obj = createEmptyObject("CSG Model", nested);
		onAddCSGModel(false);
	}

	void MainWindow::onCreateCSGBrushCube(bool nested)
	{
		GameObject* obj = createEmptyObject("Cube Brush", nested);
		onAddCSGBrush(false);
	}

	void MainWindow::onCreateCSGBrushSphere(bool nested)
	{
		GameObject* obj = createEmptyObject("Sphere Brush", nested);
		onAddCSGBrush(false);

		CSGBrush* brush = (CSGBrush*)obj->getComponent(CSGBrush::COMPONENT_TYPE);
		brush->setBrushType(CSGBrush::BrushType::Sphere);

		MainWindow::getHierarchyWindow()->updateNodeIcon(obj->getTransform());

		CSGGeometry::getSingleton()->rebuild(brush);
	}

	void MainWindow::onCreateCSGBrushCone(bool nested)
	{
		GameObject* obj = createEmptyObject("Cone Brush", nested);
		onAddCSGBrush(false);

		CSGBrush* brush = (CSGBrush*)obj->getComponent(CSGBrush::COMPONENT_TYPE);
		brush->setBrushType(CSGBrush::BrushType::Cone);

		MainWindow::getHierarchyWindow()->updateNodeIcon(obj->getTransform());

		CSGGeometry::getSingleton()->rebuild(brush);
	}

	void MainWindow::onCreateCSGBrushCylinder(bool nested)
	{
		GameObject* obj = createEmptyObject("Cylinder Brush", nested);
		onAddCSGBrush(false);

		CSGBrush* brush = (CSGBrush*)obj->getComponent(CSGBrush::COMPONENT_TYPE);
		brush->setBrushType(CSGBrush::BrushType::Cylinder);

		MainWindow::getHierarchyWindow()->updateNodeIcon(obj->getTransform());

		CSGGeometry::getSingleton()->rebuild(brush);
	}

	void MainWindow::onCreateCSGStair(bool nested)
	{
		GameObject* obj = createEmptyObject("Stair", nested, false);

		csgStairBuilder->show(obj);
	}

	void MainWindow::onCreateRagdoll()
	{
		ragdollEditor->show();
	}

	void MainWindow::onCreateCube(bool nested)
	{ 
		createPrimitive("Primitives/Cube.fbx", nested);
	}

	void MainWindow::onCreateSphere(bool nested)
	{
		createPrimitive("Primitives/Sphere.fbx", nested);
	}

	void MainWindow::onCreatePlane(bool nested)
	{
		createPrimitive("Primitives/Plane.fbx", nested);
	}

	void MainWindow::onCreateCapsule(bool nested)
	{
		createPrimitive("Primitives/Capsule.fbx", nested);
	}

	void MainWindow::onCreateCylinder(bool nested)
	{
		createPrimitive("Primitives/Cylinder.fbx", nested);
	}

	void MainWindow::onCreateTeapot(bool nested)
	{
		createPrimitive("Primitives/Teapot.fbx", nested);
	}

	void MainWindow::onCreateCone(bool nested)
	{
		createPrimitive("Primitives/Cone.fbx", nested);
	}

	void MainWindow::onCreatePyramid(bool nested)
	{
		createPrimitive("Primitives/Pyramid.fbx", nested);
	}

	void MainWindow::onCreateTorus(bool nested)
	{
		createPrimitive("Primitives/Torus.fbx", nested);
	}

	void MainWindow::onCreateTube(bool nested)
	{
		createPrimitive("Primitives/Tube.fbx", nested);
	}

	void MainWindow::addComponentUndo(std::vector<Component*> comps)
	{
		if (comps.size() == 0)
			return;

		//Undo
		UndoData* undoData = Undo::addUndo("Add " + comps[0]->getComponentType() + " component");
		undoData->objectData.resize(1);

		undoData->undoAction = [=](UndoData* data)
		{
			bool hasCsg = false;

			std::vector<Component*> comps;

			for (auto& d : data->objectData[0])
			{
				Component* comp = (Component*)d;
				GameObject* obj = comp->getGameObject();
				auto& comps = obj->getComponents();
				auto it = std::find(comps.begin(), comps.end(), comp);
				comps.erase(it);
				comp->onDetach();

				hierarchyWindow->updateNodeIcon(obj->getTransform());

				if (comp->getComponentType() == CSGBrush::COMPONENT_TYPE)
				{
					comps.push_back(comp);
					hasCsg = true;
				}
			}

			inspectorWindow->updateCurrentEditor();

			if (hasCsg)
				CSGGeometry::getSingleton()->rebuild(comps);
		};

		undoData->redoAction = [=](UndoData* data)
		{
			bool hasCsg = false;

			std::vector<Component*> comps;

			for (auto& d : data->objectData[0])
			{
				Component* comp = (Component*)d;
				GameObject* obj = comp->getGameObject();
				auto& comps = obj->getComponents();
				comps.push_back(comp);
				comp->onAttach();

				hierarchyWindow->updateNodeIcon(obj->getTransform());

				if (comp->getComponentType() == CSGBrush::COMPONENT_TYPE)
				{
					comps.push_back(comp);
					hasCsg = true;
				}
			}

			inspectorWindow->updateCurrentEditor();

			if (hasCsg)
				CSGGeometry::getSingleton()->rebuild(comps);
		};

		undoData->destroyAction = [=](UndoData* data)
		{
			for (auto& d : data->objectData[0])
			{
				if (Undo::getObjectDeleted(d))
					continue;

				Component* comp = (Component*)d;
				GameObject* obj = comp->getGameObject();
				auto& comps = obj->getComponents();
				auto it = std::find(comps.begin(), comps.end(), comp);
				if (it == comps.end())
				{
					comps.push_back(comp);
					obj->removeComponent(comp);
					Undo::setObjectDeleted(d, true);

					if (ObjectEditor::getBufferComponent() == comp)
						ObjectEditor::setBufferComponent(nullptr);
				}
			}
		};
		//

		for (auto& comp : comps)
		{
			undoData->objectData[0].push_back(comp);
		}
	}

	bool MainWindow::checkHasComponent(GameObject* obj, std::string compType, bool showMessage)
	{
		if (obj->getComponent(compType) != nullptr)
		{
			if (showMessage)
			{
	#ifdef _WIN32
				MessageBoxA(getHwnd(), "This component can not be added multiple times!", "Error", MB_OK | MB_ICONERROR);
	#else
				FILE *ff = popen("zenity --icon-name='dialog-warning' --no-wrap --info --title='Error' --text='This component can not be added multiple times!'", "r");
				pclose(ff);
	#endif
			}
			
			return true;
		}

		return false;
	}

	void MainWindow::onAddCamera(bool addUndo)
	{
		std::vector<Transform*>& sel = sceneWindow->getGizmo()->getSelectedObjects();
		std::vector<Component*> undoComps;

		int i = 0;
		for (auto it = sel.begin(); it != sel.end(); ++it, ++i)
		{
			GameObject* obj = (*it)->getGameObject();

			if (checkHasComponent(obj, Camera::COMPONENT_TYPE, sel.size() == 1))
				continue;

			Camera* component = new Camera();
			obj->addComponent(component);

			if (addUndo)
				undoComps.push_back(component);

			if (i == 0)
			{
				bool hasAL = false;
				std::vector<GameObject*> objs = Engine::getSingleton()->getGameObjects();
				for (auto gg = objs.begin(); gg != objs.end(); ++gg)
				{
					if ((*gg)->getComponent(AudioListener::COMPONENT_TYPE) != nullptr)
					{
						hasAL = true;
						break;
					}
				}

				if (!hasAL)
				{
					AudioListener* al = new AudioListener();
					obj->addComponent(al);

					if (addUndo)
						undoComps.push_back(al);
				}
			}

			hierarchyWindow->updateNodeIcon(obj->getTransform());
		}

		inspectorWindow->updateCurrentEditor();

		if (addUndo)
			addComponentUndo(undoComps);
	}

	void MainWindow::onAddMeshRenderer(bool addUndo)
	{
		std::vector<Transform*>& sel = sceneWindow->getGizmo()->getSelectedObjects();
		std::vector<Component*> undoComps;

		for (auto it = sel.begin(); it != sel.end(); ++it)
		{
			GameObject* obj = (*it)->getGameObject();

			if (checkHasComponent(obj, MeshRenderer::COMPONENT_TYPE, sel.size() == 1))
				continue;

			MeshRenderer* component = new MeshRenderer();
			obj->addComponent(component);

			if (addUndo)
				undoComps.push_back(component);

			hierarchyWindow->updateNodeIcon(obj->getTransform());
		}

		inspectorWindow->updateCurrentEditor();

		if (addUndo)
			addComponentUndo(undoComps);
	}

	void MainWindow::onAddDecalRenderer(bool addUndo)
	{
		std::vector<Transform*>& sel = sceneWindow->getGizmo()->getSelectedObjects();
		std::vector<Component*> undoComps;

		for (auto it = sel.begin(); it != sel.end(); ++it)
		{
			GameObject* obj = (*it)->getGameObject();

			if (checkHasComponent(obj, DecalRenderer::COMPONENT_TYPE, sel.size() == 1))
				continue;

			DecalRenderer* component = new DecalRenderer();
			obj->addComponent(component);

			if (addUndo)
				undoComps.push_back(component);

			hierarchyWindow->updateNodeIcon(obj->getTransform());
		}

		inspectorWindow->updateCurrentEditor();

		if (addUndo)
			addComponentUndo(undoComps);
	}

	void MainWindow::onAddLight(bool addUndo)
	{
		std::vector<Transform*>& sel = sceneWindow->getGizmo()->getSelectedObjects();
		std::vector<Component*> undoComps;

		for (auto it = sel.begin(); it != sel.end(); ++it)
		{
			GameObject* obj = (*it)->getGameObject();

			if (checkHasComponent(obj, Light::COMPONENT_TYPE, sel.size() == 1))
				continue;

			Light* component = new Light();
			obj->addComponent(component);

			if (addUndo)
				undoComps.push_back(component);

			hierarchyWindow->updateNodeIcon(obj->getTransform());
		}

		inspectorWindow->updateCurrentEditor();

		if (addUndo)
			addComponentUndo(undoComps);
	}

	void MainWindow::onAddAudioListener(bool addUndo)
	{
		std::vector<Transform*>& sel = sceneWindow->getGizmo()->getSelectedObjects();
		std::vector<Component*> undoComps;

		for (auto it = sel.begin(); it != sel.end(); ++it)
		{
			GameObject* obj = (*it)->getGameObject();

			if (checkHasComponent(obj, AudioListener::COMPONENT_TYPE, sel.size() == 1))
				continue;

			AudioListener* component = new AudioListener();
			obj->addComponent(component);

			if (addUndo)
				undoComps.push_back(component);

			hierarchyWindow->updateNodeIcon(obj->getTransform());
		}

		inspectorWindow->updateCurrentEditor();

		if (addUndo)
			addComponentUndo(undoComps);
	}

	void MainWindow::onAddAudioSource(bool addUndo)
	{
		std::vector<Transform*>& sel = sceneWindow->getGizmo()->getSelectedObjects();
		std::vector<Component*> undoComps;

		for (auto it = sel.begin(); it != sel.end(); ++it)
		{
			GameObject* obj = (*it)->getGameObject();

			AudioSource* component = new AudioSource();
			obj->addComponent(component);

			if (addUndo)
				undoComps.push_back(component);

			hierarchyWindow->updateNodeIcon(obj->getTransform());
		}

		inspectorWindow->updateCurrentEditor();

		if (addUndo)
			addComponentUndo(undoComps);
	}

	void MainWindow::onAddRigidbody(bool addUndo)
	{
		std::vector<Transform*>& sel = sceneWindow->getGizmo()->getSelectedObjects();
		std::vector<Component*> undoComps;

		for (auto it = sel.begin(); it != sel.end(); ++it)
		{
			GameObject* obj = (*it)->getGameObject();

			if (checkHasComponent(obj, RigidBody::COMPONENT_TYPE, sel.size() == 1))
				continue;

			RigidBody* component = new RigidBody();
			obj->addComponent(component);

			if (addUndo)
				undoComps.push_back(component);

			hierarchyWindow->updateNodeIcon(obj->getTransform());
		}

		inspectorWindow->updateCurrentEditor();

		if (addUndo)
			addComponentUndo(undoComps);
	}

	void MainWindow::onAddBoxCollider(bool addUndo)
	{
		std::vector<Transform*>& sel = sceneWindow->getGizmo()->getSelectedObjects();
		std::vector<Component*> undoComps;

		for (auto it = sel.begin(); it != sel.end(); ++it)
		{
			GameObject* obj = (*it)->getGameObject();

			BoxCollider* component = new BoxCollider();
			obj->addComponent(component);

			if (addUndo)
				undoComps.push_back(component);

			MeshRenderer* rend = (MeshRenderer*)obj->getComponent(MeshRenderer::COMPONENT_TYPE);
			if (rend != nullptr)
			{
				AxisAlignedBox aab = rend->getBounds(false);

				component->setBoxSize(aab.getHalfSize());
				component->setOffset(aab.getCenter());
			}

			hierarchyWindow->updateNodeIcon(obj->getTransform());
		}

		inspectorWindow->updateCurrentEditor();

		if (addUndo)
			addComponentUndo(undoComps);
	}

	void MainWindow::onAddSphereCollider(bool addUndo)
	{
		std::vector<Transform*>& sel = sceneWindow->getGizmo()->getSelectedObjects();
		std::vector<Component*> undoComps;

		for (auto it = sel.begin(); it != sel.end(); ++it)
		{
			GameObject* obj = (*it)->getGameObject();

			SphereCollider* component = new SphereCollider();
			obj->addComponent(component);

			if (addUndo)
				undoComps.push_back(component);

			MeshRenderer* rend = (MeshRenderer*)obj->getComponent(MeshRenderer::COMPONENT_TYPE);
			if (rend != nullptr)
			{
				AxisAlignedBox aab = rend->getBounds(false);

				component->setRadius(aab.getRadius());
				component->setOffset(aab.getCenter());
			}

			hierarchyWindow->updateNodeIcon(obj->getTransform());
		}

		inspectorWindow->updateCurrentEditor();

		if (addUndo)
			addComponentUndo(undoComps);
	}

	void MainWindow::onAddCapsuleCollider(bool addUndo)
	{
		std::vector<Transform*>& sel = sceneWindow->getGizmo()->getSelectedObjects();
		std::vector<Component*> undoComps;

		for (auto it = sel.begin(); it != sel.end(); ++it)
		{
			GameObject* obj = (*it)->getGameObject();

			CapsuleCollider* component = new CapsuleCollider();
			obj->addComponent(component);

			if (addUndo)
				undoComps.push_back(component);

			MeshRenderer* rend = (MeshRenderer*)obj->getComponent(MeshRenderer::COMPONENT_TYPE);
			if (rend != nullptr)
			{
				AxisAlignedBox aab = rend->getBounds(false);

				float height = aab.getSize().y;
				float radius = height * 0.25f;
				height -= radius * 2.0f;
				if (height < 0) height = 0;

				component->setRadius(radius);
				component->setHeight(height);
				component->setOffset(aab.getCenter());
			}

			hierarchyWindow->updateNodeIcon(obj->getTransform());
		}

		inspectorWindow->updateCurrentEditor();

		if (addUndo)
			addComponentUndo(undoComps);
	}

	void MainWindow::onAddMeshCollider(bool addUndo)
	{
		std::vector<Transform*>& sel = sceneWindow->getGizmo()->getSelectedObjects();
		std::vector<Component*> undoComps;

		for (auto it = sel.begin(); it != sel.end(); ++it)
		{
			GameObject* obj = (*it)->getGameObject();

			if (checkHasComponent(obj, MeshCollider::COMPONENT_TYPE, sel.size() == 1))
				continue;

			MeshCollider* component = new MeshCollider();
			obj->addComponent(component);

			if (addUndo)
				undoComps.push_back(component);

			hierarchyWindow->updateNodeIcon(obj->getTransform());
		}

		inspectorWindow->updateCurrentEditor();

		if (addUndo)
			addComponentUndo(undoComps);
	}

	void MainWindow::onAddTerrainCollider(bool addUndo)
	{
		std::vector<Transform*>& sel = sceneWindow->getGizmo()->getSelectedObjects();
		std::vector<Component*> undoComps;

		for (auto it = sel.begin(); it != sel.end(); ++it)
		{
			GameObject* obj = (*it)->getGameObject();

			if (checkHasComponent(obj, TerrainCollider::COMPONENT_TYPE, sel.size() == 1))
				continue;

			TerrainCollider* component = new TerrainCollider();
			obj->addComponent(component);

			if (addUndo)
				undoComps.push_back(component);

			hierarchyWindow->updateNodeIcon(obj->getTransform());
		}

		inspectorWindow->updateCurrentEditor();

		if (addUndo)
			addComponentUndo(undoComps);
	}

	void MainWindow::onAddFixedJoint(bool addUndo)
	{
		std::vector<Transform*>& sel = sceneWindow->getGizmo()->getSelectedObjects();
		std::vector<Component*> undoComps;

		for (auto it = sel.begin(); it != sel.end(); ++it)
		{
			GameObject* obj = (*it)->getGameObject();

			if (checkHasComponent(obj, FixedJoint::COMPONENT_TYPE, sel.size() == 1))
				continue;

			FixedJoint* component = new FixedJoint();
			obj->addComponent(component);

			if (addUndo)
				undoComps.push_back(component);

			hierarchyWindow->updateNodeIcon(obj->getTransform());
		}

		inspectorWindow->updateCurrentEditor();

		if (addUndo)
			addComponentUndo(undoComps);
	}

	void MainWindow::onAddHingeJoint(bool addUndo)
	{
		std::vector<Transform*>& sel = sceneWindow->getGizmo()->getSelectedObjects();
		std::vector<Component*> undoComps;

		for (auto it = sel.begin(); it != sel.end(); ++it)
		{
			GameObject* obj = (*it)->getGameObject();

			if (checkHasComponent(obj, HingeJoint::COMPONENT_TYPE, sel.size() == 1))
				continue;

			HingeJoint* component = new HingeJoint();
			obj->addComponent(component);

			if (addUndo)
				undoComps.push_back(component);

			hierarchyWindow->updateNodeIcon(obj->getTransform());
		}

		inspectorWindow->updateCurrentEditor();

		if (addUndo)
			addComponentUndo(undoComps);
	}

	void MainWindow::onAddConeTwistJoint(bool addUndo)
	{
		std::vector<Transform*>& sel = sceneWindow->getGizmo()->getSelectedObjects();
		std::vector<Component*> undoComps;

		for (auto it = sel.begin(); it != sel.end(); ++it)
		{
			GameObject* obj = (*it)->getGameObject();

			if (checkHasComponent(obj, ConeTwistJoint::COMPONENT_TYPE, sel.size() == 1))
				continue;

			ConeTwistJoint* component = new ConeTwistJoint();
			obj->addComponent(component);

			if (addUndo)
				undoComps.push_back(component);

			hierarchyWindow->updateNodeIcon(obj->getTransform());
		}

		inspectorWindow->updateCurrentEditor();

		if (addUndo)
			addComponentUndo(undoComps);
	}

	void MainWindow::onAddFreeJoint(bool addUndo)
	{
		std::vector<Transform*>& sel = sceneWindow->getGizmo()->getSelectedObjects();
		std::vector<Component*> undoComps;

		for (auto it = sel.begin(); it != sel.end(); ++it)
		{
			GameObject* obj = (*it)->getGameObject();

			if (checkHasComponent(obj, FreeJoint::COMPONENT_TYPE, sel.size() == 1))
				continue;

			FreeJoint* component = new FreeJoint();
			obj->addComponent(component);

			if (addUndo)
				undoComps.push_back(component);

			hierarchyWindow->updateNodeIcon(obj->getTransform());
		}

		inspectorWindow->updateCurrentEditor();

		if (addUndo)
			addComponentUndo(undoComps);
	}

	void MainWindow::onAddVehicle(bool addUndo)
	{
		std::vector<Transform*>& sel = sceneWindow->getGizmo()->getSelectedObjects();
		std::vector<Component*> undoComps;

		for (auto it = sel.begin(); it != sel.end(); ++it)
		{
			GameObject* obj = (*it)->getGameObject();

			if (checkHasComponent(obj, Vehicle::COMPONENT_TYPE, sel.size() == 1))
				continue;

			Vehicle* component = new Vehicle();
			obj->addComponent(component);

			if (addUndo)
				undoComps.push_back(component);

			hierarchyWindow->updateNodeIcon(obj->getTransform());
		}

		inspectorWindow->updateCurrentEditor();

		if (addUndo)
			addComponentUndo(undoComps);
	}

	void MainWindow::onAddNavMeshAgent(bool addUndo)
	{
		std::vector<Transform*>& sel = sceneWindow->getGizmo()->getSelectedObjects();
		std::vector<Component*> undoComps;

		for (auto it = sel.begin(); it != sel.end(); ++it)
		{
			GameObject* obj = (*it)->getGameObject();

			if (checkHasComponent(obj, NavMeshAgent::COMPONENT_TYPE, sel.size() == 1))
				continue;

			NavMeshAgent* component = new NavMeshAgent();
			obj->addComponent(component);

			if (addUndo)
				undoComps.push_back(component);

			hierarchyWindow->updateNodeIcon(obj->getTransform());
		}

		inspectorWindow->updateCurrentEditor();

		if (addUndo)
			addComponentUndo(undoComps);
	}

	void MainWindow::onAddNavMeshObstacle(bool addUndo)
	{
		std::vector<Transform*>& sel = sceneWindow->getGizmo()->getSelectedObjects();
		std::vector<Component*> undoComps;

		for (auto it = sel.begin(); it != sel.end(); ++it)
		{
			GameObject* obj = (*it)->getGameObject();

			NavMeshObstacle* component = new NavMeshObstacle();
			obj->addComponent(component);

			if (addUndo)
				undoComps.push_back(component);

			hierarchyWindow->updateNodeIcon(obj->getTransform());
		}

		inspectorWindow->updateCurrentEditor();

		if (addUndo)
			addComponentUndo(undoComps);
	}

	void MainWindow::onAddAnimationList(bool addUndo)
	{
		std::vector<Transform*>& sel = sceneWindow->getGizmo()->getSelectedObjects();
		std::vector<Component*> undoComps;

		for (auto it = sel.begin(); it != sel.end(); ++it)
		{
			GameObject* obj = (*it)->getGameObject();

			if (checkHasComponent(obj, Animation::COMPONENT_TYPE, sel.size() == 1))
				continue;

			Animation* component = new Animation();
			obj->addComponent(component);

			if (addUndo)
				undoComps.push_back(component);

			hierarchyWindow->updateNodeIcon(obj->getTransform());
		}

		inspectorWindow->updateCurrentEditor();

		if (addUndo)
			addComponentUndo(undoComps);
	}

	void MainWindow::onAddTerrain(bool create, bool nested)
	{
		std::vector<Transform*>& sel = sceneWindow->getGizmo()->getSelectedObjects();
		if (sel.size() == 0 && !create)
			return;

		if (terrainFilePath.empty())
		{
	#ifdef _WIN32
			IFileDialog* pfd;
			if (SUCCEEDED(CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd))))
			{
				DWORD dwOptions;
				if (SUCCEEDED(pfd->GetOptions(&dwOptions)))
				{
					pfd->SetOptions(dwOptions);
				}

				COMDLG_FILTERSPEC ComDlgFS[1] = { {L"Terrain files", L"*.terrain"} };
				pfd->SetFileTypes(1, ComDlgFS);

				if (SUCCEEDED(pfd->Show(NULL)))
				{
					IShellItem* psi;
					if (SUCCEEDED(pfd->GetResult(&psi)))
					{
						LPWSTR g_path = NULL;
						LPWSTR g_name = NULL;

						if (!SUCCEEDED(psi->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &g_path)))
						{
							MessageBox(NULL, "SIGDN_DESKTOPABSOLUTEPARSING failed", NULL, NULL);
						}
						else
						{
							if (!SUCCEEDED(psi->GetDisplayName(SIGDN_NORMALDISPLAY, &g_name)))
							{
								MessageBox(NULL, "SIGDN_NORMALDISPLAY failed", NULL, NULL);
							}
							else
							{
								std::string fileName = StringConvert::ws2s(g_name);
								std::string filePath = StringConvert::ws2s(g_path);

								filePath = IO::ReplaceBackSlashes(filePath);

								if (filePath.find(".terrain") == std::string::npos)
									filePath += ".terrain";

								terrainFilePath = filePath;
							}
						}

						CoTaskMemFree(g_path);
						CoTaskMemFree(g_name);

						psi->Release();
					}
				}
				pfd->Release();
			}
	#else
			char filename[1024];
			FILE *f = popen("zenity --file-selection --save --file-filter='Terrain files | *.terrain'", "r");
			fgets(filename, 1024, f);
			int r = pclose(f);

			if (r == 0)
			{
				std::string filePath = filename;
				int idx = filePath.find("\n");
				if (idx != std::string::npos)
					filePath = filePath.substr(0, idx);

				filePath = IO::ReplaceBackSlashes(filePath);

				if (filePath.find(".terrain") == std::string::npos)
					filePath += ".terrain";

				terrainFilePath = filePath;
			}
	#endif
		}

		if (terrainFilePath.empty())
		{
			terrainFilePath = "";
			terrainCreate = false;
			terrainNested = false;
			return;
		}

		if (terrainFilePath.find(Engine::getSingleton()->getAssetsPath()) == std::string::npos)
		{
	#ifdef _WIN32
			MessageBoxA(0, "Select location inside project assets folder!", "Error", MB_OK | MB_ICONERROR);
	#else
			FILE *ff = popen("zenity --icon-name='dialog-warning' --no-wrap --info --title='Error' --text='Select location inside project assets folder!'", "r");
			pclose(ff);
	#endif
			terrainFilePath = "";
			terrainCreate = false;
			terrainNested = false;
			return;
		}

		terrainFilePath = IO::RemovePart(terrainFilePath, Engine::getSingleton()->getAssetsPath());

		bool objectCreated = false;
		if (sel.size() == 0 || nested)
		{
			objectCreated = true;
			GameObject* obj = createEmptyObject("Terrain", nested);
		}

		std::vector<Component*> undoComps;

		Terrain* component = nullptr;
		for (auto it = sel.begin(); it != sel.end(); ++it)
		{
			GameObject* obj = (*it)->getGameObject();

			if (checkHasComponent(obj, Terrain::COMPONENT_TYPE, sel.size() == 1))
				continue;

			component = new Terrain();
			obj->addComponent(component);

			if (sel.size() > 0 && !nested)
				undoComps.push_back(component);

			hierarchyWindow->updateNodeIcon(obj->getTransform());
		}

		if (component != nullptr)
		{
			component->save(terrainFilePath);
			assetsWindow->reloadFiles();
		}

		inspectorWindow->updateCurrentEditor();

		terrainFilePath = "";
		terrainCreate = false;
		terrainNested = false;

		if (!objectCreated)
		{
			if (sel.size() > 0 && !nested)
				addComponentUndo(undoComps);
		}
	}

	void MainWindow::onAddParticleSystem(bool addUndo)
	{
		std::vector<Transform*>& sel = sceneWindow->getGizmo()->getSelectedObjects();
		std::vector<Component*> undoComps;

		for (auto it = sel.begin(); it != sel.end(); ++it)
		{
			GameObject* obj = (*it)->getGameObject();

			if (checkHasComponent(obj, ParticleSystem::COMPONENT_TYPE, sel.size() == 1))
				continue;

			ParticleSystem* component = new ParticleSystem();
			obj->addComponent(component);

			if (addUndo)
				undoComps.push_back(component);

			hierarchyWindow->updateNodeIcon(obj->getTransform());
		}

		inspectorWindow->updateCurrentEditor();

		if (addUndo)
			addComponentUndo(undoComps);
	}

	void MainWindow::onAddWater(bool addUndo)
	{
		std::vector<Transform*>& sel = sceneWindow->getGizmo()->getSelectedObjects();
		std::vector<Component*> undoComps;

		for (auto it = sel.begin(); it != sel.end(); ++it)
		{
			GameObject* obj = (*it)->getGameObject();

			if (checkHasComponent(obj, Water::COMPONENT_TYPE, sel.size() == 1))
				continue;

			Water* component = new Water();
			obj->addComponent(component);

			if (addUndo)
				undoComps.push_back(component);

			hierarchyWindow->updateNodeIcon(obj->getTransform());
		}

		inspectorWindow->updateCurrentEditor();

		if (addUndo)
			addComponentUndo(undoComps);
	}

	void MainWindow::onAddCanvas(bool addUndo)
	{
		std::vector<Transform*>& sel = sceneWindow->getGizmo()->getSelectedObjects();
		std::vector<Component*> undoComps;

		for (auto it = sel.begin(); it != sel.end(); ++it)
		{
			GameObject* obj = (*it)->getGameObject();

			if (checkHasComponent(obj, Canvas::COMPONENT_TYPE, sel.size() == 1))
				continue;

			Canvas* component = new Canvas();
			obj->addComponent(component);

			if (addUndo)
				undoComps.push_back(component);

			hierarchyWindow->updateNodeIcon(obj->getTransform());
		}

		inspectorWindow->updateCurrentEditor();

		if (addUndo)
			addComponentUndo(undoComps);
	}

	void MainWindow::onAddImage(bool addUndo)
	{
		std::vector<Transform*>& sel = sceneWindow->getGizmo()->getSelectedObjects();
		std::vector<Component*> undoComps;

		for (auto it = sel.begin(); it != sel.end(); ++it)
		{
			GameObject* obj = (*it)->getGameObject();

			if (checkHasComponent(obj, Image::COMPONENT_TYPE, sel.size() == 1))
				continue;

			Image* component = new Image();
			obj->addComponent(component);

			if (addUndo)
				undoComps.push_back(component);

			hierarchyWindow->updateNodeIcon(obj->getTransform());
		}

		inspectorWindow->updateCurrentEditor();

		if (addUndo)
			addComponentUndo(undoComps);
	}

	void MainWindow::onAddText(bool addUndo)
	{
		std::vector<Transform*>& sel = sceneWindow->getGizmo()->getSelectedObjects();
		std::vector<Component*> undoComps;

		for (auto it = sel.begin(); it != sel.end(); ++it)
		{
			GameObject* obj = (*it)->getGameObject();

			if (checkHasComponent(obj, Text::COMPONENT_TYPE, sel.size() == 1))
				continue;

			Text* component = new Text();
			component->setText("New Text");
			obj->addComponent(component);

			if (addUndo)
				undoComps.push_back(component);

			hierarchyWindow->updateNodeIcon(obj->getTransform());
		}

		inspectorWindow->updateCurrentEditor();

		if (addUndo)
			addComponentUndo(undoComps);
	}

	void MainWindow::onAddButton(bool addUndo)
	{
		std::vector<Transform*>& sel = sceneWindow->getGizmo()->getSelectedObjects();
		std::vector<Component*> undoComps;

		for (auto it = sel.begin(); it != sel.end(); ++it)
		{
			GameObject* obj = (*it)->getGameObject();

			if (checkHasComponent(obj, Button::COMPONENT_TYPE, sel.size() == 1))
				continue;

			Button* component = new Button();
			obj->addComponent(component);

			if (addUndo)
				undoComps.push_back(component);

			hierarchyWindow->updateNodeIcon(obj->getTransform());
		}

		inspectorWindow->updateCurrentEditor();

		if (addUndo)
			addComponentUndo(undoComps);
	}

	void MainWindow::onAddTextInput(bool addUndo)
	{
		std::vector<Transform*>& sel = sceneWindow->getGizmo()->getSelectedObjects();
		std::vector<Component*> undoComps;

		for (auto it = sel.begin(); it != sel.end(); ++it)
		{
			GameObject* obj = (*it)->getGameObject();

			if (checkHasComponent(obj, TextInput::COMPONENT_TYPE, sel.size() == 1))
				continue;

			TextInput* component = new TextInput();
			obj->addComponent(component);

			if (addUndo)
				undoComps.push_back(component);

			hierarchyWindow->updateNodeIcon(obj->getTransform());
		}

		inspectorWindow->updateCurrentEditor();

		if (addUndo)
			addComponentUndo(undoComps);
	}

	void MainWindow::onAddMask(bool addUndo)
	{
		std::vector<Transform*>& sel = sceneWindow->getGizmo()->getSelectedObjects();
		std::vector<Component*> undoComps;

		for (auto it = sel.begin(); it != sel.end(); ++it)
		{
			GameObject* obj = (*it)->getGameObject();

			if (checkHasComponent(obj, Mask::COMPONENT_TYPE, sel.size() == 1))
				continue;

			Mask* component = new Mask();
			obj->addComponent(component);

			if (addUndo)
				undoComps.push_back(component);

			hierarchyWindow->updateNodeIcon(obj->getTransform());
		}

		inspectorWindow->updateCurrentEditor();

		if (addUndo)
			addComponentUndo(undoComps);
	}

	void MainWindow::onAddSpline(bool addUndo)
	{
		std::vector<Transform*>& sel = sceneWindow->getGizmo()->getSelectedObjects();
		std::vector<Component*> undoComps;

		for (auto it = sel.begin(); it != sel.end(); ++it)
		{
			GameObject* obj = (*it)->getGameObject();

			if (checkHasComponent(obj, Spline::COMPONENT_TYPE, sel.size() == 1))
				continue;

			Spline* component = new Spline();
			obj->addComponent(component);

			if (addUndo)
				undoComps.push_back(component);

			hierarchyWindow->updateNodeIcon(obj->getTransform());
		}

		inspectorWindow->updateCurrentEditor();

		if (addUndo)
			addComponentUndo(undoComps);
	}

	void MainWindow::onAddVideoPlayer(bool addUndo)
	{
		std::vector<Transform*>& sel = sceneWindow->getGizmo()->getSelectedObjects();
		std::vector<Component*> undoComps;

		for (auto it = sel.begin(); it != sel.end(); ++it)
		{
			GameObject* obj = (*it)->getGameObject();

			if (checkHasComponent(obj, VideoPlayer::COMPONENT_TYPE, sel.size() == 1))
				continue;

			VideoPlayer* component = new VideoPlayer();
			obj->addComponent(component);

			if (addUndo)
				undoComps.push_back(component);

			hierarchyWindow->updateNodeIcon(obj->getTransform());
		}

		inspectorWindow->updateCurrentEditor();

		if (addUndo)
			addComponentUndo(undoComps);
	}

	void MainWindow::onAddCSGModel(bool addUndo)
	{
		std::vector<Transform*>& sel = sceneWindow->getGizmo()->getSelectedObjects();
		std::vector<Component*> undoComps;

		std::vector<Component*> comps;

		for (auto it = sel.begin(); it != sel.end(); ++it)
		{
			GameObject* obj = (*it)->getGameObject();

			if (checkHasComponent(obj, CSGModel::COMPONENT_TYPE, sel.size() == 1))
				continue;

			CSGModel* component = new CSGModel();
			obj->addComponent(component);

			comps.push_back(component);

			if (addUndo)
				undoComps.push_back(component);

			hierarchyWindow->updateNodeIcon(obj->getTransform());
		}

		inspectorWindow->updateCurrentEditor();

		if (addUndo)
			addComponentUndo(undoComps);

		CSGGeometry::getSingleton()->rebuild(comps);
	}

	void MainWindow::onAddCSGBrush(bool addUndo, int brushType)
	{
		std::vector<Transform*>& sel = sceneWindow->getGizmo()->getSelectedObjects();
		std::vector<Component*> undoComps;

		std::vector<Component*> comps;

		for (auto it = sel.begin(); it != sel.end(); ++it)
		{
			GameObject* obj = (*it)->getGameObject();

			if (checkHasComponent(obj, CSGBrush::COMPONENT_TYPE, sel.size() == 1))
				continue;

			CSGBrush* component = new CSGBrush();
			obj->addComponent(component);

			comps.push_back(component);

			component->setBrushType(static_cast<CSGBrush::BrushType>(brushType));

			if (addUndo)
				undoComps.push_back(component);

			hierarchyWindow->updateNodeIcon(obj->getTransform());
		}

		inspectorWindow->updateCurrentEditor();

		if (addUndo)
			addComponentUndo(undoComps);

		CSGGeometry::getSingleton()->rebuild(comps);
	}

	void MainWindow::onAddScript(std::string className, bool addUndo)
	{
		std::vector<Transform*>& sel = sceneWindow->getGizmo()->getSelectedObjects();
		std::vector<Component*> undoComps;

		for (auto it = sel.begin(); it != sel.end(); ++it)
		{
			GameObject* obj = (*it)->getGameObject();

			MonoScript* component = new MonoScript();
			component->setClassFromName(className);
			obj->addComponent(component);

			if (addUndo)
				undoComps.push_back(component);

			hierarchyWindow->updateNodeIcon(obj->getTransform());
		}

		inspectorWindow->updateCurrentEditor();

		if (addUndo)
			addComponentUndo(undoComps);
	}

	void MainWindow::onExportPackage()
	{
		assetExporter->show(DialogAssetExporter::Mode::Export);
	}

	void MainWindow::onImportPackage()
	{
		assetExporter->show(DialogAssetExporter::Mode::Import);
	}

	void MainWindow::onAbout()
	{
		aboutDialog->show();
	}

	void MainWindow::onGizmoSelect()
	{
		sceneWindow->getGizmo()->setGizmoType(Gizmo::GizmoType::GT_SELECT);

		settings->gizmoType = 0;
		settings->save();
	}

	void MainWindow::onGizmoMove()
	{
		sceneWindow->getGizmo()->setGizmoType(Gizmo::GizmoType::GT_MOVE);

		settings->gizmoType = 1;
		settings->save();
	}

	void MainWindow::onGizmoRotate()
	{
		sceneWindow->getGizmo()->setGizmoType(Gizmo::GizmoType::GT_ROTATE);

		settings->gizmoType = 2;
		settings->save();
	}

	void MainWindow::onGizmoScale()
	{
		sceneWindow->getGizmo()->setGizmoType(Gizmo::GizmoType::GT_SCALE);

		settings->gizmoType = 3;
		settings->save();
	}

	void MainWindow::onGizmoLocal()
	{
		sceneWindow->getGizmo()->setTransformSpace(Gizmo::TransformSpace::TS_LOCAL);

		settings->gizmoTransformSpace = 0;
		settings->save();
	}

	void MainWindow::onGizmoWorld()
	{
		sceneWindow->getGizmo()->setTransformSpace(Gizmo::TransformSpace::TS_WORLD);

		settings->gizmoTransformSpace = 1;
		settings->save();
	}

	void MainWindow::onGizmoSnapToGrid()
	{
		sceneWindow->getGizmo()->setSnapToGrid(!sceneWindow->getGizmo()->getSnapToGrid());

		settings->snapToGrid = sceneWindow->getGizmo()->getSnapToGrid();
		settings->save();
	}

	void MainWindow::onGizmoCenter()
	{
		sceneWindow->getGizmo()->setCenterBase(Gizmo::CenterBase::CB_CENTER);

		settings->gizmoCenterBase = 0;
		settings->save();
	}

	void MainWindow::onGizmoPivot()
	{
		sceneWindow->getGizmo()->setCenterBase(Gizmo::CenterBase::CB_PIVOT);

		settings->gizmoCenterBase = 1;
		settings->save();
	}

	void MainWindow::onPlay()
	{
		if (openedScenePath.empty())
		{
			if (!onSceneSaveAs())
			{
	#ifdef _WIN32
				MessageBoxA(0, "Save scene first!", "Error", MB_OK | MB_ICONERROR);
	#else
				FILE *ff = popen("zenity --icon-name='dialog-warning' --no-wrap --info --title='Error' --text='Save scene first!'", "r");
				pclose(ff);
	#endif
				return;
			}
		}

		addOnEndUpdateCallback([=]()
			{
				if (settings->standalone)
				{
					if (consoleWindow->getClearOnPlay())
						consoleWindow->clear();

					if (settings->saveOnPlay)
					{
						if (lastUndoPos != Undo::getCurrentPosition())
							onSceneSave();
					}

					std::string projectName = Engine::getSingleton()->getAppName();
					std::string projectDir = Engine::getSingleton()->getRootPath();

					std::string cmd = "\"" + projectDir + "\" \"Project/bin/Debug/\" \"" + openedScenePath + "\" useLibrary " +
						std::to_string(settings->runScreenWidth) + " " + std::to_string(settings->runScreenHeight) + " " + (settings->runFullscreen ? "true" : "false");

#ifdef _WIN32
					cmd = "\"" + Helper::ExePath() + "Player.exe\" " + cmd;
#else
					cmd = "\"" + Helper::ExePath() + "Player.app\" " + cmd;
#endif
					boost::process::ipstream pipe_stream;
					boost::process::child c(cmd, boost::process::std_out > pipe_stream);

					std::string line;

					consoleWindow->setScrollToBottom(true);

					while (pipe_stream && std::getline(pipe_stream, line) && !line.empty())
					{
						consoleWindow->log(CP_UNI(line));
						Renderer::getSingleton()->renderFrame();
						Renderer::getSingleton()->renderFrame();
					}

					c.wait();

					consoleWindow->setScrollToBottom(false);
				}
				else
				{
					if (!isPlaying)
					{
						if (settings->saveOnPlay)
						{
							if (lastUndoPos != Undo::getCurrentPosition())
								onSceneSave();
						}
						else
						{
							if (!checkSceneIsDirty())
								return;
						}

						if (consoleWindow->getClearOnPlay())
							consoleWindow->clear();

						//Switch state
						isPlaying = true;

						//Clear scene
						Engine::getSingleton()->clear();

						//Enable runtime mode
						Engine::getSingleton()->setIsRuntimeMode(true);

						//Reload assembly
						APIManager::getSingleton()->reload();

						//Reload scene
						loadLastScene();

						auto& cameras = Renderer::getSingleton()->getCameras();
						for (auto& cam : cameras)
						{
							if (cam != sceneWindow->getCamera())
								cam->setRenderTarget(gameWindow->renderTarget);
						}

						gameWindow->show(true);
						gameWindow->onResize();
					}
					else
					{
						//Switch state
						isPlaying = false;

						//Clear scene
						Engine::getSingleton()->clear();

						//Disable runtime mode
						Engine::getSingleton()->setIsRuntimeMode(false);

						Time::setTimeScale(1.0f);

						//Reload assembly
						APIManager::getSingleton()->reload();

						//Reload scene
						loadLastScene();

						//Unlock and show mouse cursor
						InputManager::getSingleton()->setCursorLocked(false);
						InputManager::getSingleton()->setCursorVisible(true);

						gameWindow->show(false);
					}
				}
			}
		);
	}
}