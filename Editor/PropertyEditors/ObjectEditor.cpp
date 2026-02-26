#include "ObjectEditor.h"

#include "imgui.h"
#include "imgui_internal.h"

#include "../Classes/TreeView.h"

#include "TransformEditor.h"
#include "MeshRendererEditor.h"
#include "DecalRendererEditor.h"
#include "LightEditor.h"
#include "CameraEditor.h"
#include "AnimationEditor.h"
#include "RigidBodyEditor.h"
#include "AudioSourceEditor.h"
#include "AudioListenerEditor.h"
#include "MeshColliderEditor.h"
#include "CapsuleColliderEditor.h"
#include "BoxColliderEditor.h"
#include "SphereColliderEditor.h"
#include "TerrainColliderEditor.h"
#include "FixedJointEditor.h"
#include "HingeJointEditor.h"
#include "ConeTwistJointEditor.h"
#include "FreeJointEditor.h"
#include "NavMeshAgentEditor.h"
#include "NavMeshObstacleEditor.h"
#include "VehicleEditor.h"
#include "CanvasEditor.h"
#include "ImageEditor.h"
#include "TextEditor.h"
#include "ButtonEditor.h"
#include "TextInputEditor.h"
#include "MaskEditor.h"
#include "MonoScriptEditor.h"
#include "TerrainEditor.h"
#include "ParticleSystemEditor.h"
#include "WaterEditor.h"
#include "SplineEditor.h"
#include "VideoPlayerEditor.h"
#include "CSGModelEditor.h"
#include "CSGBrushEditor.h"

#include "Property.h"
#include "PropEditorHost.h"
#include "PropVector2.h"
#include "PropVector3.h"
#include "PropVector4.h"
#include "PropString.h"
#include "PropBool.h"
#include "PropInt.h"
#include "PropFloat.h"
#include "PropGameObject.h"
#include "PropComboBox.h"

#include "../Classes/Undo.h"

#include "../Windows/MainWindow.h"
#include "../Windows/InspectorWindow.h"
#include "../Windows/HierarchyWindow.h"
#include "../Windows/SceneWindow.h"
#include "../Windows/UIEditorWindow.h"

#include "../Engine/Core/GameObject.h"
#include "../Engine/Assets/Texture.h"

#include "../Engine/Core/Engine.h"
#include "../Engine/Classes/StringConverter.h"
#include "../Engine/Classes/IO.h"
#include "../Engine/Classes/VectorUtils.h"
#include "../Engine/Classes/Helpers.h"
#include "../Engine/Gizmo/Gizmo.h"
#include "../Engine/Core/NavigationManager.h"
#include "../Engine/Core/APIManager.h"
#include "../Engine/Renderer/BatchedGeometry.h"
#include "../Engine/Renderer/CSGGeometry.h"

#include "../Engine/Components/Component.h"
#include "../Engine/Components/MeshRenderer.h"
#include "../Engine/Components/DecalRenderer.h"
#include "../Engine/Components/Light.h"
#include "../Engine/Components/Camera.h"
#include "../Engine/Components/Animation.h"
#include "../Engine/Components/Transform.h"
#include "../Engine/Components/AudioSource.h"
#include "../Engine/Components/AudioListener.h"
#include "../Engine/Components/RigidBody.h"
#include "../Engine/Components/MeshCollider.h"
#include "../Engine/Components/CapsuleCollider.h"
#include "../Engine/Components/BoxCollider.h"
#include "../Engine/Components/SphereCollider.h"
#include "../Engine/Components/TerrainCollider.h"
#include "../Engine/Components/FixedJoint.h"
#include "../Engine/Components/HingeJoint.h"
#include "../Engine/Components/ConeTwistJoint.h"
#include "../Engine/Components/FreeJoint.h"
#include "../Engine/Components/NavMeshAgent.h"
#include "../Engine/Components/NavMeshObstacle.h"
#include "../Engine/Components/Vehicle.h"
#include "../Engine/Components/Canvas.h"
#include "../Engine/Components/Image.h"
#include "../Engine/Components/Text.h"
#include "../Engine/Components/Button.h"
#include "../Engine/Components/TextInput.h"
#include "../Engine/Components/Mask.h"
#include "../Engine/Components/MonoScript.h"
#include "../Engine/Components/Terrain.h"
#include "../Engine/Components/ParticleSystem.h"
#include "../Engine/Components/Water.h"
#include "../Engine/Components/Spline.h"
#include "../Engine/Components/VideoPlayer.h"
#include "../Engine/Components/CSGBrush.h"
#include "../Engine/Components/CSGModel.h"

#include "../Engine/Math/Mathf.h"

namespace GX
{
	Component* ObjectEditor::bufferComponent = nullptr;

	bool ObjectEditor::bufferTransform = false;

	glm::vec3 ObjectEditor::bufferPosition = glm::vec3(FLT_MAX);
	glm::quat ObjectEditor::bufferRotation = glm::quat(FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX);
	glm::vec3 ObjectEditor::bufferScale = glm::vec3(FLT_MAX);

	glm::vec3 ObjectEditor::bufferLocalPosition = glm::vec3(FLT_MAX);
	glm::quat ObjectEditor::bufferLocalRotation = glm::quat(FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX);
	glm::vec3 ObjectEditor::bufferLocalScale = glm::vec3(FLT_MAX);

	std::vector<std::string> multipleComponentsAllowed = { "BoxCollider", "SphereCollider", "CapsuleCollider", "NavMeshObstacle", "MonoScript"};

	ObjectEditor::ObjectEditor()
	{
		setEditorName("ObjectEditor");
		treeView->setAllowReorder(true);
		treeView->setOnReorderCallback([=](TreeNode* node, int val) { onReorder(node, val); });
	}

	ObjectEditor::~ObjectEditor()
	{

	}

	void ObjectEditor::init(std::vector<GameObject*> nodes)
	{
		objects = nodes;
		GameObject* currentNode = nodes[0];

		ProjectSettings* projectSettings = Engine::getSingleton()->getSettings();

		std::vector<std::string> tags = projectSettings->getTags();
		std::vector<std::string> layers = projectSettings->getLayers();

		int _tag = currentNode->getTag();
		int _layer = currentNode->getLayer();

		tags.insert(tags.begin(), "Untagged");
		layers.insert(layers.begin(), "Default");

		if (_tag < 0 || _tag > tags.size() - 1)
			_tag = 0;

		if (_layer < 0 || _layer > layers.size() - 1)
			_layer = 0;

		/* BASE */
		PropBool* enabled = new PropBool(this, "Enabled", currentNode->getEnabled()); // Global enabled
		enabled->setOnChangeCallback([=](Property* prop, bool val) { onChangeEnabled(prop, val); });

		PropString* name = new PropString(this, "Name", CP_UNI(currentNode->getName()));
		name->setOnChangeCallback([=](Property* prop, std::string val) { onChangeName(prop, val); });

		PropComboBox* tag = new PropComboBox(this, "Tag", tags);
		tag->setCurrentItem(_tag);
		tag->setOnChangeCallback([=](Property* prop, int val) { onChangeTag(prop, val); });

		PropComboBox* layer = new PropComboBox(this, "Layer", layers);
		layer->setCurrentItem(_layer);
		layer->setOnChangeCallback([=](Property* prop, int val) { onChangeLayer(prop, val); });

		std::string allNone = "All";
		if (currentNode->getLightingStatic() ||
			currentNode->getNavigationStatic() ||
			currentNode->getBatchingStatic() ||
			currentNode->getOcclusionStatic())
			allNone = "None";

		std::string labelStatic = "No";

		if (allNone == "None") labelStatic = "Yes";

		PropComboBox* _static = new PropComboBox(this, "Static", { allNone, "Lighting static", "Navigation static", "Batching static", "Occlusion static"});
		_static->setLabel(labelStatic);
		_static->setValueEnabled({
			false,
			currentNode->getLightingStatic(),
			currentNode->getNavigationStatic(),
			currentNode->getBatchingStatic(),
			currentNode->getOcclusionStatic()});

		_static->setOnChangeCallback([=](Property* prop, int val) { onChangeStatic(prop, val); });

		addProperty(enabled);
		addProperty(name);
		addProperty(tag);
		addProperty(layer);
		addProperty(_static);
		/* BASE */

		transformEditor = nullptr;
		uiElementEditor = nullptr;

		/* COMPONENTS */
		listComponents();
		/* COMPONENTS */
	}

	void ObjectEditor::update()
	{
		PropertyEditor::update();

		ImGui::BeginChild("AddComponentButton", ImVec2(0, 55));

		std::string text = "Add component";
		ImVec2 rg = ImGui::GetContentRegionAvail();
		ImVec2 ts = ImGui::CalcTextSize(text.c_str());
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (rg.x * 0.5f) - (ts.x * 0.5f) - ImGui::GetScrollX());
		ImGui::SetCursorPosY(16);
		if (ImGui::Button(text.c_str(), ImVec2(150, 30)))
		{
			ImGui::OpenPopup("AddComponent_popup");
		}

		if (ImGui::BeginPopup("AddComponent_popup"))
		{
			MainWindow::getSingleton()->updateComponentsMenu();
			ImGui::EndPopup();
		}

		ImGui::EndChild();

