#include "SceneWindow.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <sstream>
#include <iomanip>

#include "MainWindow.h"
#include "HierarchyWindow.h"
#include "AssetsWindow.h"
#include "InspectorWindow.h"
#include "UIEditorWindow.h"
#include "GameWindow.h"

#include "../PropertyEditors/PropertyEditor.h"
#include "../PropertyEditors/ObjectEditor.h"
#include "../PropertyEditors/TerrainEditor.h"

#include "../Classes/TreeView.h"
#include "../Classes/TreeNode.h"
#include "../Classes/Undo.h"
#include "../Classes/Toast.h"
#include "../Serialization/EditorSettings.h"

#include "../Engine/Core/Engine.h"
#include "../Engine/Core/Time.h"
#include "../Engine/Renderer/Renderer.h"
#include "../Engine/Renderer/Primitives.h"
#include "../Engine/Renderer/RenderTexture.h"
#include "../Engine/Renderer/BatchedGeometry.h"
#include "../Engine/Renderer/CSGGeometry.h"
#include "../Engine/Math/Raycast.h"
#include "../Engine/Assets/Model3DLoader.h"
#include "../Engine/Core/InputManager.h"
#include "../Engine/Gizmo/Gizmo.h"
#include "../Engine/Classes/IO.h"
#include "../Engine/Classes/Helpers.h"
#include "../Engine/Classes/GUIDGenerator.h"
#include "../Engine/Assets/Scene.h"
#include "../Engine/Assets/Texture.h"
#include "../Engine/Assets/Material.h"
#include "../Engine/Assets/Shader.h"
#include "../Engine/Assets/Prefab.h"
#include "../Engine/Core/GameObject.h"
#include "../Engine/Components/Camera.h"
#include "../Engine/Components/Transform.h"
#include "../Engine/Components/Light.h"
#include "../Engine/Components/MeshRenderer.h"
#include "../Engine/Components/Terrain.h"
#include "../Engine/Components/CSGBrush.h"

#include <SDL2/SDL.h>

namespace GX
{
	int sceneIdHash = 0;
	UndoData* SceneWindow::undoData = nullptr;

	SceneWindow::SceneWindow()
	{
		gizmo = new Gizmo();

		sceneIdHash = ImHashStr("Scene");
	}

	SceneWindow::~SceneWindow()
	{
		delete renderTarget;
		delete cameraPreviewRenderTarget;
		delete gizmo;
	}

	void getLayers(GameObject* obj, std::map<GameObject*, int>& arr)
	{
		arr[obj] = obj->getLayer();

		std::vector<Transform*> nstack;
		std::vector<Transform*> children = obj->getTransform()->getChildren();
		for (auto it = children.begin(); it != children.end(); ++it)
			nstack.push_back(*it);

		while (nstack.size() > 0)
		{
			Transform* child = *nstack.begin();
			nstack.erase(nstack.begin());

			//Update state
			arr[child->getGameObject()] = child->getGameObject()->getLayer();
			//

			int j = 0;
			children = child->getChildren();
			for (auto it = children.begin(); it != children.end(); ++it, ++j)
			{
				Transform* ch = *it;
				nstack.insert(nstack.begin() + j, ch);
			}
		}
	}

	void setLayers(GameObject* obj, std::map<GameObject*, int>& arr)
	{
		obj->setLayer(arr[obj]);

		std::vector<Transform*> nstack;
		std::vector<Transform*> children = obj->getTransform()->getChildren();
		for (auto it = children.begin(); it != children.end(); ++it)
			nstack.push_back(*it);

		while (nstack.size() > 0)
		{
			Transform* child = *nstack.begin();
			nstack.erase(nstack.begin());

			//Update state
			child->getGameObject()->setLayer(arr[child->getGameObject()]);
			//

			int j = 0;
			children = child->getChildren();
			for (auto it = children.begin(); it != children.end(); ++it, ++j)
			{
				Transform* ch = *it;
				nstack.insert(nstack.begin() + j, ch);
			}
		}
	}

	void SceneWindow::init()
	{
		dndId = GUIDGenerator::genGuid();

		InputManager::getSingleton()->subscribeMouseDownEvent([=](InputManager::MouseButton mb, int x, int y)
			{
				if (!isHovered())
					return;

				mouseDown(x, y, static_cast<int>(mb));
			}
		);

		InputManager::getSingleton()->subscribeMouseUpEvent([=](InputManager::MouseButton mb, int x, int y)
			{
				mouseUp(x, y, static_cast<int>(mb));
			}
		);

		InputManager::getSingleton()->subscribeMouseMoveEvent([=](int x, int y)
			{
				mouseMove(x, y);
			}
		);

		InputManager::getSingleton()->subscribeMouseWheelEvent([=](int x, int y)
			{
				if (!isHovered())
					return;

				mouseWheel(x, y);
			}
		);

		cameraIcon = Texture::load(Helper::ExePath(), "Editor/Icons/Toolbar/camera.png", false, Texture::CompressionMethod::None, true);
		viewIcon = Texture::load(Helper::ExePath(), "Editor/Icons/Toolbar/view.png", false, Texture::CompressionMethod::None, true);
		gridIcon = Texture::load(Helper::ExePath(), "Editor/Icons/Toolbar/grid.png", false, Texture::CompressionMethod::None, true);
		navMeshIcon = Texture::load(Helper::ExePath(), "Editor/Icons/Toolbar/navmesh.png", false, Texture::CompressionMethod::None, true);
		shadowsIcon = Texture::load(Helper::ExePath(), "Editor/Icons/Toolbar/shadows.png", false, Texture::CompressionMethod::None, true);

		lightPointGizmoIcon = Texture::load(Helper::ExePath(), "Editor/Icons/Gizmo/point_light.png", false, Texture::CompressionMethod::None, true);
		lightSpotGizmoIcon = Texture::load(Helper::ExePath(), "Editor/Icons/Gizmo/spot_light.png", false, Texture::CompressionMethod::None, true);
		lightDirGizmoIcon = Texture::load(Helper::ExePath(), "Editor/Icons/Gizmo/dir_light.png", false, Texture::CompressionMethod::None, true);

		cameraGizmoIcon = Texture::load(Helper::ExePath(), "Editor/Icons/Gizmo/camera.png", false, Texture::CompressionMethod::None, true);

		int width = MainWindow::getSingleton()->getWidth();
		int height = MainWindow::getSingleton()->getHeight();

		renderTarget = new RenderTexture(width, height);
		cameraPreviewRenderTarget = new RenderTexture(512, 512, RenderTexture::TextureType::ColorOnly);

		GameObject* cameraObj = Engine::getSingleton()->createGameObject();
		cameraObj->getTransform()->setPosition(glm::vec3(0, 2, 0));
		cameraObj->setSerializable(false);
		renderCamera = new Camera();
		cameraObj->addComponent(renderCamera);
		renderCamera->setDepth(-1);
		renderCamera->setRenderTarget(renderTarget);
		renderCamera->setClearColor(Color(0.5f, 0.5f, 0.5f, 1.0f));
		renderCamera->setFOVy(75.0f);
		renderCamera->setIsEditorCamera(true);

		gizmo->init(renderCamera);
		gizmo->setSelectCallback([=](std::vector<Transform*>& argument0, std::vector<Transform*>& argument1, void* userData) { onGizmoSelect(argument0, argument1, userData); });
		gizmo->addManipulateCallback(onGizmoManipulate);
		gizmo->addManipulateStartCallback(onGizmoStartManipulate);
		gizmo->addManipulateEndCallback(onGizmoEndManipulate);

		loadSettings();
		auto& formats = Engine::getModel3dFileFormats();

		onMouseHover = [=]()
		{
			if (GImGui->DragDropPayload.Data != nullptr)
			{
				if (GImGui->DragDropPayload.IsDataType("DND_TreeView"))
				{
					TreeNode* move_from = (TreeNode*)GImGui->DragDropPayload.Data;
					std::string path = move_from->getPath();
					std::string ext = IO::GetFileExtension(path);

					if (std::find(formats.begin(), formats.end(), ext) != formats.end())
					{
						GameObject* obj = Model3DLoader::load3DModel(Engine::getSingleton()->getAssetsPath(), path);
						if (obj != nullptr)
						{
							layerMem.clear();
							obj->setLayer(31, true);
							dragObject = obj->getTransform();
						}
					}

					if (ext == "prefab")
					{
						Prefab* prefab = Prefab::load(Engine::getSingleton()->getAssetsPath(), path);
						GameObject* obj = Scene::loadPrefab(prefab, glm::vec3(0.0f), glm::identity<glm::quat>());
						if (obj != nullptr)
						{
							layerMem.clear();
							getLayers(obj, layerMem);
							obj->setLayer(31, true);
							dragObject = obj->getTransform();
						}
					}

					if (ext == "terrain")
					{
						layerMem.clear();
						GameObject* obj = Engine::getSingleton()->createGameObject();
						obj->setName(IO::GetFileName(move_from->alias));
						obj->setLayer(31, true);

						Terrain* terrain = new Terrain();
						obj->addComponent(terrain);
						terrain->load(path);

						dragObject = obj->getTransform();
					}
				}
			}
		};

		onMouseExit = [=]()
		{
			if (GImGui->DragDropPayload.Data != nullptr)
			{
				if (dragObject != nullptr)
				{
					Engine::getSingleton()->destroyGameObject(dragObject->getGameObject());
					dragObject = nullptr;
				}
			}
		};

		Renderer::getSingleton()->addRenderCallback([=](int viewId, int viewLayer, Camera* camera)
			{
				if (camera->getIsEditorCamera())
					drawGizmos(viewId, viewLayer, camera);
			}
		);
	}

