#include "Gizmo.h"

#include <imgui.h>
#include <imgui_internal.h>

#include "glm/mat4x4.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/matrix_decompose.hpp"

#include "ImGuizmo.h"

#include "../Core/Engine.h"
#include "../Renderer/Renderer.h"
#include "../Core/GameObject.h"
#include "../Core/InputManager.h"
#include "../Math/Raycast.h"
#include "../Math/Ray.h"

#include "../Renderer/Primitives.h"
#include "../Math/Mathf.h"
#include "../Classes/GUIDGenerator.h"

#include "../Components/Transform.h"
#include "../Components/MeshRenderer.h"
#include "../Components/DecalRenderer.h"
#include "../Components/Camera.h"
#include "../Components/Light.h"
#include "../Components/CapsuleCollider.h"
#include "../Components/BoxCollider.h"
#include "../Components/SphereCollider.h"
#include "../Components/FixedJoint.h"
#include "../Components/HingeJoint.h"
#include "../Components/FreeJoint.h"
#include "../Components/ConeTwistJoint.h"
#include "../Components/Vehicle.h"
#include "../Components/NavMeshObstacle.h"
#include "../Components/Terrain.h"
#include "../Components/ParticleSystem.h"
#include "../Components/Water.h"
#include "../Components/Spline.h"
#include "../Components/CSGBrush.h"

#include <SDL2/SDL.h>

namespace GX
{
	float _matrix[16] =
		{ 1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		0.f, 0.f, 0.f, 1.f };

	Gizmo::Gizmo()
	{

	}

	Gizmo::~Gizmo()
	{
		if (localBounds != nullptr)
		{
			delete[] localBounds;
			localBounds = nullptr;
		}

		ignoreObjects.clear();
	}

	void Gizmo::init(Camera* cam)
	{
		camera = cam;

		Renderer::getSingleton()->addRenderCallback([=](int viewId, int viewLayer, Camera* camera) { drawWireframes(viewId, viewLayer, camera); });

		hide();

		InputManager::getSingleton()->subscribeMouseDownEvent([=](InputManager::MouseButton btn, int x, int y)
			{
				if (isViewportHovered)
				{
					if (!modKeyPressed)
					{
						if (btn == InputManager::MouseButton::MBE_LEFT)
						{
							if (!rButtonDown && !mButtonDown)
							{
								lButtonDown = true;

								if (InputManager::getSingleton()->getKey(SDL_SCANCODE_LALT))
									modKeyPressed = true;
							}
						}

						if (btn == InputManager::MouseButton::MBE_RIGHT)
						{
							if (!lButtonDown && !mButtonDown)
								rButtonDown = true;
						}

						if (btn == InputManager::MouseButton::MBE_MIDDLE)
						{
							if (!lButtonDown && !rButtonDown)
								mButtonDown = true;
						}

						if (lButtonDown && !rButtonDown && !mButtonDown)
							mouseDown((float)x, (float)y);
					}
				}
			}
		);

		InputManager::getSingleton()->subscribeMouseUpEvent([=](InputManager::MouseButton btn, int x, int y)
			{
				bool lBtn = lButtonDown;
				bool modKey = modKeyPressed;
				if (btn == InputManager::MouseButton::MBE_LEFT)
				{
					lButtonDown = false;
					modKeyPressed = false;
				}
				if (btn == InputManager::MouseButton::MBE_RIGHT)
					rButtonDown = false;
				if (btn == InputManager::MouseButton::MBE_MIDDLE)
					mButtonDown = false;

				if (btn == InputManager::MouseButton::MBE_LEFT && lBtn && !modKey)
					mouseUp((float)x, (float)y, InputManager::getSingleton()->getKey(SDL_SCANCODE_LCTRL));
			}
		);

		InputManager::getSingleton()->subscribeMouseMoveEvent([=](int x, int y)
			{
				if (lButtonDown && !rButtonDown && !mButtonDown)
					mouseMove((float)x, (float)y);
			}
		);
	}

	void Gizmo::beginFrame()
	{
		ImGuizmo::BeginFrame();
	}

	void Gizmo::updateView()
	{
		view = camera->getViewMatrix();
		projection = camera->getProjectionMatrix();

		ImGuizmo::SetRect(rect.left, rect.top, rect.right, rect.bottom);
		ImGuizmo::SetClipRect(clipRect.left, clipRect.top, clipRect.right, clipRect.bottom);
	}