		const ImGuiPayload* payload = ImGui::GetDragDropPayload();
		if (payload != nullptr)
		{
			if (payload->Data != nullptr && payload->IsDataType("DND_TreeView"))
			{
				std::string ext = IO::GetFileExtension(((TreeNode*)payload->Data)->alias);
				if (ext == "cs")
				{
					ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(1.0, 1.0, 1.0, 0.1));
					ImGui::BeginChild("DND_Component", ImVec2(0, 50), true);
					std::string text = "Drag and drop script here";
					ImVec2 rg = ImGui::GetContentRegionAvail();
					ImVec2 ts = ImGui::CalcTextSize(text.c_str());
					ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (rg.x * 0.5f) - (ts.x * 0.5f) - ImGui::GetScrollX());
					ImGui::SetCursorPosY(16);
					ImGui::Text(text.c_str());
					ImGui::EndChild();
					ImGui::PopStyleColor();

					if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_TreeView"))
						{
							TreeNode* node = (TreeNode*)payload->Data;
							std::string path = node->getPath();
							std::string className = IO::GetFileName(path);

							std::vector<Component*> comps;

							for (auto it = objects.begin(); it != objects.end(); ++it)
							{
								GameObject* obj = *it;
								MonoScript* script = new MonoScript();
								script->setClassFromName(className);
								obj->addComponent(script);

								comps.push_back(script);
							}

							MainWindow::getSingleton()->addComponentUndo(comps);

							updateEditor();
						}
					}
				}
			}
		}
	}

	void ObjectEditor::updateEditor()
	{
		MainWindow::addOnEndUpdateCallback([=]() {
			float sp = MainWindow::getInspectorWindow()->getScrollPos();
			MainWindow::getInspectorWindow()->saveCollapsedProperties();

			auto props = getTreeView()->getRootNode()->children;

			for (auto it = props.begin(); it != props.end(); ++it)
				removeProperty((Property*)*it);

			init(objects);

			MainWindow::getInspectorWindow()->loadCollapsedProperties();
			MainWindow::getInspectorWindow()->setScrollPos(sp);
		});
	}

	void ObjectEditor::resetBufferObjects()
	{
		bufferComponent = nullptr;
		bufferTransform = false;
		bufferPosition = glm::vec3(FLT_MAX);
		bufferScale = glm::vec3(FLT_MAX);
		bufferRotation = glm::quat(FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX);
	}

	void ObjectEditor::updateTransform()
	{
		if (transformEditor != nullptr)
			((TransformEditor*)transformEditor)->updateTransform();
		if (uiElementEditor != nullptr)
			((UIElementEditor*)uiElementEditor)->updateValues();
	}

	void ObjectEditor::onReorder(TreeNode* node, int newIndex)
	{
		if (objects.size() == 0)
			return;

		GameObject* obj = objects[0];
		auto& comps = obj->getComponents();

		bool equals = true;
		for (auto& o : objects)
		{
			if (o->getComponents().size() != comps.size())
			{
				equals = false;
				break;
			}
		}

		if (!equals)
		{
			updateEditor();
			return;
		}

		int offset = 0;
		TreeNode* rootNode = treeView->getRootNode();
		for (auto& n : rootNode->children)
		{
			if (!n->enableDrag)
				++offset;
			else
				break;
		}

		int oldIdx = *static_cast<int*>(node->getUserData());
		
		if (comps[oldIdx]->getComponentType() == Transform::COMPONENT_TYPE)
		{
			updateEditor();
			return;
		}

		//Undo
		UndoData* undoData = Undo::addUndo("Reorder components");
		undoData->intData.resize(4);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->intData[0])
			{
				GameObject* object = (GameObject*)d.first;
				auto& comps = object->getComponents();

				int oldId = d.second;
				int newId = data->intData[1][object];

				MainWindow::addOnEndUpdateCallback([=, &comps]()
					{
						VectorUtils::move(comps, oldId, newId);
					}
				);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->intData[2])
			{
				GameObject* object = (GameObject*)d.first;
				auto& comps = object->getComponents();

				int oldId = d.second;
				int newId = data->intData[3][object];

				MainWindow::addOnEndUpdateCallback([=, &comps]()
					{
						VectorUtils::move(comps, oldId, newId);
					}
				);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		undoData->intData[0][obj] = newIndex - offset;
		undoData->intData[1][obj] = oldIdx;
		undoData->intData[2][obj] = oldIdx;
		undoData->intData[3][obj] = newIndex - offset;

		MainWindow::addOnEndUpdateCallback([=, &comps]()
			{
				VectorUtils::move(comps, oldIdx, newIndex - offset);
			}
		);
	}

	std::vector<Component*> ObjectEditor::getObjectsComponents(Component* base)
	{
		std::vector<Component*> comps;
		std::vector<Component*>& _components = objects[0]->getComponents();

		int cidx = 0;
		for (auto in = _components.begin(); in < _components.end(); ++in)
		{
			if (*in == base)
				break;

			if ((*in)->getComponentType() == base->getComponentType())
			{
				if (!(*in)->isEqualsTo(base))
					continue;

				if ((*in)->getComponentType() == MonoScript::COMPONENT_TYPE)
				{
					if (((MonoScript*)(*in))->getManagedClass() == ((MonoScript*)base)->getManagedClass())
						++cidx;
				}
				else
					++cidx;
			}
		}

		std::vector<Component*> fcomps;
		if (objects.size() > 1)
		{
			for (auto ob = objects.begin() + 1; ob != objects.end(); ++ob)
			{
				int ocidx = 0;
				std::vector<Component*>& obcomps = (*ob)->getComponents();
				for (auto oc = obcomps.begin(); oc != obcomps.end(); ++oc)
				{
					if ((*oc)->getComponentType() == base->getComponentType())
					{
						if (!(*oc)->isEqualsTo(base))
							continue;

						bool next = (*oc)->getComponentType() != MonoScript::COMPONENT_TYPE;

						if (!next)
						{
							if (((MonoScript*)(*oc))->getManagedClass() == ((MonoScript*)base)->getManagedClass())
								next = true;
						}

						if (next)
						{
							if (ocidx == cidx)
							{
								fcomps.push_back(*oc);
								break;
							}

							++ocidx;
						}
					}
				}
			}
		}

		if (fcomps.size() == objects.size() - 1)
		{
			comps.push_back(base);
			for (auto it = fcomps.begin(); it != fcomps.end(); ++it)
				comps.push_back(*it);
		}

		fcomps.clear();

		return comps;
	}

	void ObjectEditor::listComponents()
	{
		GameObject* obj = objects[0];

		int i = 0;
		std::vector<Component*>& _components = obj->getComponents();
		for (auto it = _components.begin(); it < _components.end(); ++it, ++i)
		{
			PropEditorHost* component = nullptr;
			PropertyEditor* hostedEditor = nullptr;
			Component* comp = *it;

			std::vector<Component*> comps = getObjectsComponents(comp);
			if (comps.size() == 0)
				continue;

			/* TRANSFORM */
			if (comp->getComponentType() == Transform::COMPONENT_TYPE)
			{
				TransformEditor* ed = new TransformEditor();

				transformEditor = ed;

				component = new PropEditorHost(this, Transform::COMPONENT_TYPE, ed);
				component->icon = Texture::load(Helper::ExePath(), "Editor/Icons/Inspector/transform.png", false, Texture::CompressionMethod::None, true);
				hostedEditor = ed;
			}
			/* TRANSFORM */

			/* MESH RENDERER */
			if (comp->getComponentType() == MeshRenderer::COMPONENT_TYPE)
			{
				MeshRendererEditor* ed = new MeshRendererEditor();

				component = new PropEditorHost(this, MeshRenderer::COMPONENT_TYPE, ed);
				component->icon = Texture::load(Helper::ExePath(), "Editor/Icons/Hierarchy/mesh_renderer.png", false, Texture::CompressionMethod::None, true);
				hostedEditor = ed;
			}
			/* MESH RENDERER */

			/* DECAL RENDERER */
			if (comp->getComponentType() == DecalRenderer::COMPONENT_TYPE)
			{
				DecalRendererEditor* ed = new DecalRendererEditor();

				component = new PropEditorHost(this, DecalRenderer::COMPONENT_TYPE, ed);
				component->icon = Texture::load(Helper::ExePath(), "Editor/Icons/Hierarchy/decal_renderer.png", false, Texture::CompressionMethod::None, true);
				hostedEditor = ed;
			}
			/* MESH RENDERER */

			/* LIGHT */
			if (comp->getComponentType() == Light::COMPONENT_TYPE)
			{
				LightEditor* ed = new LightEditor();

				component = new PropEditorHost(this, Light::COMPONENT_TYPE, ed);
				component->icon = Texture::load(Helper::ExePath(), "Editor/Icons/Hierarchy/light.png", false, Texture::CompressionMethod::None, true);
				hostedEditor = ed;
			}
			/* LIGHT */

			/* CAMERA */
			if (comp->getComponentType() == Camera::COMPONENT_TYPE)
			{
				CameraEditor* ed = new CameraEditor();

				component = new PropEditorHost(this, Camera::COMPONENT_TYPE, ed);
				component->icon = Texture::load(Helper::ExePath(), "Editor/Icons/Hierarchy/camera.png", false, Texture::CompressionMethod::None, true);
				hostedEditor = ed;
			}
			/* CAMERA */

			/* ANIMATION */
			if (comp->getComponentType() == Animation::COMPONENT_TYPE)
			{
				AnimationEditor* ed = new AnimationEditor();

				component = new PropEditorHost(this, Animation::COMPONENT_TYPE, ed);
				component->icon = Texture::load(Helper::ExePath(), "Editor/Icons/Inspector/animation.png", false, Texture::CompressionMethod::None, true);
				hostedEditor = ed;
			}
			/* ANIMATION */

			/* RIGIDBODY */
			if (comp->getComponentType() == RigidBody::COMPONENT_TYPE)
			{
				RigidBodyEditor* ed = new RigidBodyEditor();

				component = new PropEditorHost(this, RigidBody::COMPONENT_TYPE, ed);
				component->icon = Texture::load(Helper::ExePath(), "Editor/Icons/Inspector/rigidbody.png", false, Texture::CompressionMethod::None, true);
				hostedEditor = ed;
			}
			/* RIGIDBODY */

			/* BOXCOLLIDER */
			if (comp->getComponentType() == BoxCollider::COMPONENT_TYPE)
			{
				BoxColliderEditor* ed = new BoxColliderEditor();

				component = new PropEditorHost(this, BoxCollider::COMPONENT_TYPE, ed);
				component->icon = Texture::load(Helper::ExePath(), "Editor/Icons/Inspector/box_collider.png", false, Texture::CompressionMethod::None, true);
				hostedEditor = ed;
			}
			/* BOXCOLLIDER */

			/* CAPSULECOLLIDER */
			if (comp->getComponentType() == CapsuleCollider::COMPONENT_TYPE)
			{
				CapsuleColliderEditor* ed = new CapsuleColliderEditor();

				component = new PropEditorHost(this, CapsuleCollider::COMPONENT_TYPE, ed);
				component->icon = Texture::load(Helper::ExePath(), "Editor/Icons/Inspector/capsule_collider.png", false, Texture::CompressionMethod::None, true);
				hostedEditor = ed;
			}
			/* CAPSULECOLLIDER */

			/* SPHERECOLLIDER */
			if (comp->getComponentType() == SphereCollider::COMPONENT_TYPE)
			{
				SphereColliderEditor* ed = new SphereColliderEditor();

				component = new PropEditorHost(this, SphereCollider::COMPONENT_TYPE, ed);
				component->icon = Texture::load(Helper::ExePath(), "Editor/Icons/Inspector/sphere_collider.png", false, Texture::CompressionMethod::None, true);
				hostedEditor = ed;
			}
			/* SPHERECOLLIDER */

			/* MESHCOLLIDER */
			if (comp->getComponentType() == MeshCollider::COMPONENT_TYPE)
			{
				MeshColliderEditor* ed = new MeshColliderEditor();

				component = new PropEditorHost(this, MeshCollider::COMPONENT_TYPE, ed);
				component->icon = Texture::load(Helper::ExePath(), "Editor/Icons/Inspector/mesh_collider.png", false, Texture::CompressionMethod::None, true);
				hostedEditor = ed;
			}
			/* MESHCOLLIDER */

			/* TERRAINCOLLIDER */
			if (comp->getComponentType() == TerrainCollider::COMPONENT_TYPE)
			{
				TerrainColliderEditor* ed = new TerrainColliderEditor();

				component = new PropEditorHost(this, TerrainCollider::COMPONENT_TYPE, ed);
				component->icon = Texture::load(Helper::ExePath(), "Editor/Icons/Inspector/terrain_collider.png", false, Texture::CompressionMethod::None, true);
				hostedEditor = ed;
			}
			/* TERRAINCOLLIDER */

			/* FIXEDJOINT */
			if (comp->getComponentType() == FixedJoint::COMPONENT_TYPE)
			{
				FixedJointEditor* ed = new FixedJointEditor();

				component = new PropEditorHost(this, FixedJoint::COMPONENT_TYPE, ed);
				component->icon = Texture::load(Helper::ExePath(), "Editor/Icons/Inspector/fixed_joint.png", false, Texture::CompressionMethod::None, true);
				hostedEditor = ed;
			}
			/* FIXEDJOINT */

			/* HINGEJOINT */
			if (comp->getComponentType() == HingeJoint::COMPONENT_TYPE)
			{
				HingeJointEditor* ed = new HingeJointEditor();

				component = new PropEditorHost(this, HingeJoint::COMPONENT_TYPE, ed);
				component->icon = Texture::load(Helper::ExePath(), "Editor/Icons/Inspector/hinge_joint.png", false, Texture::CompressionMethod::None, true);
				hostedEditor = ed;
			}
			/* HINGEJOINT */

			/* FREEJOINT */
			if (comp->getComponentType() == FreeJoint::COMPONENT_TYPE)
			{
				FreeJointEditor* ed = new FreeJointEditor();

				component = new PropEditorHost(this, FreeJoint::COMPONENT_TYPE, ed);
				component->icon = Texture::load(Helper::ExePath(), "Editor/Icons/Inspector/free_joint.png", false, Texture::CompressionMethod::None, true);
				hostedEditor = ed;
			}
			/* FREEJOINT */

			/* CONETWISTJOINT */
			if (comp->getComponentType() == ConeTwistJoint::COMPONENT_TYPE)
			{
				ConeTwistJointEditor* ed = new ConeTwistJointEditor();

				component = new PropEditorHost(this, ConeTwistJoint::COMPONENT_TYPE, ed);
				component->icon = Texture::load(Helper::ExePath(), "Editor/Icons/Inspector/hinge_joint.png", false, Texture::CompressionMethod::None, true);
				hostedEditor = ed;
			}
			/* CONETWISTJOINT */

			/* VEHICLE */
			if (comp->getComponentType() == Vehicle::COMPONENT_TYPE)
			{
				VehicleEditor* ed = new VehicleEditor();

				component = new PropEditorHost(this, Vehicle::COMPONENT_TYPE, ed);
				component->icon = Texture::load(Helper::ExePath(), "Editor/Icons/Inspector/vehicle.png", false, Texture::CompressionMethod::None, true);
				hostedEditor = ed;
			}
			/* VEHICLE */

			/* AUDIOLISTENER */
			if (comp->getComponentType() == AudioListener::COMPONENT_TYPE)
			{
				AudioListenerEditor* ed = new AudioListenerEditor();

				component = new PropEditorHost(this, AudioListener::COMPONENT_TYPE, ed);
				component->icon = Texture::load(Helper::ExePath(), "Editor/Icons/Inspector/audio_listener.png", false, Texture::CompressionMethod::None, true);
				hostedEditor = ed;
			}
			/* AUDIOLISTENER */

			/* AUDIOSOURCE */
			if (comp->getComponentType() == AudioSource::COMPONENT_TYPE)
			{
				AudioSourceEditor* ed = new AudioSourceEditor();

				component = new PropEditorHost(this, AudioSource::COMPONENT_TYPE, ed);
				component->icon = Texture::load(Helper::ExePath(), "Editor/Icons/Inspector/audio_source.png", false, Texture::CompressionMethod::None, true);
				hostedEditor = ed;
			}
			/* AUDIOSOURCE */

			/* NAVMESHAGENT */
			if (comp->getComponentType() == NavMeshAgent::COMPONENT_TYPE)
			{
				NavMeshAgentEditor* ed = new NavMeshAgentEditor();

				component = new PropEditorHost(this, NavMeshAgent::COMPONENT_TYPE, ed);
				component->icon = Texture::load(Helper::ExePath(), "Editor/Icons/Inspector/navmesh_agent.png", false, Texture::CompressionMethod::None, true);
				hostedEditor = ed;
			}
			/* NAVMESHAGENT */

			/* NAVMESHOBSTACLE */
			if (comp->getComponentType() == NavMeshObstacle::COMPONENT_TYPE)
			{
				NavMeshObstacleEditor* ed = new NavMeshObstacleEditor();

				component = new PropEditorHost(this, NavMeshObstacle::COMPONENT_TYPE, ed);
				component->icon = Texture::load(Helper::ExePath(), "Editor/Icons/Inspector/navmesh_obstacle.png", false, Texture::CompressionMethod::None, true);
				hostedEditor = ed;
			}
			/* NAVMESHOBSTACLE */

			/* CANVAS */
			if (comp->getComponentType() == Canvas::COMPONENT_TYPE)
			{
				CanvasEditor* ed = new CanvasEditor();

				component = new PropEditorHost(this, Canvas::COMPONENT_TYPE, ed);
				component->icon = Texture::load(Helper::ExePath(), "Editor/Icons/Hierarchy/canvas.png", false, Texture::CompressionMethod::None, true);
				hostedEditor = ed;
			}
			/* CANVAS */

			/* IMAGE */
			if (comp->getComponentType() == Image::COMPONENT_TYPE)
			{
				ImageEditor* ed = new ImageEditor();

				uiElementEditor = ed;

				component = new PropEditorHost(this, Image::COMPONENT_TYPE, ed);
				component->icon = Texture::load(Helper::ExePath(), "Editor/Icons/Hierarchy/image.png", false, Texture::CompressionMethod::None, true);
				hostedEditor = ed;
			}
			/* IMAGE */

			/* TEXT */
			if (comp->getComponentType() == Text::COMPONENT_TYPE)
			{
				TextEditor* ed = new TextEditor();

				uiElementEditor = ed;

				component = new PropEditorHost(this, Text::COMPONENT_TYPE, ed);
				component->icon = Texture::load(Helper::ExePath(), "Editor/Icons/Hierarchy/text.png", false, Texture::CompressionMethod::None, true);
				hostedEditor = ed;
			}
			/* TEXT */

			/* BUTTON */
			if (comp->getComponentType() == Button::COMPONENT_TYPE)
			{
				ButtonEditor* ed = new ButtonEditor();

				uiElementEditor = ed;

				component = new PropEditorHost(this, Button::COMPONENT_TYPE, ed);
				component->icon = Texture::load(Helper::ExePath(), "Editor/Icons/Hierarchy/button.png", false, Texture::CompressionMethod::None, true);
				hostedEditor = ed;
			}
			/* BUTTON */

			/* TEXT INPUT */
			if (comp->getComponentType() == TextInput::COMPONENT_TYPE)
			{
				TextInputEditor* ed = new TextInputEditor();

				uiElementEditor = ed;

				component = new PropEditorHost(this, TextInput::COMPONENT_TYPE, ed);
				component->icon = Texture::load(Helper::ExePath(), "Editor/Icons/Hierarchy/text_input.png", false, Texture::CompressionMethod::None, true);
				hostedEditor = ed;
			}
			/* TEXT INPUT */

			/* MASK */
			if (comp->getComponentType() == Mask::COMPONENT_TYPE)
			{
				MaskEditor* ed = new MaskEditor();

				uiElementEditor = ed;

				component = new PropEditorHost(this, Mask::COMPONENT_TYPE, ed);
				component->icon = Texture::load(Helper::ExePath(), "Editor/Icons/Hierarchy/mask.png", false, Texture::CompressionMethod::None, true);
				hostedEditor = ed;
			}
			/* MASK */

			/* VIDEO PLAYER */
			if (comp->getComponentType() == VideoPlayer::COMPONENT_TYPE)
			{
				VideoPlayerEditor* ed = new VideoPlayerEditor();

				component = new PropEditorHost(this, VideoPlayer::COMPONENT_TYPE, ed);
				component->icon = Texture::load(Helper::ExePath(), "Editor/Icons/Inspector/video_player.png", false, Texture::CompressionMethod::None, true);
				hostedEditor = ed;
			}
			/* VIDEO PLAYER */

			/* MONOSCRIPT */
			if (comp->getComponentType() == MonoScript::COMPONENT_TYPE)
			{
				MonoScriptEditor* ed = new MonoScriptEditor();

				std::string className = ((MonoScript*)comp)->getClassName();
				if (((MonoScript*)comp)->getManagedObject() == nullptr)
					className = "MissingScript";

				component = new PropEditorHost(this, className, ed);
				component->icon = Texture::load(Helper::ExePath(), "Editor/Icons/Inspector/cs.png", false, Texture::CompressionMethod::None, true);
				hostedEditor = ed;
			}
			/* MONOSCRIPT */

			/* TERRAIN */
			if (comp->getComponentType() == Terrain::COMPONENT_TYPE)
			{
				TerrainEditor* ed = new TerrainEditor();

				component = new PropEditorHost(this, Terrain::COMPONENT_TYPE, ed);
				component->icon = Texture::load(Helper::ExePath(), "Editor/Icons/Inspector/terrain.png", false, Texture::CompressionMethod::None, true);
				hostedEditor = ed;
			}
			/* TERRAIN */

			/* PARTICLE SYSTEM */
			if (comp->getComponentType() == ParticleSystem::COMPONENT_TYPE)
			{
				ParticleSystemEditor* ed = new ParticleSystemEditor();

				component = new PropEditorHost(this, ParticleSystem::COMPONENT_TYPE, ed);
				component->icon = Texture::load(Helper::ExePath(), "Editor/Icons/Inspector/particle_system.png", false, Texture::CompressionMethod::None, true);
				hostedEditor = ed;
			}
			/* PARTICLE SYSTEM */

			/* WATER */
			if (comp->getComponentType() == Water::COMPONENT_TYPE)
			{
				WaterEditor* ed = new WaterEditor();

				component = new PropEditorHost(this, Water::COMPONENT_TYPE, ed);
				component->icon = Texture::load(Helper::ExePath(), "Editor/Icons/Inspector/water.png", false, Texture::CompressionMethod::None, true);
				hostedEditor = ed;
			}
			/* WATER */

			/* SPLINE */
			if (comp->getComponentType() == Spline::COMPONENT_TYPE)
			{
				SplineEditor* ed = new SplineEditor();

				component = new PropEditorHost(this, Spline::COMPONENT_TYPE, ed);
				component->icon = Texture::load(Helper::ExePath(), "Editor/Icons/Inspector/spline.png", false, Texture::CompressionMethod::None, true);
				hostedEditor = ed;
			}
			/* SPLINE */

			/* CSG MODEL */
			if (comp->getComponentType() == CSGModel::COMPONENT_TYPE)
			{
				CSGModelEditor* ed = new CSGModelEditor();

				component = new PropEditorHost(this, CSGModel::COMPONENT_TYPE, ed);
				component->icon = Texture::load(Helper::ExePath(), "Editor/Icons/CSG/csg_model.png", false, Texture::CompressionMethod::None, true);
				hostedEditor = ed;
			}
			/* CSG BRUSH */

			/* CSG BRUSH */
			if (comp->getComponentType() == CSGBrush::COMPONENT_TYPE)
			{
				CSGBrushEditor* ed = new CSGBrushEditor();

				component = new PropEditorHost(this, CSGBrush::COMPONENT_TYPE, ed);
				component->icon = Texture::load(Helper::ExePath(), "Editor/Icons/CSG/brush_cube.png", false, Texture::CompressionMethod::None, true);
				hostedEditor = ed;
			}
			/* CSG BRUSH */

			if (component != nullptr)
			{
				if (hostedEditor != nullptr)
				{
					((ComponentEditor*)hostedEditor)->init(comps);

					hostedEditor->getTreeView()->getRootNode()->setUserData(static_cast<void*>(new int(i)));
				}

				component->enableDrag = true;
				component->setUserData(static_cast<void*>(new int(i)));
				if (comps[0]->getComponentType() == Transform::COMPONENT_TYPE)
					component->setPopupMenu({ "Copy transform", "-",
						"Paste world transform", "Paste local transform", "-",
						"Paste world position", "Paste world rotation", "Paste world scale", "-",
						"Paste local position", "Paste local rotation", "Paste local scale", "-",
						"Paste component" },
						[=](TreeNode* node, int val) { onComponentPopup(node, val); });
				else
					component->setPopupMenu({ "Copy component", "Paste component", "-", "Remove component"}, [=](TreeNode* node, int val) { onComponentPopup(node, val); });

				if (comp->getComponentType() != Transform::COMPONENT_TYPE)
				{
					component->setCheckboxEnabled(true);
					component->setChecked(comp->getEnabled());
					component->setOnCheckedCallback([=](TreeNode* node, bool val) { onChangeComponentEnabled(comps, val); });
				}

				addProperty(component);
			}
		}
	}

	void ObjectEditor::onChangeComponentEnabled(const std::vector<Component*>& comps, bool val)
	{
		std::string txt = "Enable component";
		if (!val)
			txt = "Disable component";

		//Undo
		UndoData* undoData = Undo::addUndo(txt);
		undoData->boolData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			bool updBatches = false;
			bool updCsg = false;

			std::vector<Component*> comps;

			for (auto& d : data->boolData[0])
			{
				Component* comp = (Component*)d.first;
				comp->setEnabled(d.second);

				if (comp->getComponentType() == MeshRenderer::COMPONENT_TYPE)
				{
					if (comp->getGameObject()->getBatchingStatic())
						updBatches = true;
				}

				if (comp->getComponentType() == CSGBrush::COMPONENT_TYPE)
				{
					comps.push_back(comp);
					updCsg = true;
				}
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();

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

			for (auto& d : data->boolData[1])
			{
				Component* comp = (Component*)d.first;
				comp->setEnabled(d.second);

				if (comp->getComponentType() == MeshRenderer::COMPONENT_TYPE)
				{
					if (comp->getGameObject()->getBatchingStatic())
						updBatches = true;
				}

				if (comp->getComponentType() == CSGBrush::COMPONENT_TYPE)
				{
					comps.push_back(comp);
					updCsg = true;
				}
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();

			if (updBatches)
				MainWindow::addOnEndUpdateCallback([=]() { BatchedGeometry::getSingleton()->rebuild(); });

			if (updCsg)
				MainWindow::addOnEndUpdateCallback([=]() { CSGGeometry::getSingleton()->rebuild(comps); });
		};
		//Undo

		bool updBatches = false;
		bool updCsg = false;

		std::vector<Component*> components;

		for (auto& ob : comps)
		{
			undoData->boolData[0][ob] = ob->getEnabled();
			undoData->boolData[1][ob] = val;

			ob->setEnabled(val);

			if (ob->getComponentType() == MeshRenderer::COMPONENT_TYPE)
			{
				if (ob->getGameObject()->getBatchingStatic())
					updBatches = true;
			}

			if (ob->getComponentType() == CSGBrush::COMPONENT_TYPE)
			{
				components.push_back(ob);
				updCsg = true;
			}
		}

		if (updBatches)
			MainWindow::addOnEndUpdateCallback([=]() { BatchedGeometry::getSingleton()->rebuild(); });

		if (updCsg)
			MainWindow::addOnEndUpdateCallback([=]() { CSGGeometry::getSingleton()->rebuild(components); });
	}

	void changeNodeColor(TreeNode* root, float alpha)
	{
		if (alpha == 1.0f)
		{
			GameObject* n = Engine::getSingleton()->getGameObject(root->name);
			if (n != nullptr)
			{
				if (n->getActive()) // Visible
				{
					root->setColor(alpha, 3);

					for (auto it = root->children.begin(); it != root->children.end(); ++it)
						changeNodeColor(*it, alpha);
				}
			}
		}
		else
		{
			root->setColor(alpha, 3);

			for (auto it = root->children.begin(); it != root->children.end(); ++it)
				changeNodeColor(*it, alpha);
		}
	}

	void ObjectEditor::updateHierarchyNodeColor(GameObject* obj, bool enabled)
	{
		TreeView* tv = MainWindow::getHierarchyWindow()->getTreeView();
		TreeNode* n = tv->getNodeByName(obj->getGuid(), tv->getRootNode());
		if (n != nullptr)
		{
			if (enabled)
			{
				if (obj->getTransform()->getParent() != nullptr)
				{
					if (obj->getTransform()->getParent()->getGameObject()->getActive())
						changeNodeColor(n, 1.0f);
				}
				else
				{
					changeNodeColor(n, 1.0f);
				}

			}
			else
				changeNodeColor(n, 0.5f);
		}
	}

	void ObjectEditor::onChangeEnabled(Property* prop, bool val)
	{
		//Undo
		std::string actName = "Disable object";
		if (val == true)
			actName = "Enable object";

		UndoData* undoData = Undo::addUndo(actName);
		undoData->boolData.resize(2);

		for (auto& it : objects)
		{
			undoData->boolData[0][it] = it->getEnabled();
			undoData->boolData[1][it] = val;
		}

		undoData->undoAction = [=](UndoData* data)
		{
			bool updBatches = false;
			bool updCsg = false;

			std::vector<Component*> comps;

			for (auto& d : data->boolData[0])
			{
				GameObject* obj = (GameObject*)d.first;
				obj->setEnabled(d.second);
				updateHierarchyNodeColor((GameObject*)d.first, d.second);

				if (!updBatches)
				{
					Transform* t = obj->getTransform();
					t->iterateChildren([&updBatches, &updCsg, &comps](Transform* ct) -> bool
						{
							if (ct->getGameObject()->getBatchingStatic())
								updBatches = true;

							CSGBrush* brush = (CSGBrush*)ct->getGameObject()->getComponent(CSGBrush::COMPONENT_TYPE);
							if (brush != nullptr)
							{
								if (brush->getEnabled())
								{
									comps.push_back(brush);
									updCsg = true;
								}
							}

							return true;
						}
					);
				}
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();

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

			for (auto& d : data->boolData[1])
			{
				GameObject* obj = (GameObject*)d.first;
				obj->setEnabled(d.second);
				updateHierarchyNodeColor((GameObject*)d.first, d.second);

				if (!updBatches)
				{
					Transform* t = obj->getTransform();
					t->iterateChildren([&updBatches, &updCsg, &comps](Transform* ct) -> bool
						{
							if (ct->getGameObject()->getBatchingStatic())
								updBatches = true;

							CSGBrush* brush = (CSGBrush*)ct->getGameObject()->getComponent(CSGBrush::COMPONENT_TYPE);
							if (brush != nullptr)
							{
								if (brush->getEnabled())
								{
									comps.push_back(brush);
									updCsg = true;
								}
							}

							return true;
						}
					);
				}
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();

			if (updBatches)
				MainWindow::addOnEndUpdateCallback([=]() { BatchedGeometry::getSingleton()->rebuild(); });

			if (updCsg)
				MainWindow::addOnEndUpdateCallback([=]() { CSGGeometry::getSingleton()->rebuild(comps); });
		};
		//

		bool updBatches = false;
		bool updCsg = false;

		std::vector<Component*> comps;

		for (auto it = objects.begin(); it != objects.end(); ++it)
		{
			GameObject* node = *it;
			node->setEnabled(val);

			updateHierarchyNodeColor(node, val);

			if (!updBatches)
			{
				Transform* t = node->getTransform();
				t->iterateChildren([&updBatches, &updCsg, &comps](Transform* ct) -> bool
					{
						if (ct->getGameObject()->getBatchingStatic())
							updBatches = true;

						CSGBrush* brush = (CSGBrush*)ct->getGameObject()->getComponent(CSGBrush::COMPONENT_TYPE);
						if (brush != nullptr)
						{
							if (brush->getEnabled())
							{
								comps.push_back(brush);
								updCsg = true;
							}
						}

						return true;
					}
				);
			}
		}

		NavigationManager::getSingleton()->setNavMeshIsDirty();

		if (updBatches)
			MainWindow::addOnEndUpdateCallback([=]() { BatchedGeometry::getSingleton()->rebuild(); });

		if (updCsg)
			MainWindow::addOnEndUpdateCallback([=]() { CSGGeometry::getSingleton()->rebuild(comps); });
	}

	void ObjectEditor::onChangeName(Property* prop, std::string val)
	{
		if (val.empty())
			return;

		//Undo
		UndoData* undoData = Undo::addUndo("Rename object");
		undoData->stringData.resize(2);

		for (auto& it : objects)
		{
			undoData->stringData[0][it] = it->getName();
			undoData->stringData[1][it] = val;
		}

		undoData->undoAction = [=](UndoData* data)
		{
			TreeView* tree = MainWindow::getHierarchyWindow()->getTreeView();
			
			for (auto& d : data->stringData[0])
			{
				GameObject* obj = ((GameObject*)d.first);

				TreeNode* treeNode = tree->getNodeByName(obj->getGuid(), tree->getRootNode());
				if (treeNode != nullptr)
					treeNode->alias = CP_UNI(d.second);

				obj->setName(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			TreeView* tree = MainWindow::getHierarchyWindow()->getTreeView();

			for (auto& d : data->stringData[1])
			{
				GameObject* obj = ((GameObject*)d.first);

				TreeNode* treeNode = tree->getNodeByName(obj->getGuid(), tree->getRootNode());
				if (treeNode != nullptr)
					treeNode->alias = CP_UNI(d.second);

				obj->setName(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = objects.begin(); it != objects.end(); ++it)
		{
			GameObject* node = *it;
			node->setName(CP_SYS(val));

			TreeView* tree = MainWindow::getHierarchyWindow()->getTreeView();
			TreeNode* treeNode = tree->getNodeByName(node->getGuid(), tree->getRootNode());
			if (treeNode != nullptr)
				treeNode->alias = val;
		}
	}

	void ObjectEditor::addTransformUndo(int index, bool setPosition, bool setRotation, bool setScale)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Paste transform");
		undoData->matrixData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			bool updBatches = false;

			for (auto& d : data->matrixData[0])
			{
				Transform* comp = (Transform*)d.first;
				comp->setTransformMatrix(d.second);

				if (!updBatches)
				{
					comp->iterateChildren([&updBatches](Transform* child) -> bool
						{
							if (child->getGameObject() != nullptr)
							{
								if (child->getGameObject()->getBatchingStatic())
								{
									updBatches = true;
									return false;
								}
							}

							return true;
						}
					);
				}
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();

			if (updBatches)
				MainWindow::addOnEndUpdateCallback([=]() { BatchedGeometry::getSingleton()->rebuild(); });
		};

		undoData->redoAction = [=](UndoData* data)
		{
			bool updBatches = false;

			for (auto& d : data->matrixData[1])
			{
				Transform* comp = (Transform*)d.first;
				comp->setTransformMatrix(d.second);

				if (!updBatches)
				{
					comp->iterateChildren([&updBatches](Transform* child) -> bool
						{
							if (child->getGameObject() != nullptr)
							{
								if (child->getGameObject()->getBatchingStatic())
								{
									updBatches = true;
									return false;
								}
							}

							return true;
						}
					);
				}
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();

			if (updBatches)
				MainWindow::addOnEndUpdateCallback([=]() { BatchedGeometry::getSingleton()->rebuild(); });
		};
		//

		bool updBatches = false;

		for (auto& it : objects)
		{
			Transform* t = nullptr;
			t = (Transform*)it->getComponent(index);

			glm::vec3 pos = t->getPosition();
			glm::quat rot = t->getRotation();
			glm::vec3 scl = t->getScale();

			if (setPosition) pos = bufferPosition;
			if (setRotation) rot = bufferRotation;
			if (setScale) scl = bufferScale;

			undoData->matrixData[0][t] = t->getTransformMatrix();
			undoData->matrixData[1][t] = Transform::makeTransformMatrix(pos, rot, scl);

			if (!updBatches)
			{
				t->iterateChildren([&updBatches](Transform* child) -> bool
					{
						if (child->getGameObject() != nullptr)
						{
							if (child->getGameObject()->getBatchingStatic())
							{
								updBatches = true;
								return false;
							}
						}

						return true;
					}
				);
			}
		}

		if (updBatches)
			MainWindow::addOnEndUpdateCallback([=]() { BatchedGeometry::getSingleton()->rebuild(); });
	}

	void ObjectEditor::removeComponent(int index)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Remove component");
		undoData->objectData.resize(1);
		undoData->intData.resize(1);

		undoData->undoAction = [=](UndoData* data)
		{
			bool updBatches = false;
			bool updCsg = false;

			std::vector<Component*> comps;

			for (auto& d : data->objectData[0])
			{
				Component* comp = (Component*)d;
				GameObject* obj = comp->getGameObject();
				auto& comps = obj->getComponents();
				int idx = data->intData[0][comp];
				comps.insert(comps.begin() + idx, comp);
				comp->onAttach();

				if (comp->getComponentType() == MeshRenderer::COMPONENT_TYPE)
				{
					if (obj->getBatchingStatic())
						updBatches = true;
				}

				if (comp->getComponentType() == CSGBrush::COMPONENT_TYPE)
				{
					if (obj->getActive() && comp->getEnabled())
					{
						comps.push_back(comp);
						updCsg = true;
					}
				}

				MainWindow::getSingleton()->getHierarchyWindow()->updateNodeIcon(obj->getTransform());
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
			NavigationManager::getSingleton()->setNavMeshIsDirty();

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
				Component* comp = (Component*)d;

				if (bufferComponent == comp)
					bufferComponent = nullptr;

				if (comp->isUiComponent())
				{
					UIEditorWindow* uiEditor = MainWindow::getUIEditorWindow();
					auto& sel = uiEditor->getSelectedObjects();
					auto ci = std::find(sel.begin(), sel.end(), comp);
					if (ci != sel.end())
						sel.erase(ci);
				}

				GameObject* obj = comp->getGameObject();
				auto& comps = obj->getComponents();
				auto it = std::find(comps.begin(), comps.end(), comp);
				comps.erase(it);
				comp->onDetach();

				if (comp->getComponentType() == MeshRenderer::COMPONENT_TYPE)
				{
					if (obj->getBatchingStatic())
						updBatches = true;
				}

				if (comp->getComponentType() == CSGBrush::COMPONENT_TYPE)
				{
					if (obj->getActive() && comp->getEnabled())
					{
						comps.push_back(comp);
						updCsg = true;
					}
				}

				MainWindow::getSingleton()->getHierarchyWindow()->updateNodeIcon(obj->getTransform());
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
			NavigationManager::getSingleton()->setNavMeshIsDirty();

			if (updBatches)
				MainWindow::addOnEndUpdateCallback([=]() { BatchedGeometry::getSingleton()->rebuild(); });

			if (updCsg)
				MainWindow::addOnEndUpdateCallback([=]() { CSGGeometry::getSingleton()->rebuild(comps); });
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
				}
			}
		};
		//

		bool updBatches = false;
		bool updCsg = false;

		std::vector<Component*> comps;

		for (auto it = objects.begin(); it != objects.end(); ++it)
		{
			GameObject* node = *it;
			Component* comp = node->getComponents().at(index);

			if (bufferComponent == comp)
				bufferComponent = nullptr;

			if (comp->isUiComponent())
			{
				UIEditorWindow* uiEditor = MainWindow::getUIEditorWindow();
				auto& sel = uiEditor->getSelectedObjects();
				auto ci = std::find(sel.begin(), sel.end(), comp);
				if (ci != sel.end())
					sel.erase(ci);
			}

			if (comp->getComponentType() == MeshRenderer::COMPONENT_TYPE)
			{
				if (node->getBatchingStatic())
					updBatches = true;
			}

			if (comp->getComponentType() == CSGBrush::COMPONENT_TYPE)
			{
				if (node->getActive() && comp->getEnabled())
				{
					comps.push_back(comp);
					updCsg = true;
				}
			}

			//node->removeComponent(index);
			auto& comps = node->getComponents();
			auto _it = std::find(comps.begin(), comps.end(), comp);
			comps.erase(_it);
			comp->onDetach();

			undoData->objectData[0].push_back(comp);
			undoData->intData[0][comp] = index;

			MainWindow::getSingleton()->getHierarchyWindow()->updateNodeIcon(node->getTransform());
		}

		updateEditor();
		NavigationManager::getSingleton()->setNavMeshIsDirty();

		if (updBatches)
			MainWindow::addOnEndUpdateCallback([=]() { BatchedGeometry::getSingleton()->rebuild(); });

		if (updCsg)
			MainWindow::addOnEndUpdateCallback([=]() { CSGGeometry::getSingleton()->rebuild(comps); });
	}

	void ObjectEditor::onComponentPopup(TreeNode* node, int val)
	{
		int index = *static_cast<int*>(node->getUserData());
		GameObject* obj = objects[0];
		Component* cmp = nullptr;
		cmp = obj->getComponent(index);

		if (val == 0)
		{
			if (cmp->getComponentType() != Transform::COMPONENT_TYPE)
			{
				bufferComponent = cmp;
				bufferTransform = false;
			}
			else
			{
				bufferComponent = nullptr;
				bufferTransform = true;

				bufferPosition = ((Transform*)cmp)->getPosition();
				bufferRotation = ((Transform*)cmp)->getRotation();
				bufferScale = ((Transform*)cmp)->getScale();

				bufferLocalPosition = ((Transform*)cmp)->getLocalPosition();
				bufferLocalRotation = ((Transform*)cmp)->getLocalRotation();
				bufferLocalScale = ((Transform*)cmp)->getLocalScale();
			}
		}

		if (val == 1 || val == 2 || val == 9)
		{
			if (cmp->getComponentType() == Transform::COMPONENT_TYPE)
			{
				if (val == 1)
				{
					if (bufferTransform)
					{
						if (bufferPosition != glm::vec3(FLT_MAX) &&
							bufferRotation != glm::quat(FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX) &&
							bufferScale != glm::vec3(FLT_MAX))
						{
							addTransformUndo(index, true, true, true);

							for (auto& it : objects)
							{
								Transform* t = nullptr;
								t = (Transform*)it->getComponent(index);

								t->setPosition(bufferPosition);
								t->setRotation(bufferRotation);
								t->setScale(bufferScale);
							}

							updateEditor();
						}
					}
				}

				if (val == 2)
				{
					if (bufferTransform)
					{
						if (bufferLocalPosition != glm::vec3(FLT_MAX) &&
							bufferLocalRotation != glm::quat(FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX) &&
							bufferLocalScale != glm::vec3(FLT_MAX))
						{
							addTransformUndo(index, true, true, true);

							for (auto& it : objects)
							{
								Transform* t = nullptr;
								t = (Transform*)it->getComponent(index);

								t->setLocalPosition(bufferLocalPosition);
								t->setLocalRotation(bufferLocalRotation);
								t->setLocalScale(bufferLocalScale);
							}

							updateEditor();
						}
					}
				}
			}

			if ((val == 1 && cmp->getComponentType() != Transform::COMPONENT_TYPE)
				|| (val == 9 && cmp->getComponentType() == Transform::COMPONENT_TYPE))
			{
				if (bufferComponent != nullptr)
				{
					//Undo
					UndoData* undoData = Undo::addUndo("Paste component");
					undoData->objectData.resize(1);

					undoData->undoAction = [=](UndoData* data)
					{
						for (auto& d : data->objectData[0])
						{
							Component* comp = (Component*)d;
							GameObject* obj = comp->getGameObject();
							auto& comps = obj->getComponents();
							auto it = std::find(comps.begin(), comps.end(), comp);
							comps.erase(it);
							comp->onDetach();

							MainWindow::getSingleton()->getHierarchyWindow()->updateNodeIcon(obj->getTransform());
						}

						MainWindow::getInspectorWindow()->updateCurrentEditor();
						NavigationManager::getSingleton()->setNavMeshIsDirty();
					};

					undoData->redoAction = [=](UndoData* data)
					{
						for (auto& d : data->objectData[0])
						{
							Component* comp = (Component*)d;
							GameObject* obj = comp->getGameObject();
							auto& comps = obj->getComponents();
							comps.push_back(comp);
							comp->onAttach();

							MainWindow::getSingleton()->getHierarchyWindow()->updateNodeIcon(obj->getTransform());
						}

						MainWindow::getInspectorWindow()->updateCurrentEditor();
						NavigationManager::getSingleton()->setNavMeshIsDirty();
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
							}
						}
					};
					//

					for (auto& it : objects)
					{
						if (std::find(multipleComponentsAllowed.begin(),
							multipleComponentsAllowed.end(),
							bufferComponent->getComponentType()) == multipleComponentsAllowed.end())
						{
							if (it->getComponent(bufferComponent->getComponentType()) != nullptr)
								continue;
						}

						Component* newComponent = bufferComponent->onClone();

						undoData->objectData[0].push_back(newComponent);

						it->addComponent(newComponent);
						MainWindow::getSingleton()->getHierarchyWindow()->updateNodeIcon(it->getTransform());
					}

					if (undoData->objectData[0].size() == 0)
						Undo::removeUndo(undoData);

					updateEditor();
				}
			}
		}

		if (val == 2)
		{
			if (cmp->getComponentType() != Transform::COMPONENT_TYPE)
			{
				removeComponent(index);
			}
		}

		//World transform
		if (val == 3)
		{
			if (cmp->getComponentType() == Transform::COMPONENT_TYPE)
			{
				if (bufferPosition != glm::vec3(FLT_MAX))
				{
					addTransformUndo(index, true, false, false);

					for (auto& it : objects)
					{
						Transform* t = nullptr;
						t = (Transform*)it->getComponent(index);

						t->setPosition(bufferPosition);
					}

					updateEditor();
				}
			}
		}

		if (val == 4)
		{
			if (cmp->getComponentType() == Transform::COMPONENT_TYPE)
			{
				if (bufferRotation != glm::quat(FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX))
				{
					addTransformUndo(index, false, true, false);

					for (auto& it : objects)
					{
						Transform* t = nullptr;
						t = (Transform*)it->getComponent(index);

						t->setRotation(bufferRotation);
					}

					updateEditor();
				}
			}
		}

		if (val == 5)
		{
			if (cmp->getComponentType() == Transform::COMPONENT_TYPE)
			{
				if (bufferScale != glm::vec3(FLT_MAX))
				{
					addTransformUndo(index, false, false, true);

					for (auto& it : objects)
					{
						Transform* t = nullptr;
						t = (Transform*)it->getComponent(index);

						t->setScale(bufferScale);
					}

					updateEditor();
				}
			}
		}

		//Local transform
		if (val == 6)
		{
			if (cmp->getComponentType() == Transform::COMPONENT_TYPE)
			{
				if (bufferLocalPosition != glm::vec3(FLT_MAX))
				{
					addTransformUndo(index, true, false, false);

					for (auto& it : objects)
					{
						Transform* t = nullptr;
						t = (Transform*)it->getComponent(index);

						t->setLocalPosition(bufferLocalPosition);
					}

					updateEditor();
				}
			}
		}

		if (val == 7)
		{
			if (cmp->getComponentType() == Transform::COMPONENT_TYPE)
			{
				if (bufferLocalRotation != glm::quat(FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX))
				{
					addTransformUndo(index, false, true, false);

					for (auto& it : objects)
					{
						Transform* t = nullptr;
						t = (Transform*)it->getComponent(index);

						t->setLocalRotation(bufferRotation);
					}

					updateEditor();
				}
			}
		}

		if (val == 8)
		{
			if (cmp->getComponentType() == Transform::COMPONENT_TYPE)
			{
				if (bufferLocalScale != glm::vec3(FLT_MAX))
				{
					addTransformUndo(index, false, false, true);

					for (auto& it : objects)
					{
						Transform* t = nullptr;
						t = (Transform*)it->getComponent(index);

						t->setLocalScale(bufferScale);
					}

					updateEditor();
				}
			}
		}
	}

	void ObjectEditor::onChangeTag(Property* node, int val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change object tag");
		undoData->intData.resize(2);

		for (auto& it : objects)
		{
			undoData->intData[0][it] = it->getTag();
			undoData->intData[1][it] = val;
		}

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->intData[0])
				((GameObject*)d.first)->setTag(d.second);

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->intData[1])
				((GameObject*)d.first)->setTag(d.second);

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = objects.begin(); it != objects.end(); ++it)
		{
			GameObject* node = *it;
			node->setTag(val);
		}
	}

	void ObjectEditor::onChangeLayer(Property* node, int val)
	{
		bool hasChild = false;
		for (auto& obj : objects)
		{
			if (obj->getTransform()->getChildren().size() > 0)
			{
				hasChild = true;
				break;
			}
		}

		int result = 7; //IDNO;
		if (hasChild)
		{
	#ifdef _WIN32
			result = MessageBoxA(0, "Do you want to change children?", "Change layer", MB_YESNOCANCEL | MB_ICONQUESTION);
	#else
			char filename[1024];
			FILE *f = popen("zenity --info --text 'Do you want to change children?' --title 'Change layer' --no-wrap \
							--ok-label Yes \
							--extra-button No \
							--extra-button Cancel", "r");
			fgets(filename, 1024, f);
			int r = pclose(f);

			if (std::string(filename).rfind("No", 0) != std::string::npos)
				result = 7;
			else if (std::string(filename).rfind("Cancel") != std::string::npos)
				result = 2;
			else if (r != 0)
				result = 2;
			else
				result = 6;
	#endif
		}
		
		if (result == 6) //IDYES
		{
			//Undo
			UndoData* undoData = Undo::addUndo("Change object layer");
			undoData->intData.resize(2);

			undoData->undoAction = [=](UndoData* data)
			{
				bool updBatches = false;

				for (auto& d : data->intData[0])
				{
					GameObject* obj = (GameObject*)d.first;
					obj->setLayer(d.second);

					if (obj->getBatchingStatic())
						updBatches = true;
				}

				MainWindow::getInspectorWindow()->updateCurrentEditor();

				if (updBatches)
					MainWindow::addOnEndUpdateCallback([=]() { BatchedGeometry::getSingleton()->rebuild(); });
			};

			undoData->redoAction = [=](UndoData* data)
			{
				bool updBatches = false;

				for (auto& d : data->intData[1])
				{
					GameObject* obj = (GameObject*)d.first;
					obj->setLayer(d.second);

					if (obj->getBatchingStatic())
						updBatches = true;
				}

				MainWindow::getInspectorWindow()->updateCurrentEditor();

				if (updBatches)
					MainWindow::addOnEndUpdateCallback([=]() { BatchedGeometry::getSingleton()->rebuild(); });
			};
			//

			bool updBatches = false;

			std::vector<Transform*> nstack;
			for (auto it = objects.begin(); it != objects.end(); ++it)
				nstack.push_back((*it)->getTransform());

			while (nstack.size() > 0)
			{
				Transform* child = *nstack.begin();
				nstack.erase(nstack.begin());

				//Update static
				GameObject* obj = child->getGameObject();
				undoData->intData[0][obj] = obj->getLayer();
				undoData->intData[1][obj] = val;
				obj->setLayer(val);
				//

				if (obj->getBatchingStatic())
					updBatches = true;

				int j = 0;
				for (auto it = child->getChildren().begin(); it != child->getChildren().end(); ++it, ++j)
				{
					Transform* ch = *it;
					nstack.insert(nstack.begin() + j, ch);
				}
			}

			if (updBatches)
				MainWindow::addOnEndUpdateCallback([=]() { BatchedGeometry::getSingleton()->rebuild(); });
		}

		if (result == 7) //IDNO
		{
			//Undo
			UndoData* undoData = Undo::addUndo("Change object layer");
			undoData->intData.resize(2);

			for (auto& obj : objects)
			{
				undoData->intData[0][obj] = obj->getLayer();
				undoData->intData[1][obj] = val;
			}

			undoData->undoAction = [=](UndoData* data)
			{
				bool updBatches = false;

				for (auto& d : data->intData[0])
				{
					GameObject* obj = (GameObject*)d.first;
					obj->setLayer(d.second);

					if (obj->getBatchingStatic())
						updBatches = true;
				}

				MainWindow::getInspectorWindow()->updateCurrentEditor();

				if (updBatches)
					MainWindow::addOnEndUpdateCallback([=]() { BatchedGeometry::getSingleton()->rebuild(); });
			};

			undoData->redoAction = [=](UndoData* data)
			{
				bool updBatches = false;

				for (auto& d : data->intData[1])
				{
					GameObject* obj = (GameObject*)d.first;
					obj->setLayer(d.second);

					if (obj->getBatchingStatic())
						updBatches = true;
				}

				MainWindow::getInspectorWindow()->updateCurrentEditor();

				if (updBatches)
					MainWindow::addOnEndUpdateCallback([=]() { BatchedGeometry::getSingleton()->rebuild(); });
			};
			//

			bool updBatches = false;

			for (auto& obj : objects)
			{
				obj->setLayer(val);

				if (obj->getBatchingStatic())
					updBatches = true;
			}

			if (updBatches)
				MainWindow::addOnEndUpdateCallback([=]() { BatchedGeometry::getSingleton()->rebuild(); });
		}

		if (result != 2) //IDCANCEL
			((PropComboBox*)node)->setCurrentItem(val);
		else
			updateEditor();
	}

	void ObjectEditor::onChangeStatic(Property* node, int val)
	{
		int result = 7; //IDNO

		bool hasChild = false;
		for (auto& obj : objects)
		{
			if (obj->getTransform()->getChildren().size() > 0)
			{
				hasChild = true;
				break;
			}
		}

		if (hasChild)
		{
	#ifdef _WIN32
			result = MessageBoxA(0, "Do you want to change children?", "Change static", MB_YESNOCANCEL | MB_ICONQUESTION);
	#else
			char filename[1024];
			FILE *f = popen("zenity --info --text 'Do you want to change children?' --title 'Change static' --no-wrap \
							--ok-label Yes \
							--extra-button No \
							--extra-button Cancel", "r");
			fgets(filename, 1024, f);
			int r = pclose(f);

			if (std::string(filename).rfind("No", 0) != std::string::npos)
				result = 7;
			else if (std::string(filename).rfind("Cancel") != std::string::npos)
				result = 2;
			else if (r != 0)
				result = 2;
			else
				result = 6;
	#endif
		}
		
		GameObject* currentNode = objects[0];
		
		bool _val = true;
		if (currentNode->getLightingStatic() || currentNode->getNavigationStatic() || currentNode->getBatchingStatic() || currentNode->getOcclusionStatic())
			_val = false;

		bool _lighting = currentNode->getLightingStatic();
		bool _navigation = currentNode->getNavigationStatic();
		bool _batching = currentNode->getBatchingStatic();
		bool _occlusion = currentNode->getOcclusionStatic();

		if (result == 6) //IDYES
		{
			//Undo
			UndoData* undoData = Undo::addUndo("Change object static");
			undoData->intData.resize(8);

			undoData->undoAction = [=](UndoData* data)
			{
				for (auto& d : data->intData[0])
					((GameObject*)d.first)->setLightingStatic(d.second);

				for (auto& d : data->intData[1])
					((GameObject*)d.first)->setNavigationStatic(d.second);

				for (auto& d : data->intData[2])
					((GameObject*)d.first)->setBatchingStatic(d.second);

				for (auto& d : data->intData[3])
					((GameObject*)d.first)->setOcclusionStatic(d.second);
				
				MainWindow::getInspectorWindow()->updateCurrentEditor();
				MainWindow::addOnEndUpdateCallback([=]() { BatchedGeometry::getSingleton()->rebuild(); });
			};

			undoData->redoAction = [=](UndoData* data)
			{
				for (auto& d : data->intData[4])
					((GameObject*)d.first)->setLightingStatic(d.second);

				for (auto& d : data->intData[5])
					((GameObject*)d.first)->setNavigationStatic(d.second);

				for (auto& d : data->intData[6])
					((GameObject*)d.first)->setBatchingStatic(d.second);

				for (auto& d : data->intData[7])
					((GameObject*)d.first)->setOcclusionStatic(d.second);

				MainWindow::getInspectorWindow()->updateCurrentEditor();
				MainWindow::addOnEndUpdateCallback([=]() { BatchedGeometry::getSingleton()->rebuild(); });
			};
			//

			std::vector<Transform*> nstack;
			for (auto it = objects.begin(); it != objects.end(); ++it)
				nstack.push_back((*it)->getTransform());

			while (nstack.size() > 0)
			{
				Transform* child = *nstack.begin();
				nstack.erase(nstack.begin());

				//Update static
				GameObject* obj = child->getGameObject();
				undoData->intData[0][obj] = obj->getLightingStatic();
				undoData->intData[1][obj] = obj->getNavigationStatic();
				undoData->intData[2][obj] = obj->getBatchingStatic();
				undoData->intData[3][obj] = obj->getOcclusionStatic();

				if (val == 0)
				{
					undoData->intData[4][obj] = _val;
					undoData->intData[5][obj] = _val;
					undoData->intData[6][obj] = _val;
					undoData->intData[7][obj] = _val;

					obj->setLightingStatic(_val);
					obj->setNavigationStatic(_val);
					obj->setBatchingStatic(_val);
					obj->setOcclusionStatic(_val);
				}
				if (val == 1)
				{
					undoData->intData[4][obj] = !_lighting;
					undoData->intData[5][obj] = obj->getNavigationStatic();
					undoData->intData[6][obj] = obj->getBatchingStatic();
					undoData->intData[7][obj] = obj->getOcclusionStatic();

					obj->setLightingStatic(!_lighting);
				}
				if (val == 2)
				{
					undoData->intData[4][obj] = obj->getLightingStatic();
					undoData->intData[5][obj] = !_navigation;
					undoData->intData[6][obj] = obj->getBatchingStatic();
					undoData->intData[7][obj] = obj->getOcclusionStatic();

					obj->setNavigationStatic(!_navigation);
				}
				if (val == 3)
				{
					undoData->intData[4][obj] = obj->getLightingStatic();
					undoData->intData[5][obj] = obj->getNavigationStatic();
					undoData->intData[6][obj] = !_batching;
					undoData->intData[7][obj] = obj->getOcclusionStatic();

					obj->setBatchingStatic(!_batching);
				}
				if (val == 4)
				{
					undoData->intData[4][obj] = obj->getLightingStatic();
					undoData->intData[5][obj] = obj->getNavigationStatic();
					undoData->intData[6][obj] = obj->getBatchingStatic();
					undoData->intData[7][obj] = !_occlusion;

					obj->setOcclusionStatic(!_occlusion);
				}
				//

				int j = 0;
				for (auto it = child->getChildren().begin(); it != child->getChildren().end(); ++it, ++j)
				{
					Transform* ch = *it;
					nstack.insert(nstack.begin() + j, ch);
				}
			}
		}

		if (result == 7) //IDNO
		{
			//Undo
			UndoData* undoData = Undo::addUndo("Change object static");
			undoData->intData.resize(8);

			for (auto& obj : objects)
			{
				undoData->intData[0][obj] = obj->getLightingStatic();
				undoData->intData[1][obj] = obj->getNavigationStatic();
				undoData->intData[2][obj] = obj->getBatchingStatic();
				undoData->intData[3][obj] = obj->getOcclusionStatic();

				if (val == 0)
				{
					undoData->intData[4][obj] = _val;
					undoData->intData[5][obj] = _val;
					undoData->intData[6][obj] = _val;
					undoData->intData[7][obj] = _val;
				}
				if (val == 1)
				{
					undoData->intData[4][obj] = !_lighting;
					undoData->intData[5][obj] = obj->getNavigationStatic();
					undoData->intData[6][obj] = obj->getBatchingStatic();
					undoData->intData[7][obj] = obj->getOcclusionStatic();
				}
				if (val == 2)
				{
					undoData->intData[4][obj] = obj->getLightingStatic();
					undoData->intData[5][obj] = !_navigation;
					undoData->intData[6][obj] = obj->getBatchingStatic();
					undoData->intData[7][obj] = obj->getOcclusionStatic();
				}
				if (val == 3)
				{
					undoData->intData[4][obj] = obj->getLightingStatic();
					undoData->intData[5][obj] = obj->getNavigationStatic();
					undoData->intData[6][obj] = !_batching;
					undoData->intData[7][obj] = obj->getOcclusionStatic();
				}
				if (val == 4)
				{
					undoData->intData[4][obj] = obj->getLightingStatic();
					undoData->intData[5][obj] = obj->getNavigationStatic();
					undoData->intData[6][obj] = obj->getBatchingStatic();
					undoData->intData[7][obj] = !_occlusion;
				}
			}

			undoData->undoAction = [=](UndoData* data)
			{
				for (auto& d : data->intData[0])
					((GameObject*)d.first)->setLightingStatic(d.second);

				for (auto& d : data->intData[1])
					((GameObject*)d.first)->setNavigationStatic(d.second);

				for (auto& d : data->intData[2])
					((GameObject*)d.first)->setBatchingStatic(d.second);

				for (auto& d : data->intData[3])
					((GameObject*)d.first)->setOcclusionStatic(d.second);

				MainWindow::getInspectorWindow()->updateCurrentEditor();
				MainWindow::addOnEndUpdateCallback([=]() { BatchedGeometry::getSingleton()->rebuild(); });
			};

			undoData->redoAction = [=](UndoData* data)
			{
				for (auto& d : data->intData[4])
					((GameObject*)d.first)->setLightingStatic(d.second);

				for (auto& d : data->intData[5])
					((GameObject*)d.first)->setNavigationStatic(d.second);

				for (auto& d : data->intData[6])
					((GameObject*)d.first)->setBatchingStatic(d.second);

				for (auto& d : data->intData[7])
					((GameObject*)d.first)->setOcclusionStatic(d.second);

				MainWindow::getInspectorWindow()->updateCurrentEditor();
				MainWindow::addOnEndUpdateCallback([=]() { BatchedGeometry::getSingleton()->rebuild(); });
			};
			//

			for (auto& obj : objects)
			{
				if (val == 0)
				{
					obj->setLightingStatic(_val);
					obj->setNavigationStatic(_val);
					obj->setBatchingStatic(_val);
					obj->setOcclusionStatic(_val);
				}

				if (val == 1)
					obj->setLightingStatic(!_lighting);

				if (val == 2)
					obj->setNavigationStatic(!_navigation);

				if (val == 3)
					obj->setBatchingStatic(!_batching);

				if (val == 4)
					obj->setOcclusionStatic(!_occlusion);
			}
		}

		if (result != 2) //IDCANCEL
		{
			PropComboBox* _static = (PropComboBox*)node;

			_static->setValueEnabled(
				{
					false,
					currentNode->getLightingStatic(),
					currentNode->getNavigationStatic(),
					currentNode->getBatchingStatic(),
					currentNode->getOcclusionStatic()
				}
			);

			std::string allNone = "All";
			if (currentNode->getLightingStatic() ||
				currentNode->getNavigationStatic() ||
				currentNode->getBatchingStatic() ||
				currentNode->getOcclusionStatic())
			{
				allNone = "None";
			}
			std::string labelStatic = "No";
			if (allNone == "None") labelStatic = "Yes";
			_static->setLabel(labelStatic);
			_static->setValue(0, allNone);

			if (val == 0 || val == 1 || val == 3)
				MainWindow::addOnEndUpdateCallback([=]() { BatchedGeometry::getSingleton()->rebuild(); });
		}
		else
			updateEditor();
	}
}