	void SceneWindow::update()
	{
		bool lastHovered = false;

		if (opened)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
			bool begin = ImGui::Begin("Scene", &opened, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
			ImGui::PopStyleVar();

			if (begin)
			{
				focused = ImGui::IsWindowFocused();

				if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
				{
					ImGui::SetWindowFocus();
					ImGui::ClearActiveID();
				}

				position = glm::vec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y);
				size = glm::vec2(ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);

				float mx = ImGui::GetMousePos().x;
				float my = ImGui::GetMousePos().y;

				lastHovered = hovered;
				mouseOver = (mx > position.x && mx < position.x + size.x && my > position.y + getToolbarHeight() && my < position.y + size.y);
				
				float ww = renderTarget->getWidth();
				float hh = renderTarget->getHeight();
				
				renderCamera->setScreenOffsetLeft(1.0f / ww * position.x);
				renderCamera->setScreenOffsetTop(1.0f / hh * position.y);

				if (size != oldSize)
				{
					if (size.x > 0 && size.y > 0)
						onResize();

					oldSize = size;
				}

				ImVec2 sz = ImVec2(ImGui::GetWindowSize().x, ImGui::GetWindowSize().y - 22);

				if (sz.x > 0 && sz.y > 0)
				{
					if (bgfx::isValid(renderTarget->getColorTextureHandle()))
						ImGui::Image((void*)renderTarget->getColorTextureHandle().idx, sz, ImVec2(0, 1), ImVec2(1, 0));
				}
				
				hovered = mouseOver && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);

				if (!wasHovered && hovered && (ImGui::IsMouseDragging(0) || ImGui::IsMouseDragging(1)))
					wasHovered = hovered;

				if (!ImGui::IsMouseReleased(0))
				{
					if (lastHovered != hovered)
					{
						if (hovered)
						{
							if (onMouseHover != nullptr)
								onMouseHover();
						}
						else
						{
							if (onMouseExit != nullptr)
								onMouseExit();
						}
					}
				}
				else
				{
					const ImGuiPayload* payload = ImGui::GetDragDropPayload();
					if (payload != nullptr)
					{
						TreeNode* move_from = (TreeNode*)payload->Data;
						glm::vec2 point = glm::vec2(ImGui::GetMousePos().x - position.x, ImGui::GetMousePos().y - position.y);

						if (dragObject != nullptr)
							dropObject(dragObject->getGameObject());

						dragObject = nullptr;
					}
				}

				ImGuiID dnd = ImGui::GetCurrentWindow()->GetIDNoKeepAlive(dndId.c_str());
				if (ImGui::BeginDragDropTargetCustom(ImRect(position.x + 4, position.y + 2 + getToolbarHeight(), position.x + size.x - 4, position.y + size.y - 4), dnd))
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_TreeView", ImGuiDragDropFlags_SourceNoDisableHover))
					{
						
					}
					ImGui::EndDragDropTarget();
				}

				updateToolbar();

				ImVec2 dbgTextPos = ImVec2(5, 52);