	void Gizmo::updateManipulator()
	{
		//glm::mat4x4 inversionMatrix(1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
		//view = inversionMatrix * view;
		
		ImGuizmo::OPERATION op = ImGuizmo::OPERATION::TRANSLATE;
		ImGuizmo::MODE md = ImGuizmo::MODE::WORLD;

		switch (gizmoType)
		{
		case GizmoType::GT_SELECT:
			ImGuizmo::Enable(false);
			break;
		case GizmoType::GT_MOVE:
			op = ImGuizmo::OPERATION::TRANSLATE;
			ImGuizmo::Enable(true);
			break;
		case GizmoType::GT_ROTATE:
			op = ImGuizmo::OPERATION::ROTATE;
			ImGuizmo::Enable(true);
			break;
		case GizmoType::GT_SCALE:
			op = ImGuizmo::OPERATION::SCALE;
			ImGuizmo::Enable(true);
			break;
		default:
			op = ImGuizmo::OPERATION::TRANSLATE;
			ImGuizmo::Enable(true);
			break;
		}

		switch (transformSpace)
		{
		case TransformSpace::TS_WORLD:
			md = ImGuizmo::MODE::WORLD;
			break;
		case TransformSpace::TS_LOCAL:
			md = ImGuizmo::MODE::LOCAL;
			break;
		default:
			md = ImGuizmo::MODE::WORLD;
			break;
		}

		CSGBrush* csgBrush = nullptr;

		if (selectedObjects.size() > 0)
		{
			GameObject* obj = selectedObjects[0]->getGameObject();
			if (obj != nullptr)
			{
				csgBrush = (CSGBrush*)obj->getComponent(CSGBrush::COMPONENT_TYPE);
				if (op == ImGuizmo::OPERATION::SCALE)
				{
					if (csgBrush != nullptr)
						op = ImGuizmo::OPERATION::BOUNDS;
				}
			}
		}

		bool _snapToGrid = snapToGrid;

		if (InputManager::getSingleton()->getKey(SDL_SCANCODE_LCTRL))
			_snapToGrid = !snapToGrid;

		float snap[3] = { 0, 0, 0 };
		if (_snapToGrid)
		{
			if (gizmoType == GizmoType::GT_MOVE)
			{
				snap[0] = snapToGridMoveSize;
				snap[1] = snapToGridMoveSize;
				snap[2] = snapToGridMoveSize;
			}

			if (gizmoType == GizmoType::GT_ROTATE)
			{
				snap[0] = snapToGridRotateSize;
				snap[1] = snapToGridRotateSize;
				snap[2] = snapToGridRotateSize;
			}

			if (gizmoType == GizmoType::GT_SCALE)
			{
				snap[0] = snapToGridScaleSize;
				snap[1] = snapToGridScaleSize;
				snap[2] = snapToGridScaleSize;
			}
		}

		bool modKey = InputManager::getSingleton()->getKey(SDL_SCANCODE_LALT);

		if (rButtonDown || mButtonDown || ImGuizmo::IsOverView() || ImGuizmo::IsUsingView() || !isViewportHovered || modKeyPressed || modKey)
		{
			if (!ImGuizmo::IsUsing())
				ImGuizmo::Enable(false);
		}

		if (canManipulate)
		{
			ImGuizmo::SetOrthographic(camera->getProjectionType() == ProjectionType::Orthographic);

			if (op == ImGuizmo::OPERATION::BOUNDS)
			{
				if (csgBrush != nullptr)
				{
					if (localBounds == nullptr)
					{
						localBounds = new float[6];

						AxisAlignedBox bounds = csgBrush->getLocalBounds();

						localBounds[0] = bounds.getMinimum().x;
						localBounds[1] = bounds.getMinimum().y;
						localBounds[2] = bounds.getMinimum().z;
						localBounds[3] = bounds.getMaximum().x;
						localBounds[4] = bounds.getMaximum().y;
						localBounds[5] = bounds.getMaximum().z;
					}
				}
			}
			else
			{
				if (localBounds != nullptr)
				{
					delete[] localBounds;
					localBounds = nullptr;
				}
			}

			ImGuizmo::SetDrawlist();
			ImGui::PushClipRect(ImVec2(clipRect.left, clipRect.top), ImVec2(clipRect.left + clipRect.right, clipRect.top + clipRect.bottom), true);
			ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(projection), op, md, _matrix, nullptr, snap, localBounds, snap);
			ImGui::PopClipRect();

			if (ImGuizmo::IsUsing())
			{
				float _translation[3] = { 0, 0, 0 };
				float _rotation[3] = { 0, 0, 0 };
				float _scale[3] = { 1, 1, 1 };

				ImGuizmo::DecomposeMatrixToComponents(_matrix, _translation, _rotation, _scale);

				transformMatrix = glm::make_mat4(_matrix);

				if (transformOffsets.size() == 0)
				{
					for (auto it = selectedObjects.begin(); it != selectedObjects.end(); ++it)
						transformOffsets[*it] = glm::inverse(transformMatrix) * (*it)->getTransformMatrix();
				}

				for (auto it = selectedObjects.begin(); it != selectedObjects.end(); ++it)
				{
					Transform* t = *it;
					t->setTransformMatrix(transformMatrix * transformOffsets[t]);
				}
			}

			if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
				updatePosition();
		}
	}

	void Gizmo::updateViewManipulator()
	{
		if (rButtonDown || mButtonDown || !isViewportHovered)
		{
			if (!ImGuizmo::IsUsingView())
				ImGuizmo::EnableView(false);
		}
		else
			ImGuizmo::EnableView(true);

		ImGuizmo::ViewManipulate(glm::value_ptr(view), dist, ImVec2(rect.left + rect.right - 105, rect.top + 32), ImVec2(100, 100), 0x00000000);

		if (ImGuizmo::IsUsingView())
		{
			if (!rButtonDown && !mButtonDown)
			{
				glm::mat4x4 inversionMatrix = glm::mat4x4(-1, 0, 0, 0, 0, 1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 1);
				view = inversionMatrix * view;

				glm::mat4x4 vw = glm::inverse(view);

				camera->getGameObject()->getTransform()->setTransformMatrix(vw);
			}
		}
	}

	void Gizmo::show()
	{
		ImGuizmo::Enable(true);
		visible = true;

		updatePosition();
	}

	void Gizmo::hide()
	{
		//showWireframe(false);
		ImGuizmo::Enable(false);
		visible = false;

		updatePosition();
	}

	bool Gizmo::isVisible()
	{
		return visible;
	}

	void Gizmo::mouseDown(float x, float y)
	{
		if (!manipulatorActive)
			return;

		//mStart.x = x / Renderer::getSingleton()->getWidth();
		//mStart.y = y / Renderer::getSingleton()->getHeight();
	}

	void Gizmo::mouseUp(float x, float y, bool multipleSelection)
	{
		if (!manipulatorActive)
			return;

		if (!ImGuizmo::IsUsing() && !(ImGuizmo::IsUsingView() || ImGuizmo::IsOverView()) && !rButtonDown && !mButtonDown && isViewportHovered)
		{
			SelectedObjects prevObjects = selectedObjects;

			Ray ray = camera->getCameraToViewportRay(x, y);
			
			Raycast raycast;
			raycast.checkCSGGeometry = true;
			raycast.findCSGBrushes = true;

			auto hits = raycast.execute(ray);
			Raycast::HitInfo inf;
			if (hits.size() > 0)
				inf = hits[0];
			
			Transform* hitObj = inf.object;

			if (hitObj != NULL)
			{
				selectObject(hitObj, multipleSelection);
			}
			else
			{
				if (!multipleSelection)
				{
					selectedObjects.clear();

					if (selectCallback != nullptr)
						selectCallback(selectedObjects, prevObjects, nullptr);

					hide();
				}
			}

			prevObjects.clear();
		}

		if (moving)
		{
			moving = false;
			startMoving = false;

			transformOffsets.clear();
		}

		if (ImGuizmo::IsUsing())
		{
			for (auto& cb : manipulateCallbacks)
				cb.second(selectedObjects);

			for (auto& cb : manipulateEndCallbacks)
				cb.second(selectedObjects);
		}

		if (localBounds != nullptr)
		{
			delete[] localBounds;
			localBounds = nullptr;
		}
	}

	void Gizmo::mouseMove(float x, float y)
	{
		if (!manipulatorActive)
			return;

		if (lButtonDown)
		{
			if (!ImGuizmo::IsOver() && !(ImGuizmo::IsUsingView() || ImGuizmo::IsOverView()) && !rButtonDown && !mButtonDown)
			{
				//mStop.x = x / Renderer::getSingleton()->getWidth();
				//mStop.y = y / Renderer::getSingleton()->getHeight();

				//mSelectionBox->setCorners(mStart, mStop);

				//mBoxSelecting = true;
				//mSelectionBox->clear();
				//mSelectionBox->setVisible(true);
			}

			//if (mBoxSelecting && canBoxSelect)
			//{
			//	//mStop.x = x / Renderer::getSingleton()->getWidth();
			//	//mStop.y = y / Renderer::getSingleton()->getHeight();

			//	//mSelectionBox->setCorners(mStart, mStop);
			//}

			if (selectedObjects.size() > 0 && ImGuizmo::IsUsing())
			{
				moving = true;

				if (!startMoving)
				{
					startMoving = true;

					for (auto& cb : manipulateStartCallbacks)
						cb.second(selectedObjects);

					//addToGizmo();
				}
			}

			if (ImGuizmo::IsUsing())
			{
				for (auto& cb : manipulateCallbacks)
					cb.second(selectedObjects);
			}
		}
	}

	void Gizmo::getNodesBounds(Transform* root, AxisAlignedBox& box)
	{
		if (root->getGameObject() != nullptr)
		{
			if (root->getGameObject()->isSerializable())
			{
				MeshRenderer* rend = (MeshRenderer*)root->getGameObject()->getComponent(MeshRenderer::COMPONENT_TYPE);

				if (rend == nullptr)
				{
					box.merge(root->getPosition());
				}
				else
				{
					box.merge(rend->getBounds());
				}

				std::vector<Transform*>& children = root->getChildren();
				for (auto it = children.begin(); it != children.end(); ++it)
				{
					Transform* child = *it;
					getNodesBounds(child, box);
				}
			}
		}
		else
		{
			box.merge(root->getPosition());
		}
	}

	glm::vec3 Gizmo::findMidPoint(std::vector<Transform*>& nodes)
	{
		glm::vec3 barycenter = glm::vec3(0, 0, 0);

		if (centerBase == CenterBase::CB_PIVOT)
		{
			if (nodes.size() > 0)
			{
				for (std::vector<Transform*>::iterator it = nodes.begin(); it != nodes.end(); ++it)
				{
					barycenter += (*it)->getPosition();
				}

				barycenter *= pow(nodes.size(), -1);
			}
		}
		else
		{
			if (nodes.size() > 0)
			{
				Transform* first = nodes[0];

				AxisAlignedBox box;
				//if (first->getGameObject()->getComponent(MeshRenderer::COMPONENT_TYPE) == nullptr)
				//	box.setExtents(first->getPosition(), first->getPosition() + glm::vec3(0.1f, 0.1f, 0.1f));

				for (auto it = nodes.begin(); it != nodes.end(); ++it)
				{
					Transform* node = *it;
					getNodesBounds(node, box);
				}

				barycenter = box.getCenter();
			}
		}

		return barycenter;
	}

	void Gizmo::selectObject(Transform* idObject, bool multipleSelection, bool callCallback, void* userData)
	{
		auto it = std::find(ignoreObjects.begin(), ignoreObjects.end(), idObject);
		if (it == ignoreObjects.end())
		{
			SelectedObjects prevObjects = selectedObjects;

			if (!multipleSelection)
				selectedObjects.clear();

			if (idObject != NULL)
			{
				auto _ext = std::find(selectedObjects.begin(), selectedObjects.end(), idObject);
				if (_ext == selectedObjects.end())
					selectedObjects.push_back(idObject);
				else
					selectedObjects.erase(_ext);
			}
			else
			{
				if (!multipleSelection)
					selectedObjects.clear();
			}

			updatePosition();

			if (callCallback)
			{
				if (selectedObjects.size() > 0)
				{
					show();
					if (selectCallback != nullptr)
						selectCallback(selectedObjects, prevObjects, userData);
				}
				else
				{
					hide();
					if (selectCallback != nullptr)
						selectCallback(selectedObjects, prevObjects, userData);
				}
			}

			prevObjects.clear();
		}
	}

	void Gizmo::selectObjects(SelectedObjects idObjects, void* userData, bool callCallback, bool updateWireframe)
	{
		SelectedObjects prevObjects = selectedObjects;
		selectedObjects.clear();

		for (SelectedObjects::iterator it = idObjects.begin(); it != idObjects.end(); ++it)
		{
			selectedObjects.push_back(*it);
		}

		updatePosition();

		if (selectedObjects.size() > 0)
		{
			show();

			if (callCallback)
				selectCallback(selectedObjects, prevObjects, userData);
		}
		else
		{
			hide();

			if (callCallback)
				selectCallback(selectedObjects, prevObjects, userData);
		}

		prevObjects.clear();
	}

	std::string Gizmo::addManipulateCallback(std::function<void(SelectedObjects& nodes)> callback)
	{
		std::string guid = GUIDGenerator::genGuid();
		manipulateCallbacks.push_back({ guid, callback });

		return guid;
	}

	std::string Gizmo::addManipulateStartCallback(std::function<void(SelectedObjects& nodes)> callback)
	{
		std::string guid = GUIDGenerator::genGuid();
		manipulateStartCallbacks.push_back({ guid, callback });

		return guid;
	}

	std::string Gizmo::addManipulateEndCallback(std::function<void(SelectedObjects& nodes)> callback)
	{
		std::string guid = GUIDGenerator::genGuid();
		manipulateEndCallbacks.push_back({ guid, callback });

		return guid;
	}

	void Gizmo::removeManipulateCallback(std::string guid)
	{
		auto it = std::find_if(manipulateCallbacks.begin(), manipulateCallbacks.end(), [=](std::pair<std::string, std::function<void(SelectedObjects& nodes)>>& cb) -> bool
			{
				return cb.first == guid;
			}
		);

		if (it != manipulateCallbacks.end())
			manipulateCallbacks.erase(it);
	}

	void Gizmo::removeManipulateStartCallback(std::string guid)
	{
		auto it = std::find_if(manipulateStartCallbacks.begin(), manipulateStartCallbacks.end(), [=](std::pair<std::string, std::function<void(SelectedObjects& nodes)>>& cb) -> bool
			{
				return cb.first == guid;
			}
		);

		if (it != manipulateStartCallbacks.end())
			manipulateStartCallbacks.erase(it);
	}

	void Gizmo::removeManipulateEndCallback(std::string guid)
	{
		auto it = std::find_if(manipulateEndCallbacks.begin(), manipulateEndCallbacks.end(), [=](std::pair<std::string, std::function<void(SelectedObjects& nodes)>>& cb) -> bool
			{
				return cb.first == guid;
			}
		);

		if (it != manipulateEndCallbacks.end())
			manipulateEndCallbacks.erase(it);
	}

	void Gizmo::callSelectCallback()
	{
		if (selectCallback != nullptr)
			selectCallback(selectedObjects, selectedObjects, nullptr);
	}

	void Gizmo::setIgnoreObject(Transform* t, bool value)
	{
		auto it = std::find(ignoreObjects.begin(), ignoreObjects.end(), t);

		if (it != ignoreObjects.end())
		{
			if (!value)
				ignoreObjects.erase(it);
		}
		else
		{
			if (value)
				ignoreObjects.push_back(t);
		}
	}

	void Gizmo::clearSelection()
	{
		selectObject(nullptr, false, false);
		hide();
	}

	void Gizmo::setCenterBase(CenterBase value)
	{
		centerBase = value;
		selectObjects(selectedObjects, nullptr, false);
	}

	void Gizmo::updatePosition()
	{
		if (localBounds != nullptr)
		{
			delete[] localBounds;
			localBounds = nullptr;
		}

		dist = 10.0f;

		if (isVisible())
		{
			glm::vec3 pos = findMidPoint(selectedObjects);
			glm::vec3 rot = glm::vec3(0.0f);
			glm::vec3 scl = glm::vec3(1.0f);
			if (selectedObjects.size() == 1)
			{
				rot = Mathf::toEuler(selectedObjects[0]->getRotation());
				scl = selectedObjects[0]->getScale();
			}

			if (selectedObjects.size() > 0)
				dist = glm::distance(pos, camera->getGameObject()->getTransform()->getPosition());

			ImGuizmo::RecomposeMatrixFromComponents(glm::value_ptr(pos), glm::value_ptr(rot), glm::value_ptr(scl), _matrix);
		}
		else
		{
			glm::vec3 pos = glm::vec3(0, 0, 0);
			glm::vec3 rot = glm::vec3(0, 0, 0);
			glm::vec3 scl = glm::vec3(1, 1, 1);

			ImGuizmo::RecomposeMatrixFromComponents(glm::value_ptr(pos), glm::value_ptr(rot), glm::value_ptr(scl), _matrix);
		}
	}

	void Gizmo::drawWireframes(int viewId, int viewLayer, Camera* camera)
	{
		std::vector<Transform*>& selected = selectedObjects;

		uint64_t colliderState = BGFX_STATE_WRITE_RGB
			| BGFX_STATE_WRITE_A
			| BGFX_STATE_WRITE_Z
			| BGFX_STATE_DEPTH_TEST_ALWAYS
			| BGFX_STATE_CULL_CW;

		uint64_t csgState = BGFX_STATE_WRITE_RGB
			| BGFX_STATE_WRITE_A
			| BGFX_STATE_WRITE_Z
			| BGFX_STATE_DEPTH_TEST_LEQUAL
			| BGFX_STATE_CULL_CW;

		int overlayViewId = Renderer::getSingleton()->getOverlayViewId() + viewLayer;

		std::vector<Transform*> nstack;
		for (auto it = selected.begin(); it != selected.end(); ++it)
			nstack.push_back(*it);

		while (nstack.size() > 0)
		{
			Transform* child = *nstack.begin();
			nstack.erase(nstack.begin());

			GameObject* obj = child->getGameObject();
			if (obj == nullptr || !obj->getActive())
				continue;

			Transform* t = obj->getTransform();
			std::vector<Component*> & comps = obj->getComponents();
			for (auto it = comps.begin(); it != comps.end(); ++it)
			{
				Component* comp = *it;
				if (!comp->getEnabled())
					continue;

				if (comp->getComponentType() == Light::COMPONENT_TYPE)
				{
					Light* light = (Light*)comp;

					if (light->getLightType() == LightType::Point)
					{
						glm::mat4x4 mtx = glm::identity<glm::mat4x4>();
						mtx = glm::translate(mtx, t->getPosition());
						mtx = glm::scale(mtx, glm::vec3(1.0) * light->getRadius());
						Primitives::circledSphere(mtx, light->getColor(), viewId, colliderState | BGFX_STATE_PT_LINES, Renderer::getSingleton()->getTransparentProgram(), camera);
					}

					if (light->getLightType() == LightType::Spot)
					{
						glm::mat4x4 mtx = glm::identity<glm::mat4x4>();
						glm::mat4x4 rotMat = glm::mat4_cast(t->getRotation());
						mtx = glm::translate(mtx, t->getPosition());
						mtx *= rotMat;
						mtx = glm::scale(mtx, glm::vec3(1.0));
						Primitives::circledCone(mtx, light->getRadius(), (light->getOuterRadius() * Mathf::fDeg2Rad) * light->getRadius(), light->getColor(), viewId, colliderState | BGFX_STATE_PT_LINES, Renderer::getSingleton()->getTransparentProgram(), camera);
					}

					if (light->getLightType() == LightType::Directional)
					{
						glm::mat4x4 mtx = glm::identity<glm::mat4x4>();
						glm::mat4x4 rotMat = glm::mat4_cast(t->getRotation());
						mtx = glm::translate(mtx, t->getPosition());
						mtx *= rotMat;
						mtx = glm::scale(mtx, glm::vec3(1.0));
						Primitives::circledRays(mtx, 1.0f, 0.35f, light->getColor(), viewId, colliderState | BGFX_STATE_PT_LINES, Renderer::getSingleton()->getTransparentProgram(), camera);
					}
				}

				if (comp->getComponentType() == Camera::COMPONENT_TYPE)
				{
					Camera* cam = (Camera*)comp;

					glm::mat4x4 mtx = glm::identity<glm::mat4x4>();
					glm::mat4x4 rotMat = glm::mat4_cast(t->getRotation());

					if (cam->getProjectionType() == ProjectionType::Perspective)
					{
						mtx = glm::translate(mtx, t->getPosition());
						mtx *= rotMat;
						mtx = glm::scale(mtx, glm::vec3(1.0));

						Primitives::cameraFrustum(mtx, cam, Color::White, viewId, colliderState | BGFX_STATE_PT_LINES, Renderer::getSingleton()->getTransparentProgram(), camera);
					}
					else
					{
						float orthoSize = cam->getOrthographicSize() * 2.0f;
						float aspect = cam->getAspectRatio();
						float length = cam->getFar() - cam->getNear();

						glm::vec3 pos = t->getRotation() * glm::vec3(0.0f, 0.0f, cam->getFar() + cam->getNear());
						glm::vec3 scl = glm::vec3(orthoSize * aspect, orthoSize, length);

						mtx = glm::translate(mtx, t->getPosition() + pos);
						mtx *= rotMat;
						mtx = glm::scale(mtx, scl);

						Primitives::box(mtx, Color::White, viewId, colliderState | BGFX_STATE_PT_LINES, Renderer::getSingleton()->getTransparentProgram(), camera);
					}
				}

				if (comp->getComponentType() == MeshRenderer::COMPONENT_TYPE)
				{
					MeshRenderer* rend = (MeshRenderer*)comp;
					if (rend->getMesh() != nullptr)
					{
						uint64_t state = BGFX_STATE_WRITE_RGB
							| BGFX_STATE_WRITE_A
							| BGFX_STATE_WRITE_Z
							| BGFX_STATE_DEPTH_TEST_ALWAYS
							| BGFX_STATE_CULL_CW;

						Primitives::mesh(child->getTransformMatrix(), rend->getMesh(), Color::White, Renderer::getSingleton()->getOutlineViewId() + viewLayer, state, Renderer::getSingleton()->getOutlineProgram(), camera);

						if (showBounds)
						{
							glm::mat4x4 mtxCube = glm::identity<glm::mat4x4>();
							mtxCube = glm::translate(mtxCube, rend->getBounds().getCenter());
							mtxCube = glm::scale(mtxCube, rend->getBounds().getHalfSize());
							Primitives::box(mtxCube, Color::White, viewId, state | BGFX_STATE_PT_LINES, Renderer::getSingleton()->getTransparentProgram(), camera);
						}
					}
				}

				if (comp->getComponentType() == DecalRenderer::COMPONENT_TYPE)
				{
					DecalRenderer* decal = (DecalRenderer*)comp;

					glm::mat4x4 mtx = glm::identity<glm::mat4x4>();
					glm::mat4x4 rotMat = glm::mat4_cast(t->getRotation());
					mtx = glm::translate(mtx, t->getPosition());
					mtx *= rotMat;
					mtx = glm::scale(mtx, t->getScale());
					Primitives::box(mtx, Color::Red, overlayViewId, colliderState | BGFX_STATE_PT_LINES, Renderer::getSingleton()->getSimpleProgram(), camera);

					if (showBounds)
					{
						glm::mat4x4 mtxCube = glm::identity<glm::mat4x4>();
						mtxCube = glm::translate(mtxCube, decal->getBounds().getCenter());
						mtxCube = glm::scale(mtxCube, decal->getBounds().getHalfSize());
						Primitives::box(mtxCube, Color::White, viewId, colliderState | BGFX_STATE_PT_LINES, Renderer::getSingleton()->getTransparentProgram(), camera);
					}
				}

				if (comp->getComponentType() == ParticleSystem::COMPONENT_TYPE)
				{
					ParticleSystem* ps = (ParticleSystem*)comp;

					std::vector<ParticleEmitter*>& emitters = ps->getEmitters();
					for (auto em = emitters.begin(); em != emitters.end(); ++em)
					{
						uint64_t state = BGFX_STATE_WRITE_RGB
							| BGFX_STATE_WRITE_A
							| BGFX_STATE_WRITE_Z
							| BGFX_STATE_DEPTH_TEST_ALWAYS
							| BGFX_STATE_CULL_CW;

						glm::mat4x4 mtxCube = t->getTransformMatrix();

						ParticleEmitter* emitter = *em;
						if (emitter->getShape() == ParticleEmitterShape::Box)
						{
							mtxCube = glm::scale(mtxCube, emitter->getSize());
							Primitives::box(mtxCube, Color::Blue, overlayViewId, state | BGFX_STATE_PT_LINES, Renderer::getSingleton()->getTransparentProgram(), camera);
						}

						if (emitter->getShape() == ParticleEmitterShape::Sphere)
						{
							mtxCube = glm::scale(mtxCube, glm::vec3(emitter->getRadius()));
							Primitives::circledSphere(mtxCube, Color::Blue, overlayViewId, state | BGFX_STATE_PT_LINES, Renderer::getSingleton()->getTransparentProgram(), camera);
						}

						if (emitter->getShape() == ParticleEmitterShape::Circle)
						{
							mtxCube = glm::scale(mtxCube, glm::vec3(emitter->getRadius()));
							Primitives::circle(mtxCube, Color::Blue, overlayViewId, state | BGFX_STATE_PT_LINES, Renderer::getSingleton()->getTransparentProgram(), camera);
						}
					}

					if (showBounds)
					{
						uint64_t state = BGFX_STATE_WRITE_RGB
							| BGFX_STATE_WRITE_A
							| BGFX_STATE_WRITE_Z
							| BGFX_STATE_DEPTH_TEST_ALWAYS
							| BGFX_STATE_CULL_CW;

						for (int em = 0; em < ps->getEmitters().size(); ++em)
						{
							ParticleEmitter* emitter = ps->getEmitter(em);

							AxisAlignedBox bbox = emitter->getBounds();
							if (!bbox.isNull() && !bbox.isInfinite())
							{
								glm::mat4x4 mtxCube = glm::identity<glm::mat4x4>();
								mtxCube = glm::translate(mtxCube, bbox.getCenter());
								mtxCube = glm::scale(mtxCube, bbox.getHalfSize());
								Primitives::box(mtxCube, Color::White, viewId, state | BGFX_STATE_PT_LINES, Renderer::getSingleton()->getTransparentProgram(), camera);
							}
						}
					}
				}

				if (comp->getComponentType() == Water::COMPONENT_TYPE)
				{
					Water* rend = (Water*)comp;

					uint64_t state = BGFX_STATE_WRITE_RGB
						| BGFX_STATE_WRITE_A
						| BGFX_STATE_WRITE_Z
						| BGFX_STATE_DEPTH_TEST_ALWAYS
						| BGFX_STATE_CULL_CW;

					if (showBounds)
					{
						glm::mat4x4 mtxCube = glm::identity<glm::mat4x4>();
						mtxCube = glm::translate(mtxCube, rend->getBounds().getCenter());
						mtxCube = glm::scale(mtxCube, rend->getBounds().getHalfSize());
						Primitives::box(mtxCube, Color::White, viewId, state | BGFX_STATE_PT_LINES, Renderer::getSingleton()->getTransparentProgram(), camera);
					}
				}

				if (comp->getComponentType() == Terrain::COMPONENT_TYPE)
				{
					Terrain* terrain = (Terrain*)comp;
					uint64_t state = BGFX_STATE_WRITE_RGB
						| BGFX_STATE_WRITE_A
						| BGFX_STATE_WRITE_Z
						| BGFX_STATE_DEPTH_TEST_ALWAYS
						| BGFX_STATE_CULL_CW;

					uint64_t stateTerrain = BGFX_STATE_WRITE_RGB
						| BGFX_STATE_WRITE_A
						| BGFX_STATE_WRITE_Z
						| BGFX_STATE_DEPTH_TEST_LEQUAL
						| BGFX_STATE_CULL_CW;

					if (showBounds)
					{
						glm::mat4x4 mtxCube = glm::identity<glm::mat4x4>();
						AxisAlignedBox terrAab = terrain->getBounds();
						if (!terrAab.isNull() && !terrAab.isInfinite())
						{
							mtxCube = glm::translate(mtxCube, terrAab.getCenter());
							mtxCube = glm::scale(mtxCube, terrAab.getHalfSize());
							Primitives::box(mtxCube, Color::White, viewId, state | BGFX_STATE_PT_LINES, Renderer::getSingleton()->getTransparentProgram(), camera);
						}

						//Draw grass batches bounds
						if (showTerrainGrassBounds)
						{
							std::vector<TerrainGrassData*>& grass = terrain->getGrass();
							for (auto g = grass.begin(); g != grass.end(); ++g)
							{
								const std::vector<TerrainGrassData::Batch*>& batches = (*g)->getBatches();
								for (auto b = batches.begin(); b != batches.end(); ++b)
								{
									mtxCube = glm::identity<glm::mat4x4>();
									AxisAlignedBox aab = (*b)->getBounds();
									if (aab.isNull() || aab.isInfinite())
										continue;
									mtxCube = glm::translate(mtxCube, aab.getCenter());
									mtxCube = glm::scale(mtxCube, aab.getHalfSize());
									Primitives::box(mtxCube, Color::White, viewId, stateTerrain | BGFX_STATE_PT_LINES, Renderer::getSingleton()->getTransparentProgram(), camera);
								}
							}
						}

						//Draw trees bounds
						if (showTerrainTreesBounds)
						{
							std::vector<TerrainTreeData*>& trees = terrain->getTrees();
							for (auto t = trees.begin(); t != trees.end(); ++t)
							{
								const std::vector<TerrainTreeData::TreeMesh*>& meshes = (*t)->getMeshes();
								for (auto m = meshes.begin(); m != meshes.end(); ++m)
								{
									mtxCube = glm::identity<glm::mat4x4>();
									AxisAlignedBox aab = (*m)->getBounds();
									if (aab.isNull() || aab.isInfinite())
										continue;
									mtxCube = glm::translate(mtxCube, aab.getCenter());
									mtxCube = glm::scale(mtxCube, aab.getHalfSize());
									Primitives::box(mtxCube, Color::White, viewId, stateTerrain | BGFX_STATE_PT_LINES, Renderer::getSingleton()->getTransparentProgram(), camera);
								}
							}
						}

						//Draw detail meshes bounds
						if (showTerrainDetailMeshesBounds)
						{
							std::vector<TerrainDetailMeshData*>& meshes = terrain->getDetailMeshes();
							for (auto m = meshes.begin(); m != meshes.end(); ++m)
							{
								const std::vector<TerrainDetailMeshData::Batch*>& batches = (*m)->getBatches();
								for (auto b = batches.begin(); b != batches.end(); ++b)
								{
									mtxCube = glm::identity<glm::mat4x4>();
									AxisAlignedBox aab = (*b)->getBounds();
									if (aab.isNull() || aab.isInfinite())
										continue;
									mtxCube = glm::translate(mtxCube, aab.getCenter());
									mtxCube = glm::scale(mtxCube, aab.getHalfSize());
									Primitives::box(mtxCube, Color::White, viewId, stateTerrain | BGFX_STATE_PT_LINES, Renderer::getSingleton()->getTransparentProgram(), camera);
								}
							}
						}
					}
				}

				if (comp->getComponentType() == BoxCollider::COMPONENT_TYPE)
				{
					BoxCollider* col = (BoxCollider*)comp;
					
					glm::mat4x4 mtx = glm::identity<glm::mat4x4>();
					glm::mat4x4 rotMat = glm::mat4_cast(t->getRotation() * col->getRotation());
					mtx = glm::translate(mtx, t->getPosition() + (col->getOffset() * t->getScale()) * glm::inverse(glm::mat3x3(t->getRotation())));
					mtx *= rotMat;
					mtx = glm::scale(mtx, t->getScale() * col->getBoxSize());
					Primitives::box(mtx, Color::Green, overlayViewId, colliderState | BGFX_STATE_PT_LINES, Renderer::getSingleton()->getSimpleProgram(), camera);
				}

				if (comp->getComponentType() == SphereCollider::COMPONENT_TYPE)
				{
					SphereCollider* col = (SphereCollider*)comp;

					glm::mat4x4 mtx = glm::identity<glm::mat4x4>();
					glm::mat4x4 rotMat = glm::mat4_cast(t->getRotation() * col->getRotation());
					mtx = glm::translate(mtx, t->getPosition() + (col->getOffset() * t->getScale()) * glm::inverse(glm::mat3x3(t->getRotation())));
					mtx *= rotMat;
					mtx = glm::scale(mtx, t->getScale() * col->getRadius());
					Primitives::circledSphere(mtx, Color::Green, overlayViewId, colliderState | BGFX_STATE_PT_LINES, Renderer::getSingleton()->getSimpleProgram(), camera);
				}

				if (comp->getComponentType() == CapsuleCollider::COMPONENT_TYPE)
				{
					CapsuleCollider* col = (CapsuleCollider*)comp;

					glm::mat4x4 mtx = glm::identity<glm::mat4x4>();
					glm::mat4x4 rotMat = glm::mat4_cast(t->getRotation() * col->getRotation());
					mtx = glm::translate(mtx, t->getPosition() + (col->getOffset() * t->getScale()) * glm::inverse(glm::mat3x3(t->getRotation())));
					mtx *= rotMat;
					mtx = glm::scale(mtx, t->getScale());
					Primitives::circledCapsule(mtx, col->getHeight() + col->getRadius() * 2.0f, col->getRadius(), Color::Green, overlayViewId, colliderState | BGFX_STATE_PT_LINES, Renderer::getSingleton()->getSimpleProgram(), camera);
				}

				if (comp->getComponentType() == FixedJoint::COMPONENT_TYPE)
				{
					FixedJoint* joint = (FixedJoint*)comp;

					GameObject* connectedObject = Engine::getSingleton()->getGameObject(joint->getConnectedObjectGuid());
					if (connectedObject != nullptr)
					{
						glm::mat4x4 mtx = glm::identity<glm::mat4x4>();
						mtx = glm::translate(mtx, connectedObject->getTransform()->getPosition() + (glm::mat3x3(connectedObject->getTransform()->getTransformMatrix()) * joint->getConnectedAnchor()));
						mtx = glm::scale(mtx, glm::vec3(1));
						Primitives::point(mtx, 40, Color(1, 1, 0, 1), overlayViewId, colliderState | BGFX_STATE_PT_POINTS, Renderer::getSingleton()->getSimpleProgram(), camera);

						mtx = glm::identity<glm::mat4x4>();
						mtx = glm::translate(mtx, t->getPosition() + (glm::mat3x3(t->getTransformMatrix()) * joint->getAnchor()));
						mtx = glm::scale(mtx, glm::vec3(1));
						Primitives::point(mtx, 40, Color(1, 1, 0, 1), overlayViewId, colliderState | BGFX_STATE_PT_POINTS, Renderer::getSingleton()->getSimpleProgram(), camera);
					}
				}

				if (comp->getComponentType() == HingeJoint::COMPONENT_TYPE)
				{
					HingeJoint* joint = (HingeJoint*)comp;

					GameObject* connectedObject = Engine::getSingleton()->getGameObject(joint->getConnectedObjectGuid());
					if (connectedObject != nullptr)
					{
						glm::mat4x4 mtx = glm::identity<glm::mat4x4>();
						mtx = glm::translate(mtx, connectedObject->getTransform()->getPosition() + (glm::mat3x3(connectedObject->getTransform()->getTransformMatrix()) * joint->getConnectedAnchor()));
						mtx = glm::scale(mtx, glm::vec3(1));
						Primitives::point(mtx, 40, Color(1, 1, 0, 1), overlayViewId, colliderState | BGFX_STATE_PT_POINTS, Renderer::getSingleton()->getSimpleProgram(), camera);

						mtx = glm::identity<glm::mat4x4>();
						mtx = glm::translate(mtx, t->getPosition() + (glm::mat3x3(t->getTransformMatrix()) * joint->getAnchor()));
						mtx = glm::scale(mtx, glm::vec3(1));
						Primitives::point(mtx, 40, Color(1, 1, 0, 1), overlayViewId, colliderState | BGFX_STATE_PT_POINTS, Renderer::getSingleton()->getSimpleProgram(), camera);
					}
				}

				if (comp->getComponentType() == FreeJoint::COMPONENT_TYPE)
				{
					FreeJoint* joint = (FreeJoint*)comp;

					GameObject* connectedObject = Engine::getSingleton()->getGameObject(joint->getConnectedObjectGuid());
					if (connectedObject != nullptr)
					{
						glm::mat4x4 mtx = glm::identity<glm::mat4x4>();
						mtx = glm::translate(mtx, connectedObject->getTransform()->getPosition() + (glm::mat3x3(connectedObject->getTransform()->getTransformMatrix()) * joint->getConnectedAnchor()));
						mtx = glm::scale(mtx, glm::vec3(1));
						Primitives::point(mtx, 40, Color(1, 1, 0, 1), overlayViewId, colliderState | BGFX_STATE_PT_POINTS, Renderer::getSingleton()->getSimpleProgram(), camera);

						mtx = glm::identity<glm::mat4x4>();
						mtx = glm::translate(mtx, t->getPosition() + (glm::mat3x3(t->getTransformMatrix()) * joint->getAnchor()));
						mtx = glm::scale(mtx, glm::vec3(1));
						Primitives::point(mtx, 40, Color(1, 1, 0, 1), overlayViewId, colliderState | BGFX_STATE_PT_POINTS, Renderer::getSingleton()->getSimpleProgram(), camera);
					}
				}

				if (comp->getComponentType() == ConeTwistJoint::COMPONENT_TYPE)
				{
					ConeTwistJoint* joint = (ConeTwistJoint*)comp;

					GameObject* connectedObject = Engine::getSingleton()->getGameObject(joint->getConnectedObjectGuid());
					if (connectedObject != nullptr)
					{
						glm::mat4x4 mtx = glm::identity<glm::mat4x4>();
						mtx = glm::translate(mtx, connectedObject->getTransform()->getPosition() + (glm::mat3x3(connectedObject->getTransform()->getTransformMatrix()) * joint->getConnectedAnchor()));
						mtx = glm::scale(mtx, glm::vec3(1));
						Primitives::point(mtx, 40, Color(1, 1, 0, 1), overlayViewId, colliderState | BGFX_STATE_PT_POINTS, Renderer::getSingleton()->getSimpleProgram(), camera);

						mtx = glm::identity<glm::mat4x4>();
						mtx = glm::translate(mtx, t->getPosition() + (glm::mat3x3(t->getTransformMatrix()) * joint->getAnchor()));
						mtx = glm::scale(mtx, glm::vec3(1));
						Primitives::point(mtx, 40, Color(1, 1, 0, 1), overlayViewId, colliderState | BGFX_STATE_PT_POINTS, Renderer::getSingleton()->getSimpleProgram(), camera);
					}
				}

				if (comp->getComponentType() == NavMeshObstacle::COMPONENT_TYPE)
				{
					NavMeshObstacle* obst = (NavMeshObstacle*)comp;

					glm::mat4x4 mtx = glm::identity<glm::mat4x4>();
					glm::mat4x4 rotMat = glm::mat4_cast(t->getRotation());
					mtx = glm::translate(mtx, t->getPosition() + (obst->getOffset() * t->getScale()) * glm::inverse(glm::mat3x3(t->getRotation())));
					mtx *= rotMat;
					mtx = glm::scale(mtx, t->getScale() * obst->getSize());
					Primitives::box(mtx, Color(0, 1, 1, 1), overlayViewId, colliderState | BGFX_STATE_PT_LINES, Renderer::getSingleton()->getTransparentProgram(), camera);
				}

				if (comp->getComponentType() == Vehicle::COMPONENT_TYPE)
				{
					Vehicle* vehicle = (Vehicle*)comp;

					std::vector<Vehicle::WheelInfo>& wheels = vehicle->getWheels();

					for (auto it = wheels.begin(); it != wheels.end(); ++it)
					{
						auto wheel = *it;

						GameObject* connectedNode = nullptr;
						Transform* connectedTrans = nullptr;
						connectedNode = Engine::getSingleton()->getGameObject(wheel.m_connectedObjectGuid);

						if (connectedNode != nullptr)
						{
							//Center point
							glm::mat4x4 mtx = t->getTransformMatrix();
							mtx = glm::translate(mtx, wheel.m_connectionPoint);
							Primitives::point(mtx, 10, Color(0, 1, 1, 1), overlayViewId, colliderState | BGFX_STATE_PT_LINES, Renderer::getSingleton()->getTransparentProgram(), camera);

							//Direction
							mtx = t->getTransformMatrix();
							glm::vec3 pt = wheel.m_connectionPoint;
							Primitives::line(mtx, pt, pt + wheel.m_direction * wheel.m_suspensionRestLength, Color(0, 1, 1, 1), overlayViewId, colliderState | BGFX_STATE_PT_LINES, Renderer::getSingleton()->getTransparentProgram(), camera);

							//Circle
							glm::vec3 dir = wheel.m_axle;
							dir = glm::normalize(dir);
							glm::vec3 right = glm::vec3(dir.z, 0, -dir.x);
							right = glm::normalize(right);
							glm::vec3 up = glm::cross(dir, right);
							glm::highp_quat quat = Mathf::fromAxes(right, up, dir);

							mtx = t->getTransformMatrix();
							mtx = glm::translate(mtx, wheel.m_connectionPoint);
							mtx *= glm::mat4_cast(quat) * mat4_cast(Mathf::toQuaternion(glm::vec3(-90, 0, 0)));
							mtx = glm::scale(mtx, glm::vec3(wheel.m_radius));
							Primitives::circle(mtx, Color(0, 1, 1, 1), overlayViewId, colliderState | BGFX_STATE_PT_LINES, Renderer::getSingleton()->getTransparentProgram(), camera);
						}
					}
				}

				if (comp->getComponentType() == CSGBrush::COMPONENT_TYPE)
				{
					CSGBrush* brush = (CSGBrush*)comp;

					glm::mat4x4 mtx = glm::identity<glm::mat4x4>();
					glm::mat4x4 rotMat = glm::mat4_cast(t->getRotation());
					mtx = glm::translate(mtx, t->getPosition());
					mtx *= rotMat;
					mtx = glm::scale(mtx, t->getScale());
					
					auto& vertices = brush->getVertices();
					auto& faces = brush->getFaces();

					std::vector<glm::vec3> points;

					for (auto& face : faces)
					{
						if (face.indices.size() == 3)
						{
							glm::vec3 p1 = vertices[face.indices[0]];
							glm::vec3 p2 = vertices[face.indices[1]];
							glm::vec3 p3 = vertices[face.indices[2]];

							points.push_back(p1);
							points.push_back(p2);
							points.push_back(p2);
							points.push_back(p3);
							points.push_back(p3);
							points.push_back(p1);
						}

						if (face.indices.size() == 4)
						{
							glm::vec3 p1 = vertices[face.indices[0]];
							glm::vec3 p2 = vertices[face.indices[1]];
							glm::vec3 p3 = vertices[face.indices[2]];
							glm::vec3 p4 = vertices[face.indices[3]];

							points.push_back(p1);
							points.push_back(p2);
							points.push_back(p2);
							points.push_back(p3);
							points.push_back(p3);
							points.push_back(p4);
							points.push_back(p4);
							points.push_back(p1);
						}
					}

					Primitives::mesh(mtx, points, Color::Blue, overlayViewId, csgState | BGFX_STATE_PT_LINES, Renderer::getSingleton()->getSimpleProgram(), camera);

					points.clear();
				}
			}

			int j = 0;
			for (auto it = child->getChildren().begin(); it != child->getChildren().end(); ++it, ++j)
			{
				Transform* ch = *it;
				nstack.insert(nstack.begin() + j, ch);
			}
		}
	}
}