				if (showStats)
				{
					int fps = Time::getFramesPerSecond();
					float frameTime = Renderer::getSingleton()->getFrameTime();
					float cpuTime = Renderer::getSingleton()->getCpuTime();
					float gpuTime = Renderer::getSingleton()->getGpuTime();
					uint32_t numDrawCalls = Renderer::getSingleton()->getNumDrawCalls();
					uint32_t numTriangles = Renderer::getSingleton()->getNumTriangles();
					int64_t gpuMemUsed = Renderer::getSingleton()->getGpuMemoryUsed();

					std::string frameTimeStr = std::to_string(frameTime).substr(0, std::to_string(frameTime).find(".") + 3 + 1);
					std::string cpuTimeStr = std::to_string(cpuTime).substr(0, std::to_string(cpuTime).find(".") + 3 + 1);
					std::string gpuTimeStr = std::to_string(gpuTime).substr(0, std::to_string(gpuTime).find(".") + 3 + 1);

					ImVec4 fpsColor = ImVec4(0, 1, 0, 1);
					if (fps >= 0 && fps < 30) fpsColor = ImVec4(1, 0, 0, 1);
					if (fps >= 30 && fps < 55) fpsColor = ImVec4(1, 1, 0, 1);

					int wh = 133;
					if (Camera::getDynamicResolution() < 1.0f)
						wh += 20;

					ImGui::SetCursorPos(ImVec2(5, 52));
					ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.15f, 0.15f, 0.15f, 0.8f));
					ImGui::BeginChild("Stats", ImVec2(200, wh), true);
					ImGui::PopStyleColor();
					ImGui::TextColored(fpsColor, ("FPS: " + std::to_string(fps)).c_str());
					ImGui::Text(("Frame time (ms): " + frameTimeStr).c_str());
					ImGui::Separator();
					ImGui::Text(("CPU time (ms): " + cpuTimeStr).c_str());
					ImGui::Text(("GPU time (ms): " + gpuTimeStr).c_str());
					ImGui::Separator();
					ImGui::Text(("Draw calls: " + std::to_string(numDrawCalls)).c_str());
					ImGui::Text(("Triangles: " + std::to_string(numTriangles)).c_str());
					ImGui::Separator();
					ImGui::Text(("GPU memory used: " + std::to_string(gpuMemUsed) + "MB").c_str());
					if (Camera::getDynamicResolution() < 1.0f)
					{
						ImGui::Separator();
						std::ostringstream streamObj2;
						streamObj2 << std::fixed;
						streamObj2 << std::setprecision(2);
						streamObj2 << Camera::getDynamicResolution();
						std::string strObj3 = streamObj2.str();
						ImGui::Text(("Dynamic resolution: " + strObj3).c_str());
					}
					ImGui::EndChild();

					ImGui::SameLine();
					dbgTextPos = ImVec2(215, 52);
				}

				if (BatchedGeometry::getSingleton()->needRebuild())
				{
					ImGui::SetCursorPos(dbgTextPos);
					ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.15f, 0.15f, 0.15f, 0.8f));
					ImGui::BeginChild("Warning Messages", ImVec2(260, 36), true);
					ImGui::PopStyleColor();
					ImGui::TextColored(ImVec4(1, 0, 0, 1), "Static geometry needs to be rebuilt");

					ImGui::SameLine();

					if (ImGui::Button("Rebuild"))
					{
						MainWindow::getSingleton()->addOnEndUpdateCallback([=]()
							{
								BatchedGeometry::getSingleton()->rebuild(true);
							}
						);
					}
					ImGui::EndChild();
				}

				glm::vec2 _position = glm::vec2(position.x, position.y + 22.0f);
				glm::vec2 _size = glm::vec2(size.x, size.y - 22.0f);

				gizmo->setViewportHovered(hovered);
				gizmo->setRect(Rect(_position.x, _position.y, _size.x, _size.y));
				gizmo->setClipRect(Rect(_position.x, _position.y + getToolbarHeight(), _size.x, _size.y - getToolbarHeight()));
				gizmo->beginFrame();
				gizmo->updateView();

				gizmo->setManipulatorActive(!TerrainEditor::isEditModeActive());
				if (!TerrainEditor::isEditModeActive())
				{
					if (gizmo->getSelectedObjects().size() > 0)
						gizmo->updateManipulator();
				}

				gizmo->updateViewManipulator();

				auto& sel = gizmo->getSelectedObjects();
				if (sel.size() == 1)
				{
					GameObject* obj = sel[0]->getGameObject();
					if (obj != nullptr)
					{
						Camera* cam = (Camera*)obj->getComponent(Camera::COMPONENT_TYPE);

						if (cam != nullptr)
						{
							RenderTexture* rt = cameraPreviewRenderTarget;

							if (!Engine::getSingleton()->getIsRuntimeMode())
							{
								if (cam->getBackBuffer() == nullptr)
								{
									cam->setRenderTarget(cameraPreviewRenderTarget);

									cam->initResources();
									cam->renderFrame();
									cam->destroyResources();

									cam->setRenderTarget(nullptr);
								}
							}
							else
							{
								rt = cam->getRenderTarget();
							}

							RenderTexture* _rt = renderTarget;
							if (cam->getRenderTarget() != nullptr)
								_rt = cam->getRenderTarget();

							float aspect = (float)_rt->getHeight() / (float)_rt->getWidth();
							float aspectW = (float)_rt->getWidth() / (float)_rt->getHeight();

							float camWidth = 300.0f;
							float camHeight = camWidth * aspect;

							if (camHeight > 400.0f)
							{
								camHeight = 400.0f;
								camWidth = camHeight * aspectW;
							}

							if (camWidth > 0 && camHeight > 0)
							{
								if (cameraPreviewRenderTarget->getWidth() != (int)camWidth || cameraPreviewRenderTarget->getHeight() != (int)camHeight)
									cameraPreviewRenderTarget->reset(camWidth, camHeight);
							}

							float imgWidth = camWidth;
							float imgHeight = camHeight;

							camWidth *= cam->getViewportWidth();
							camHeight *= cam->getViewportHeight();

							float offsetX = imgWidth * cam->getViewportLeft();
							float offsetY = imgHeight * cam->getViewportTop();

							if (camWidth > 0 && camHeight > 0)
							{
								ImVec2 cp = ImGui::GetCursorPos();
								ImGui::SetCursorPos(ImVec2(size.x - imgWidth - 10, size.y - imgHeight - 10));
								ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
								ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.2f, 0.2f, 0.2f, 0.8f));
								if (ImGui::BeginChild("##camera_preview", ImVec2(imgWidth, imgHeight), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
								{
									ImGui::SetCursorPos(ImVec2(offsetX, offsetY));
									if (ImGui::BeginChild("##camera_preview_mask", ImVec2(camWidth, camHeight), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
									{
										ImGui::SetCursorPos(ImVec2(-offsetX, -offsetY));
										ImGui::Image((void*)rt->getColorTextureHandle().idx, ImVec2(imgWidth, imgHeight), ImVec2(0, 1), ImVec2(1, 0));
									}
									ImGui::EndChild();
								}
								ImGui::EndChild();
								ImGui::PopStyleColor();
								ImGui::PopStyleVar();
								ImGui::SetCursorPos(cp);
							}
						}
					}
				}
			}
			else
			{
				gizmo->setViewportHovered(false);
			}

			ImGui::End();

			if (!ImGui::GetIO().WantCaptureKeyboard && !ImGui::GetIO().WantTextInput)
			{
				ctrlPressed = InputManager::getSingleton()->getKey(SDL_SCANCODE_LCTRL);
				shiftPressed = InputManager::getSingleton()->getKey(SDL_SCANCODE_LSHIFT);

				if (focused)
				{
					if (shiftPressed)
						cameraSpeed = cameraSpeedFast;
					else
						cameraSpeed = cameraSpeedNormal;

					if (rButtonDown)
					{
						float dt = Time::getDeltaTime();

						if (InputManager::getSingleton()->getKey(SDL_SCANCODE_W)) // W
						{
							renderCamera->getGameObject()->getTransform()->translate(glm::vec3(0, 0, 1) * cameraSpeed * dt);
							MainWindow::getSingleton()->focusCameraOnObject = false;
						}

						if (InputManager::getSingleton()->getKey(SDL_SCANCODE_S)) // S
						{
							renderCamera->getGameObject()->getTransform()->translate(-glm::vec3(0, 0, 1) * cameraSpeed * dt);
							MainWindow::getSingleton()->focusCameraOnObject = false;
						}

						if (InputManager::getSingleton()->getKey(SDL_SCANCODE_A)) // A
						{
							renderCamera->getGameObject()->getTransform()->translate(glm::vec3(1, 0, 0) * cameraSpeed * dt);
							MainWindow::getSingleton()->focusCameraOnObject = false;
						}

						if (InputManager::getSingleton()->getKey(SDL_SCANCODE_D)) // D
						{
							renderCamera->getGameObject()->getTransform()->translate(-glm::vec3(1, 0, 0) * cameraSpeed * dt);
							MainWindow::getSingleton()->focusCameraOnObject = false;
						}

						if (InputManager::getSingleton()->getKey(SDL_SCANCODE_Q)) // Q
						{
							renderCamera->getGameObject()->getTransform()->translate(-glm::vec3(0, 1, 0) * cameraSpeed * dt);
							MainWindow::getSingleton()->focusCameraOnObject = false;
						}

						if (InputManager::getSingleton()->getKey(SDL_SCANCODE_E)) // E
						{
							renderCamera->getGameObject()->getTransform()->translate(glm::vec3(0, 1, 0) * cameraSpeed * dt);
							MainWindow::getSingleton()->focusCameraOnObject = false;
						}
					}
				}
			}
		}
		else
		{
			mouseOver = false;
		}

		if (!ImGui::GetIO().WantCaptureKeyboard && !ImGui::GetIO().WantTextInput)
		{
			ctrlPressed = InputManager::getSingleton()->getKey(SDL_SCANCODE_LCTRL);
			shiftPressed = InputManager::getSingleton()->getKey(SDL_SCANCODE_LSHIFT);

			if (!rButtonDown)
			{
				if (InputManager::getSingleton()->getKeyDown(SDL_SCANCODE_Q))
					MainWindow::getSingleton()->onGizmoSelect();

				if (InputManager::getSingleton()->getKeyDown(SDL_SCANCODE_W))
					MainWindow::getSingleton()->onGizmoMove();

				if (InputManager::getSingleton()->getKeyDown(SDL_SCANCODE_E))
					MainWindow::getSingleton()->onGizmoRotate();

				if (InputManager::getSingleton()->getKeyDown(SDL_SCANCODE_R))
					MainWindow::getSingleton()->onGizmoScale();

				if (InputManager::getSingleton()->getKeyDown(SDL_SCANCODE_T))
				{
					if (gizmo->getTransformSpace() == Gizmo::TransformSpace::TS_LOCAL)
						MainWindow::getSingleton()->onGizmoWorld();
					else
						MainWindow::getSingleton()->onGizmoLocal();
				}
			}

			if (InputManager::getSingleton()->getKeyDown(SDL_SCANCODE_1))
			{
				if (cameraSpeedPreset != 0)
					Toast::showMessage("Camera speed preset 1", ToastIcon::TI_CAMERA);

				cameraSpeedNormal = cameraSpeed1;
				cameraSpeedFast = cameraSpeed1 * 2.0f;
				cameraSpeed = cameraSpeed1;
				cameraSpeedPreset = 0;

				saveSettings();
			}

			if (InputManager::getSingleton()->getKeyDown(SDL_SCANCODE_2))
			{
				if (cameraSpeedPreset != 1)
					Toast::showMessage("Camera speed preset 2", ToastIcon::TI_CAMERA);

				cameraSpeedNormal = cameraSpeed2;
				cameraSpeedFast = cameraSpeed2 * 2.0f;
				cameraSpeed = cameraSpeed2;
				cameraSpeedPreset = 1;

				saveSettings();
			}

			if (InputManager::getSingleton()->getKeyDown(SDL_SCANCODE_3))
			{
				if (cameraSpeedPreset != 2)
					Toast::showMessage("Camera speed preset 3", ToastIcon::TI_CAMERA);

				cameraSpeedNormal = cameraSpeed3;
				cameraSpeedFast = cameraSpeed3 * 2.0f;
				cameraSpeed = cameraSpeed3;
				cameraSpeedPreset = 2;

				saveSettings();
			}

			if (ctrlPressed && !shiftPressed)
			{
				if (InputManager::getSingleton()->getKeyDown(SDL_SCANCODE_D))
				{
					MainWindow::getSingleton()->onDuplicate();
				}
			}

			if (!ctrlPressed && !shiftPressed)
			{
				if (InputManager::getSingleton()->getKeyDown(SDL_SCANCODE_DELETE))
					MainWindow::getSingleton()->onDelete();
			}
		}
	}

	void SceneWindow::drawGizmos(int viewId, int viewLayer, Camera* camera)
	{
		//Gizmos
		std::vector<Light*>& lights = Renderer::getSingleton()->getLights();
		std::vector<Camera*>& cameras = Renderer::getSingleton()->getCameras();

		//----------------Render gizmos begin-----------------//

		for (auto it = lights.begin(); it != lights.end(); ++it)
		{
			Light* light = *it;

			uint64_t state = BGFX_STATE_WRITE_RGB
				| BGFX_STATE_WRITE_A
				| BGFX_STATE_WRITE_Z
				| BGFX_STATE_DEPTH_TEST_LEQUAL
				| BGFX_STATE_CULL_CCW;

			Color color = light->getColor();
			color[3] = 1.0f;
			Transform* lt = light->getGameObject()->getTransform();

			Texture* icon = lightPointGizmoIcon;
			if (light->getLightType() == LightType::Spot)
				icon = lightSpotGizmoIcon;
			else if (light->getLightType() == LightType::Directional)
				icon = lightDirGizmoIcon;

			Primitives::billboard(lt, icon, color, gizmoIconsScale, viewId, state, Renderer::getSingleton()->getTransparentProgram(), camera);
		}

		for (auto it = cameras.begin(); it != cameras.end(); ++it)
		{
			Camera* cam = *it;
			if (cam->getGameObject() == nullptr)
				continue;

			if (cam == renderCamera)
				continue;

			uint64_t state = BGFX_STATE_WRITE_RGB
				| BGFX_STATE_WRITE_A
				| BGFX_STATE_WRITE_Z
				| BGFX_STATE_DEPTH_TEST_LEQUAL
				| BGFX_STATE_CULL_CCW;

			Transform* ct = cam->getTransform();
			Primitives::billboard(ct, cameraGizmoIcon, Color::White, gizmoIconsScale, viewId, state, Renderer::getSingleton()->getTransparentProgram(), camera);
		}

		//----------------Render gizmos end-----------------//

		if (showGrid)
		{
			//----------------Render grid begin-----------------//

			Color lineColor = Color(0.8f, 0.8f, 0.8f, 0.8f);
			Color xColor = Color(0.8f, 0.0f, 0.0f, 0.8f);
			Color zColor = Color(0.0f, 0.0f, 0.8f, 0.8f);

			uint64_t state = BGFX_STATE_WRITE_RGB
				| BGFX_STATE_WRITE_A
				| BGFX_STATE_WRITE_Z
				| BGFX_STATE_DEPTH_TEST_LEQUAL
				| BGFX_STATE_CULL_CCW;

			Primitives::grid(glm::identity<glm::mat4x4>(), 64, 1.0f, lineColor, viewId, state, Renderer::getSingleton()->getTransparentProgram(), camera);

			Transform* t = renderCamera->getTransform();
			glm::vec3 camPos = t->getPosition();

			int startX = camPos.x / 64.0f * 64.0f;
			int startZ = camPos.z / 64.0f * 64.0f;

			glm::vec3 px1 = glm::vec3(startX - 32.0f, 0.0f, startZ);
			glm::vec3 px2 = glm::vec3(startX + 31.0f, 0.0f, startZ);

			glm::vec3 pz1 = glm::vec3(startX, 0.0f, startZ - 32.0f);
			glm::vec3 pz2 = glm::vec3(startX, 0.0f, startZ + 31.0f);

			Primitives::line(glm::identity<glm::mat4x4>(), px1, px2, xColor, viewId, state, Renderer::getSingleton()->getTransparentProgram(), camera);
			Primitives::line(glm::identity<glm::mat4x4>(), pz1, pz2, zColor, viewId, state, Renderer::getSingleton()->getTransparentProgram(), camera);

			//-----------------Render grid end------------------//
		}
	}

	glm::vec2 SceneWindow::getPosition()
	{
		return position;
	}

	glm::vec2 SceneWindow::getSize()
	{
		return size;
	}

	bool SceneWindow::isFocused()
	{
		return focused;
	}

	bool SceneWindow::isHovered()
	{
		return hovered;
	}

	float SceneWindow::getToolbarHeight()
	{
		return 28.0f;
	}

	static void SceneHelpMarker(const char* desc)
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

	void SceneWindow::updateToolbar()
	{
		ImGui::SetCursorPos(ImVec2(0, 20));

		ImVec4 _col = ImGui::GetStyleColorVec4(ImGuiCol_ChildBg);
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(_col.x, _col.y, _col.z, 0.8f));
		bool begin = ImGui::BeginChild("SceneToolbar", ImVec2(size.x, 28));
		ImGui::PopStyleColor();
		if (begin)
		{
			glm::vec3 camPos = renderCamera->getGameObject()->getTransform()->getPosition();
			float fcp = renderCamera->getFar();
			float fov = renderCamera->getFOVy();

			ImGui::SetCursorPos(ImVec2(3, 6));
			ImGui::Image((void*)cameraIcon->getHandle().idx, ImVec2(16, 16));

			ImGui::SameLine();

			ImGui::SetCursorPosY(3.0f);

			ImGui::SetNextItemWidth(140);
			//ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.15f, 0.15f, 0.15f, 0.85f));
			if (ImGui::BeginCombo("##camera_settings", "Camera settings", ImGuiComboFlags_HeightLarge))
			{
				ImGui::Text("Camera speed");

				if (cameraSpeedPreset != 0)
					ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
				bool btn1 = ImGui::Button("1", ImVec2(45, 22));

				if (cameraSpeedPreset != 0)
					ImGui::PopStyleVar();

				SceneHelpMarker("Camera speed preset 1");

				if (btn1)
				{
					cameraSpeedNormal = cameraSpeed1;
					cameraSpeedFast = cameraSpeed1 * 2.0f;
					cameraSpeed = cameraSpeed1;
					cameraSpeedPreset = 0;

					saveSettings();
				}

				ImGui::SameLine();

				if (cameraSpeedPreset != 1)
					ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
				bool btn2 = ImGui::Button("2", ImVec2(45, 22));

				if (cameraSpeedPreset != 1)
					ImGui::PopStyleVar();

				SceneHelpMarker("Camera speed preset 2");

				if (btn2)
				{
					cameraSpeedNormal = cameraSpeed2;
					cameraSpeedFast = cameraSpeed2 * 2.0f;
					cameraSpeed = cameraSpeed2;
					cameraSpeedPreset = 1;

					saveSettings();
				}

				ImGui::SameLine();

				if (cameraSpeedPreset != 2)
					ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
				bool btn3 = ImGui::Button("3", ImVec2(45, 22));

				if (cameraSpeedPreset != 2)
					ImGui::PopStyleVar();

				SceneHelpMarker("Camera speed preset 3");

				if (btn3)
				{
					cameraSpeedNormal = cameraSpeed3;
					cameraSpeedFast = cameraSpeed3 * 2.0f;
					cameraSpeed = cameraSpeed3;
					cameraSpeedPreset = 2;

					saveSettings();
				}

				ImGui::SetNextItemWidth(-1);
				if (ImGui::InputFloat("##CameraSpeed", &cameraSpeedNormal, 0.25f, 0.5f))
				{
					if (cameraSpeedNormal < 0.05f)
						cameraSpeedNormal = 0.05f;

					cameraSpeedFast = cameraSpeedNormal * 2.0f;
					cameraSpeed = cameraSpeedNormal;

					if (cameraSpeedPreset == 0)
						cameraSpeed1 = cameraSpeedNormal;

					if (cameraSpeedPreset == 1)
						cameraSpeed2 = cameraSpeedNormal;

					if (cameraSpeedPreset == 2)
						cameraSpeed3 = cameraSpeedNormal;

					saveSettings();
				}

				ImGui::Separator();

				ImGui::Text("Position");

				ImGui::SetNextItemWidth(-1);
				if (ImGui::InputFloat3("##CameraPositionX", &camPos.x))
				{
					renderCamera->getGameObject()->getTransform()->setPosition(camPos);
				}

				ImGui::Separator();

				ImGui::Text("Clipping plane distance: ");

				ImGui::SetNextItemWidth(-1);
				if (ImGui::InputFloat("##CameraFarClipPlane", &fcp))
				{
					renderCamera->setFar(fcp);

					saveSettings();
				}

				ImGui::Separator();

				ImGui::Text("Projection: ");

				bool persp = renderCamera->getProjectionType() == ProjectionType::Perspective;
				bool ortho = renderCamera->getProjectionType() == ProjectionType::Orthographic;

				ImGui::SetNextItemWidth(145);
				if (ImGui::BeginCombo("##camera_projection", persp ? "Perspective" : "Orthographic"))
				{
					if (ImGui::Selectable("Perspective", &persp))
					{
						renderCamera->setProjectionType(ProjectionType::Perspective);
						saveSettings();
					}

					if (ImGui::Selectable("Orthographic", &ortho))
					{
						renderCamera->setProjectionType(ProjectionType::Orthographic);
						saveSettings();
					}

					ImGui::EndCombo();
				}

				if (ortho)
				{
					ImGui::Text("Size: ");
					float orthoSize = renderCamera->getOrthographicSize();
					ImGui::SetNextItemWidth(-1);
					if (ImGui::DragFloat("", &orthoSize))
					{
						if (orthoSize > 0 && orthoSize < FLT_MAX)
							renderCamera->setOrthographicSize(orthoSize);
					}
				}
				else
				{
					ImGui::Text("Field of view: ");

					ImGui::SetNextItemWidth(-1);
					if (ImGui::SliderFloat("##CameraFOV", &fov, 45.0f, 90.0f, "%.2f"))
					{
						renderCamera->setFOVy(fov);

						saveSettings();
					}
				}

				ImGui::EndCombo();
			}

			//ImGui::PopStyleColor();

			ImGui::SameLine();
			ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
			ImGui::SameLine();
			ImGui::SetCursorPosY(6.0f);
			ImGui::Image((void*)viewIcon->getHandle().idx, ImVec2(16, 16));
			ImGui::SameLine();

			ImGui::SetCursorPosY(3.0f);

			bool selgizmo = false;
			ImGui::SetNextItemWidth(140);
			//ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.15f, 0.15f, 0.15f, 0.85f));
			if (ImGui::BeginCombo("##view_settings", "View settings", ImGuiComboFlags_HeightLarge))
			{
				if (ImGui::Checkbox("Show stats", &showStats))
					saveSettings();

				ImGui::Separator();

				if (ImGui::Checkbox("Show bounds", &gizmo->getShowBounds()))
					saveSettings();

				ImGui::Separator();

				if (ImGui::Checkbox("Show grass bounds", &gizmo->getShowTerrainGrassBounds()))
					saveSettings();

				if (ImGui::Checkbox("Show trees bounds", &gizmo->getShowTerrainTreesBounds()))
					saveSettings();

				if (ImGui::Checkbox("Show detail meshes bounds", &gizmo->getShowTerrainDetailMeshesBounds()))
					saveSettings();

				ImGui::Separator();

				bool cull = renderCamera->getOcclusionCulling();
				if (ImGui::Checkbox("Occlusion culling", &cull))
				{
					renderCamera->setOcclusionCulling(cull);
					saveSettings();
				}

				ImGui::Separator();
				
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x * 0.5f) - (ImGui::CalcTextSize("Icons size").x * 0.5f));
				ImGui::Text("Icons size");
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				if (ImGui::SliderFloat("##icons_size", &gizmoIconsScale, 0, 1.5f))
					saveSettings();

				ImGui::EndCombo();
			}

			//ImGui::PopStyleColor();

			ImGui::SameLine();
			ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
			ImGui::SameLine();

			ImVec4 col1 = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
			ImVec4 col2 = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
			ImVec4 col3 = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);

			if (showGrid) col1 = ImVec4(1, 1, 1, 1);
			if (showNavMesh) col2 = ImVec4(1, 1, 1, 1);
			if (Renderer::getSingleton()->getShadowsEnabled()) col3 = ImVec4(1, 1, 1, 1);

			if (ImGui::ImageButton((void*)gridIcon->getHandle().idx, ImVec2(16, 16), ImVec2(0, 1), ImVec2(1, 0), -1, ImVec4(0, 0, 0, 0), col1))
			{
				setShowGrid(!showGrid);
			} // Grid
			ImGui::SameLine(); SceneHelpMarker("Toggle grid");
			if (ImGui::ImageButton((void*)navMeshIcon->getHandle().idx, ImVec2(16, 16), ImVec2(0, 1), ImVec2(1, 0), -1, ImVec4(0, 0, 0, 0), col2))
			{
				setShowNavMesh(!showNavMesh);
			} // NavMesh
			ImGui::SameLine(); SceneHelpMarker("Toggle NavMesh");
			if (ImGui::ImageButton((void*)shadowsIcon->getHandle().idx, ImVec2(16, 16), ImVec2(0, 1), ImVec2(1, 0), -1, ImVec4(0, 0, 0, 0), col3))
			{
				Renderer::getSingleton()->setShadowsEnabled(!Renderer::getSingleton()->getShadowsEnabled());
				saveSettings();
			} // Shadows
			ImGui::SameLine(); SceneHelpMarker("Toggle shadows");
		}

		ImGui::EndChild();
	}

	void SceneWindow::saveSettings()
	{
		EditorSettings* settings = MainWindow::settings;

		settings->cameraSpeed = cameraSpeed1;
		settings->cameraSpeed2 = cameraSpeed2;
		settings->cameraSpeed3 = cameraSpeed3;
		settings->cameraSpeedPreset = cameraSpeedPreset;
		settings->cameraFOV = renderCamera->getFOVy();

		if (renderCamera != nullptr)
			settings->cameraFarClipPlane = renderCamera->getFar();

		settings->showStats = showStats;
		settings->gizmoIconsScale = gizmoIconsScale;
		settings->gizmoShowBounds = gizmo->getShowBounds();
		settings->gizmoShowTerrainGrassBounds = gizmo->getShowTerrainGrassBounds();
		settings->gizmoShowTerrainTreesBounds = gizmo->getShowTerrainTreesBounds();
		settings->gizmoShowTerrainDetailMeshesBounds = gizmo->getShowTerrainDetailMeshesBounds();

		settings->showNavMesh = showNavMesh;
		settings->showGrid = showGrid;
		settings->shadowsEnabled = Renderer::getSingleton()->getShadowsEnabled();

		settings->occlusionCulling = renderCamera->getOcclusionCulling();

		settings->save();
	}

	void SceneWindow::loadSettings()
	{
		EditorSettings* settings = MainWindow::settings;

		cameraSpeed1 = settings->cameraSpeed;
		cameraSpeed2 = settings->cameraSpeed2;
		cameraSpeed3 = settings->cameraSpeed3;
		cameraSpeedPreset = settings->cameraSpeedPreset;
		renderCamera->setFOVy(settings->cameraFOV);

		if (renderCamera != nullptr)
			renderCamera->setFar(settings->cameraFarClipPlane);

		if (cameraSpeedPreset == 0)
			cameraSpeedNormal = cameraSpeed1;
		if (cameraSpeedPreset == 1)
			cameraSpeedNormal = cameraSpeed2;
		if (cameraSpeedPreset == 2)
			cameraSpeedNormal = cameraSpeed3;

		cameraSpeedFast = cameraSpeedNormal * 2.0f;
		cameraSpeed = cameraSpeedNormal;

		showStats = settings->showStats;
		gizmoIconsScale = settings->gizmoIconsScale;
		gizmo->setShowBounds(settings->gizmoShowBounds);
		gizmo->setShowTerrainGrassBounds(settings->gizmoShowTerrainGrassBounds);
		gizmo->setShowTerrainTreesBounds(settings->gizmoShowTerrainTreesBounds);
		gizmo->setShowTerrainDetailMeshesBounds(settings->gizmoShowTerrainDetailMeshesBounds);

		showNavMesh = settings->showNavMesh;
		showGrid = settings->showGrid;
		Renderer::getSingleton()->setShadowsEnabled(settings->shadowsEnabled);

		renderCamera->setOcclusionCulling(settings->occlusionCulling);
	}

	void SceneWindow::onResize()
	{
		MainWindow::getSingleton()->addOnEndUpdateCallback([=]()
			{
				renderTarget->reset(getSize().x, getSize().y);
				Renderer::getSingleton()->resetFrameBuffers();
			}
		);
	}

	void SceneWindow::onGizmoSelect(std::vector<Transform*>& nodes, std::vector<Transform*>& prevNodes, void* userData)
	{
		if (TerrainEditor::isEditModeActive())
			return;

		if (nodes.size() == 0 && prevNodes.size() == 0)
			return;

		int cnt = 0;
		for (auto& obj : nodes)
		{
			if (obj->getGameObject() == nullptr)
				continue;

			++cnt;
		}

		if (cnt == 0 && nodes.size() == 0)
		{
			for (auto& obj : prevNodes)
			{
				if (obj->getGameObject() == nullptr)
					continue;

				++cnt;
			}
		}

		if (cnt > 0)
		{
			//Undo
			UndoData* dt = Undo::addUndo("Select object");
			dt->objectData.resize(2);

			for (auto& node : prevNodes)
			{
				if (node->getGameObject() == nullptr)
					continue;

				dt->objectData[0].push_back(node);
			}

			for (auto& node : nodes)
			{
				if (node->getGameObject() == nullptr)
					continue;

				dt->objectData[1].push_back(node);
			}

			dt->undoAction = [=](UndoData* data)
			{
				std::vector<Transform*> objects;
				for (auto& obj : data->objectData[0])
					objects.push_back((Transform*)obj);

				Gizmo* gizmo = MainWindow::getSceneWindow()->getGizmo();
				gizmo->selectObjects(objects, nullptr, false);
				selectObjects(objects, nullptr);

				objects.clear();
			};

			dt->redoAction = [=](UndoData* data)
			{
				std::vector<Transform*> objects;
				for (auto& obj : data->objectData[1])
					objects.push_back((Transform*)obj);

				Gizmo* gizmo = MainWindow::getSceneWindow()->getGizmo();
				gizmo->selectObjects(objects, nullptr, false);
				selectObjects(objects, nullptr);

				objects.clear();
			};
			//
		}

		selectObjects(nodes, userData);
	}

	void SceneWindow::selectObjects(std::vector<Transform*>& nodes, void* userData)
	{
		InspectorWindow* inspectorWindow = MainWindow::getInspectorWindow();

		std::vector<std::string> names;
		std::vector<UIElement*> uiObjects;

		for (auto it = nodes.begin(); it != nodes.end(); ++it)
		{
			if ((*it)->getGameObject() != nullptr)
			{
				names.push_back((*it)->getGameObject()->getGuid());
				std::vector<Component*>& comps = (*it)->getGameObject()->getComponents();
				for (auto ct = comps.begin(); ct != comps.end(); ++ct)
				{
					if ((*ct)->isUiComponent())
					{
						uiObjects.push_back((UIElement*)(*ct));
					}
				}
			}
		}

		TreeView* hierarchyTree = MainWindow::getHierarchyWindow()->getTreeView();
		TreeView* assetsTree = MainWindow::getAssetsWindow()->getTreeView();

		hierarchyTree->selectNodes(names, false);
		assetsTree->selectNodes({  }, false);

		MainWindow::getUIEditorWindow()->selectObjects(uiObjects, false);
		uiObjects.clear();

		if (userData == nullptr)
		{
			if (nodes.size() == 1 && names.size() > 0)
			{
				TreeNode* node = hierarchyTree->getNodeByName(names[0], hierarchyTree->getRootNode());
				hierarchyTree->focusOnNode(node);
			}
		}

		if (nodes.size() > 0)
		{
			std::vector<GameObject*> lst;
			for (auto it = nodes.begin(); it != nodes.end(); ++it)
			{
				if ((*it)->getGameObject() != nullptr)
					lst.push_back((*it)->getGameObject());
			}

			if (lst.size() > 0)
			{
				ObjectEditor* editor = new ObjectEditor();
				editor->init(lst);
				inspectorWindow->setEditor(editor);
			}
		}
		else
			inspectorWindow->setEditor(nullptr);
	}

	void SceneWindow::mouseDown(int x, int y, int mb)
	{
		InputManager::MouseButton mbe = static_cast<InputManager::MouseButton>(mb);

		if (mbe == InputManager::MouseButton::MBE_LEFT)
		{
			if (!rButtonDown && !mButtonDown)
			{
				lButtonDown = true;

				GameObject* obj = renderCamera->getGameObject();
				Transform* t = obj->getTransform();

				auto& selected = gizmo->getSelectedObjects();
				if (selected.size() > 0)
				{
					glm::vec3 midPoint = gizmo->findMidPoint(selected);
					float d = glm::distance(t->getPosition(), midPoint);

					cameraPivot = t->getPosition() + t->getForward() * d;
				}
				else
				{
					cameraPivot = t->getPosition() + t->getForward() * 2.0f;
				}
			}
		}

		if (mbe == InputManager::MouseButton::MBE_RIGHT)
		{
			if (!lButtonDown && !mButtonDown && dragObject == nullptr)
				rButtonDown = true;
		}

		if (mbe == InputManager::MouseButton::MBE_MIDDLE && dragObject == nullptr)
		{
			if (!lButtonDown && !rButtonDown)
				mButtonDown = true;
		}
	}

	void SceneWindow::mouseUp(int x, int y, int mb)
	{
		InputManager::MouseButton mbe = static_cast<InputManager::MouseButton>(mb);

		if (mbe == InputManager::MouseButton::MBE_LEFT)
		{
			lButtonDown = false;
		}

		if (mbe == InputManager::MouseButton::MBE_RIGHT)
		{
			if (dragObject == nullptr)
				rButtonDown = false;
		}

		if (mbe == InputManager::MouseButton::MBE_MIDDLE)
		{
			mButtonDown = false;
		}

		if (GImGui->DragDropPayload.Data == nullptr)
		{
			if (wasHovered && !mouseOver && dragObject == nullptr)
			{
				ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
			}
		}

		wasHovered = false;
	}

	void SceneWindow::mouseMove(int x, int y)
	{
		//Camera moving
		float rOffsetX = x - prevMousePos.x;
		float rOffsetY = y - prevMousePos.y;

		bool modKey = gizmo->isModKeyPressed();

		GameObject* obj = renderCamera->getGameObject();
		Transform* t = obj->getTransform();

		if (rButtonDown)
		{
			rOffsetX *= 0.15f;
			rOffsetY *= 0.15f;

			t->yaw(-rOffsetX, true);
			t->pitch(rOffsetY, false);
		}

		if (mButtonDown)
		{
			rOffsetX *= 0.025f * cameraSpeedNormal * 0.02f;
			rOffsetY *= 0.025f * cameraSpeedNormal * 0.02f;

			glm::vec3 vCamPos = t->getPosition();
			glm::vec3 vDirUp = t->getUp();
			glm::vec3 vDirRight = t->getRight();

			vCamPos += (vDirRight * rOffsetX) + (vDirUp * rOffsetY);
			t->setPosition(vCamPos);
		}

		if (lButtonDown && !rButtonDown && !mButtonDown && modKey)
		{
			if (dragObject == nullptr)
			{
				GameObject* obj = renderCamera->getGameObject();
				Transform* t = obj->getTransform();

				rOffsetX *= 0.15f;
				rOffsetY *= 0.15f;

				glm::vec3 vDirUp = t->getUp();
				glm::vec3 vDirRight = t->getRight();
				glm::vec3 vCamRot = (vDirRight * rOffsetY) + (vDirUp * -rOffsetX);

				glm::vec3 origin = t->getPosition();
				glm::quat angle = Mathf::toQuaternion(vCamRot);

				glm::mat4x4 matRot = glm::mat4_cast(angle);

				glm::vec3 point = Mathf::rotateAround(origin, cameraPivot, matRot);
				glm::vec3 dir = glm::normalize(cameraPivot - point);
				glm::quat rot = glm::quatLookAtLH(dir, glm::vec3(0, 1, 0));

				t->setPosition(point);
				t->setRotation(rot);
			}
		}

		prevMousePos = glm::vec2(x, y);

		if (dragObject != nullptr)
		{
			Ray ray = renderCamera->getCameraToViewportRay(x, y);

			LayerMask mask;
			mask.setLayer(31, false);

			Raycast raycast;
			raycast.checkCSGGeometry = true;
			raycast.raycastTransientRenderables = false;
			
			auto hits = raycast.execute(ray, mask);
			Raycast::HitInfo inf;
			if (hits.size() > 0)
				inf = hits[0];

			if (inf.object != nullptr || inf.csgSubMesh != nullptr)
			{
				dragObject->setPosition(inf.hitPoint);
			}
			else
				dragObject->setPosition(ray.origin + ray.direction * 7.0f);
		}
	}

	void SceneWindow::mouseWheel(int x, int y)
	{
		if (!lButtonDown)
		{
			if (renderCamera->getProjectionType() == ProjectionType::Perspective)
			{
				Transform* camTrans = renderCamera->getTransform();

				glm::vec3 vCamPos = camTrans->getPosition();
				glm::vec3 vCamDir = camTrans->getForward();
				vCamPos += vCamDir * (float)y * cameraSpeedNormal * 0.1f;
				camTrans->setPosition(vCamPos);

				MainWindow::getSingleton()->focusCameraOnObject = false;
			}
			else
			{
				float orthoSize = renderCamera->getOrthographicSize();
				float step = -(float)y * cameraSpeedNormal * 0.1f;
				orthoSize += step;

				if (orthoSize <= 0.0f)
					orthoSize = -step;

				renderCamera->setOrthographicSize(orthoSize);

				MainWindow::getSingleton()->focusCameraOnObject = false;
			}
		}
	}

	void SceneWindow::dropObject(GameObject * obj)
	{
		if (TerrainEditor::isEditModeActive())
			MainWindow::getInspectorWindow()->setEditor(nullptr);

		if (layerMem.size() == 0)
			obj->setLayer(0, true);
		else
		{
			setLayers(obj, layerMem);
			layerMem.clear();
		}

		gizmo->selectObject(nullptr);
		MainWindow::getSingleton()->addGameObjectUndo({ obj }, "Add object");

		MainWindow::getSingleton()->getHierarchyWindow()->insertNode(obj->getTransform(), nullptr);

		gizmo->selectObject(obj->getTransform());

		wasHovered = false;
	}

	void SceneWindow::setShowGrid(bool value)
	{
		showGrid = value;
		saveSettings();
	}

	void SceneWindow::setShowNavMesh(bool value)
	{
		showNavMesh = value;
		saveSettings();
	}

	void SceneWindow::onGizmoStartManipulate(std::vector<Transform*>& objects)
	{
		int cnt = 0;
		for (auto& obj : objects)
		{
			if (obj->getGameObject() == nullptr)
				continue;

			++cnt;
		}

		if (cnt == 0)
			return;

		undoData = Undo::addUndo("Transform object");
		undoData->matrixData.resize(2);

		for (auto& obj : objects)
		{
			if (obj->getGameObject() == nullptr)
				continue;

			undoData->matrixData[0][obj] = obj->getTransformMatrix();
		}

		undoData->undoAction = [=](UndoData* data)
		{
			bool upd = false;
			bool upd1 = false;

			std::vector<Component*> comps;

			for (auto& obj : data->matrixData[0])
			{
				Transform* t = (Transform*)obj.first;
				t->setTransformMatrix(obj.second);

				if (!upd || !upd1)
				{
					t->iterateChildren([&upd, &upd1, &comps](Transform* child) -> bool
						{
							if (child->getGameObject() != nullptr)
							{
								if (child->getGameObject()->getBatchingStatic())
									upd = true;

								CSGBrush* brush = (CSGBrush*)child->getGameObject()->getComponent(CSGBrush::COMPONENT_TYPE);
								if (brush != nullptr)
								{
									comps.push_back(brush);

									brush->rebuild();
									upd1 = true;
								}
							}

							return true;
						}
					);
				}
			}

			Gizmo* gizmo = MainWindow::getSceneWindow()->getGizmo();
			gizmo->updatePosition();

			MainWindow::getSingleton()->getInspectorWindow()->updateObjectEditorTransform();

			if (upd)
				MainWindow::addOnEndUpdateCallback([=]() { BatchedGeometry::getSingleton()->rebuild(); });

			if (upd1)
				MainWindow::addOnEndUpdateCallback([=]() { CSGGeometry::getSingleton()->rebuild(comps); });
		};
	}

	void SceneWindow::onGizmoEndManipulate(std::vector<Transform*>& objects)
	{
		if (undoData == nullptr)
			return;

		bool updateBatches = false;
		bool updateCSG = false;

		std::vector<Component*> components;

		for (auto& obj : objects)
		{
			if (obj->getGameObject() == nullptr)
				continue;

			if (!updateBatches || !updateCSG)
			{
				obj->iterateChildren([&updateBatches, &updateCSG, &components](Transform* child) -> bool
					{
						if (child->getGameObject() != nullptr)
						{
							if (child->getGameObject()->getBatchingStatic())
								updateBatches = true;

							CSGBrush* brush = (CSGBrush*)child->getGameObject()->getComponent(CSGBrush::COMPONENT_TYPE);
							if (brush != nullptr)
							{
								components.push_back(brush);

								brush->rebuild();
								updateCSG = true;
							}
						}

						return true;
					}
				);
			}

			undoData->matrixData[1][obj] = obj->getTransformMatrix();
		}

		undoData->redoAction = [=](UndoData* data)
		{
			bool upd = false;
			bool upd1 = false;

			std::vector<Component*> comps;

			for (auto& obj : data->matrixData[1])
			{
				Transform* t = (Transform*)obj.first;
				t->setTransformMatrix(obj.second);

				if (!upd || !upd1)
				{
					t->iterateChildren([&upd, &upd1, &comps](Transform* child) -> bool
						{
							if (child->getGameObject() != nullptr)
							{
								if (child->getGameObject()->getBatchingStatic())
									upd = true;

								CSGBrush* brush = (CSGBrush*)child->getGameObject()->getComponent(CSGBrush::COMPONENT_TYPE);
								if (brush != nullptr)
								{
									comps.push_back(brush);

									brush->rebuild();
									upd1 = true;
								}
							}

							return true;
						}
					);
				}
			}

			Gizmo* gizmo = MainWindow::getSceneWindow()->getGizmo();
			gizmo->updatePosition();

			MainWindow::getSingleton()->getInspectorWindow()->updateObjectEditorTransform();

			if (upd)
				MainWindow::addOnEndUpdateCallback([=]() { BatchedGeometry::getSingleton()->rebuild(); });

			if (upd1)
				MainWindow::addOnEndUpdateCallback([=]() { CSGGeometry::getSingleton()->rebuild(comps); });
		};

		undoData = nullptr;

		if (updateBatches)
			MainWindow::addOnEndUpdateCallback([=]() { BatchedGeometry::getSingleton()->rebuild(); });

		if (updateCSG)
			MainWindow::addOnEndUpdateCallback([=]() { CSGGeometry::getSingleton()->rebuild(components); });
	}

	void SceneWindow::onGizmoManipulate(std::vector<Transform*>& objects)
	{
		MainWindow::getSingleton()->getInspectorWindow()->updateObjectEditorTransform();
	}
}