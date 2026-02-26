#include "HierarchyWindow.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <dear-imgui/misc/cpp/imgui_stdlib.h>

#include "MainWindow.h"
#include "SceneWindow.h"
#include "InspectorWindow.h"
#include "UIEditorWindow.h"
#include "AssetsWindow.h"

#include "../Classes/TreeView.h"
#include "../Classes/TreeNode.h"
#include "../PropertyEditors/TerrainEditor.h"
#include "../Classes/Undo.h"

#include "../Engine/Core/Engine.h"
#include "../Engine/Gizmo/Gizmo.h"
#include "../Engine/Classes/Helpers.h"
#include "../Engine/Classes/GUIDGenerator.h"
#include "../Engine/Classes/Hash.h"
#include "../Engine/Classes/StringConverter.h"
#include "../Engine/Classes/IO.h"
#include "../Engine/Classes/VectorUtils.h"
#include "../Engine/Assets/Texture.h"
#include "../Engine/Assets/Scene.h"
#include "../Engine/Assets/Prefab.h"
#include "../Engine/Assets/Model3DLoader.h"

#include "../Engine/Renderer/CSGGeometry.h"
#include "../Engine/Core/GameObject.h"
#include "../Engine/Components/Transform.h"
#include "../Engine/Components/MeshRenderer.h"
#include "../Engine/Components/DecalRenderer.h"
#include "../Engine/Components/Camera.h"
#include "../Engine/Components/Light.h"
#include "../Engine/Components/Terrain.h"
#include "../Engine/Components/ParticleSystem.h"
#include "../Engine/Components/Canvas.h"
#include "../Engine/Components/Image.h"
#include "../Engine/Components/Text.h"
#include "../Engine/Components/Button.h"
#include "../Engine/Components/TextInput.h"
#include "../Engine/Components/Mask.h"
#include "../Engine/Components/AudioSource.h"
#include "../Engine/Components/AudioListener.h"
#include "../Engine/Components/Water.h"
#include "../Engine/Components/Spline.h"
#include "../Engine/Components/VideoPlayer.h"
#include "../Engine/Components/CSGBrush.h"

namespace GX
{
	std::vector<GameObject*> HierarchyWindow::ignoreObjects;

	HierarchyWindow::HierarchyWindow()
	{
		treeView = new TreeView();
		treeView->setTag("Hierarchy");
		treeView->setAllowMoveNodes(true);
		treeView->setProcessDragDropInTarget(true);
		treeView->setAllowReorder(true);
		treeView->setSelectCallback(onNodesSelected);
		treeView->setHasTopPadding(true);
		treeView->setOnReorderCallback([=](TreeNode* node, int newIndex) { onReorder(node, newIndex); });
		treeView->setOnReorderCallback([=](std::map<TreeNode*, std::pair<TreeNode*, std::pair<int, int>>> nodes) { onReorder(nodes); });

		auto formats = Engine::getModel3dFileFormats();
		formats.push_back("*.");
		formats.push_back("::GameObject");
		formats.push_back("prefab");
		formats.push_back("terrain");

		treeView->getRootNode()->setSupportedFormats(formats);
		treeView->getRootNode()->setOnDropCallback([=](TreeNode* node, TreeNode* from) { onDropNode(node, from); });
	}

	HierarchyWindow::~HierarchyWindow()
	{
		delete treeView;
	}

	void HierarchyWindow::init()
	{
		addIcon = Texture::load(Helper::ExePath(), "Editor/Icons/Toolbar/add.png", false, Texture::CompressionMethod::None, true);
		closeIcon = Texture::load(Helper::ExePath(), "Editor/Icons/Hierarchy/close.png", false, Texture::CompressionMethod::None, true);

		exportIcon = MainWindow::loadEditorIcon("Menu/export.png");
		renameIcon = MainWindow::loadEditorIcon("Menu/rename.png");
		deleteIcon = MainWindow::loadEditorIcon("Menu/delete_object.png");
		duplicateIcon = MainWindow::loadEditorIcon("Menu/duplicate_object.png");
	}

	void HierarchyWindow::update()
	{
		bool rootOpened = true;

		if (opened)
		{
			if (ImGui::Begin("Hierarchy", &opened, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
			{
				focused = ImGui::IsWindowFocused();

				updateObjectsMenu();

				/* TREE VIEW */

				ImGuiWindow* window = GImGui->CurrentWindow;
				ImGuiID id = window->GetIDNoKeepAlive("##HierarchyVS1");
				ImGui::BeginChild(id);
				//ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(5, 0));
				treeView->update();
				//ImGui::PopStyleVar();
				ImGui::EndChild();

				if (!focused) focused = ImGui::IsItemHovered();

				/* TREEVIEW */

				if (ImGui::IsMouseDown(0) && ImGui::IsMouseDragging(0))
					isWindowDragging = true;

				//if (!isWindowDragging || ImGui::IsMouseReleased(0))
				treeView->updateDragDropTarget();

				if (ImGui::IsWindowHovered() && ImGui::IsMouseReleased(0))
				{
					if (!isWindowDragging)
					{
						if (!ImGui::IsAnyItemHovered())
						{
							treeView->selectNodes({  });
						}
					}
				}

				if (ImGui::IsMouseReleased(0))
				{
					isWindowDragging = false;
				}
			}

			ImGui::End();
		}
	}

	void HierarchyWindow::updateObjectsMenu()
	{
		ImGui::ImageButton((void*)addIcon->getHandle().idx, ImVec2(16, 16), ImVec2(0, 1), ImVec2(1, 0));

		if (ImGui::BeginPopupContextItem("add_object_popup", 0))
		{
			MainWindow::updateObjectsMenu();

			ImGui::EndPopup();
		}

		ImGui::SameLine();

		if (treeView->getFilter().empty())
		{
			ImGui::SetNextItemWidth(-1);
		}
		else
		{
			float w = ImGui::GetContentRegionAvail().x;
			ImGui::SetNextItemWidth(w - 25);
		}

		if (ImGui::InputText("##HierarchyFilter", &treeView->getFilter()))
		{
			treeView->setFilter(treeView->getFilter());
		}

		ImVec2 pos = ImGui::GetItemRectMin();
		pos = ImVec2(pos.x - ImGui::GetWindowPos().x, pos.y - ImGui::GetWindowPos().y);

		if (!treeView->getFilter().empty())
		{
			ImGui::SameLine();
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 5);
			if (ImGui::ImageButton((void*)closeIcon->getHandle().idx, ImVec2(15, 15), ImVec2(0, 1), ImVec2(1, 0)))
			{
				treeView->setFilter("");
			}
		}
		else
		{
			ImGui::SameLine();
			ImGui::SetCursorPosX(pos.x + 5);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY());
			ImGui::TextColored(ImVec4(1.0, 1.0, 1.0, 0.5), "Search...");
		}

		ImGui::Separator();
	}

	bool HierarchyWindow::isNodeChildOf(GameObject* node, GameObject* parent)
	{
		Transform* p = node->getTransform()->getParent();

		bool result = false;
		while (p != nullptr)
		{
			if (parent->getTransform() == p)
			{
				result = true;
				break;
			}
			else
				p = p->getParent();
		}

		return result;
	}

	void HierarchyWindow::onNodesSelected(std::vector<std::string> names, void* userData)
	{
		if (TerrainEditor::isEditModeActive())
			MainWindow::getInspectorWindow()->setEditor(nullptr);

		Gizmo* gizmo = MainWindow::getSingleton()->getSceneWindow()->getGizmo();

		std::vector<Transform*> objects;
		std::vector<Transform*> ignoredObjects;

		for (auto it = names.begin(); it != names.end(); ++it)
		{
			GameObject* obj = Engine::getSingleton()->getGameObject(*it);
			if (obj != nullptr)
			{
				auto it = std::find(ignoreObjects.begin(), ignoreObjects.end(), obj);

				if (it == ignoreObjects.end())
					objects.push_back(obj->getTransform());
				else
					ignoredObjects.push_back(obj->getTransform());
			}
		}

		if (ignoredObjects.size() > 0 && objects.size() == 0)
			gizmo->selectObjects(ignoredObjects, nullptr, false);
		else
			gizmo->selectObjects(objects, gizmo);

		if (objects.size() == 0)
			MainWindow::getUIEditorWindow()->clearSelection();
	}

	void HierarchyWindow::onDropNode(TreeNode* node, TreeNode* from)
	{
		if (from->treeView == treeView)
		{
			std::vector<TreeNode*> & items = treeView->getSelectedNodes();

			auto it = find_if(items.begin(), items.end(), [=](TreeNode* tn) -> bool { return tn->name == from->name; });
			if (it == items.end())
				items = { from };

			if (!treeView->getIsReorderingInProgress())
			{
				//Undo
				UndoData* undoData = Undo::addUndo("Move objects");
				undoData->intData.resize(1);
				undoData->objectData.resize(1);
				undoData->object2Data.resize(5);

				undoData->undoAction = [=](UndoData* data)
				{
					bool updCsg = false;

					std::vector<Component*> comps;

					for (auto& d : data->objectData[0])
					{
						Transform* trans = (Transform*)d;
						Transform* parent = (Transform*)data->object2Data[0][trans];

						if (trans->getParent() != parent)
							trans->setParent(parent);

						trans->iterateChildren([&updCsg, &comps](Transform* child) -> bool
							{
								CSGBrush* csgBrush = (CSGBrush*)child->getGameObject()->getComponent(CSGBrush::COMPONENT_TYPE);
								if (csgBrush != nullptr)
								{
									comps.push_back(csgBrush);

									updCsg = true;
									return false;
								}

								return true;
							},
							true);

						int idx = data->intData[0][trans];

						if (parent != nullptr)
							parent->setChildIndex(trans, idx);
						else
							Engine::getSingleton()->setRootObjectIndex(trans->getGameObject(), idx);

						TreeNode* node = (TreeNode*)data->object2Data[2][trans];
						TreeNode* _parent = (TreeNode*)data->object2Data[3][trans];
						if (node->parent != _parent)
						{
							node->parent->removeChild(node);
							_parent->addChild(node);
						}

						auto it = std::find(_parent->children.begin(), _parent->children.end(), node);
						int ii1 = std::distance(_parent->children.begin(), it);
						if (ii1 != idx)
							VectorUtils::move(_parent->children, ii1, idx);

						updateTreeNodes(node);
					}

					MainWindow::getInspectorWindow()->updateCurrentEditor();

					if (updCsg)
						MainWindow::addOnEndUpdateCallback([=]() { CSGGeometry::getSingleton()->rebuild(comps); });
				};

				undoData->redoAction = [=](UndoData* data)
				{
					bool updCsg = false;

					std::vector<Component*> comps;

					for (auto& d : data->objectData[0])
					{
						Transform* trans = (Transform*)d;
						Transform* parent = (Transform*)data->object2Data[1][trans];

						if (trans->getParent() != parent)
							trans->setParent(parent);

						trans->iterateChildren([&updCsg, &comps](Transform* child) -> bool
							{
								CSGBrush* csgBrush = (CSGBrush*)child->getGameObject()->getComponent(CSGBrush::COMPONENT_TYPE);
								if (csgBrush != nullptr)
								{
									comps.push_back(csgBrush);

									updCsg = true;
									return false;
								}

								return true;
							},
							true);

						TreeNode* node = (TreeNode*)data->object2Data[2][trans];
						TreeNode* _parent = (TreeNode*)data->object2Data[4][trans];
						if (node->parent != _parent)
						{
							node->parent->removeChild(node);
							_parent->addChild(node);
						}

						updateTreeNodes(node);
					}

					MainWindow::getInspectorWindow()->updateCurrentEditor();

					if (updCsg)
						MainWindow::addOnEndUpdateCallback([=]() { CSGGeometry::getSingleton()->rebuild(comps); });
				};
				//

				for (auto& x : items)
				{
					if (x == node)
						continue;

					GameObject* obj = Engine::getSingleton()->getGameObject(x->name);
					GameObject* par = Engine::getSingleton()->getGameObject(node->name);
					Transform* trans = obj->getTransform();
					Transform* transPar = trans->getParent();

					bool add = false;
					if (par != nullptr)
					{
						if (!isNodeChildOf(par, obj) && par->getTransform() != transPar && par != obj)
						{
							add = true;
						}
					}
					else if (transPar != nullptr)
						add = true;

					if (add)
					{
						undoData->intData[0][trans] = transPar != nullptr ? transPar->getChildIndex(trans) : Engine::getSingleton()->getRootObjectIndex(obj);
						undoData->object2Data[0][trans] = transPar;
						undoData->object2Data[1][trans] = par != nullptr ? par->getTransform() : nullptr;
						undoData->object2Data[2][trans] = x;
						undoData->object2Data[3][trans] = x->parent;
						undoData->object2Data[4][trans] = node;
						undoData->objectData[0].push_back(trans);
					}
				}

				std::sort(undoData->objectData[0].begin(), undoData->objectData[0].end(), [=](void* a, void* b) -> bool
					{
						return undoData->intData[0][a] < undoData->intData[0][b];
					}
				);

				if (undoData->objectData[0].size() == 0)
					Undo::removeUndo(undoData);
			}

			onNodesMoved(items, node);
		}
		else
		{
			if (from->treeView->getTag() == "Assets")
			{
				Transform* objTo = nullptr;
				if (node != nullptr && node != treeView->getRootNode())
				{
					GameObject* obj = Engine::getSingleton()->getGameObject(node->name);
					objTo = obj->getTransform();
				}

				std::string path = from->getPath();
				std::string ext = IO::GetFileExtension(path);

				auto formats = Engine::getModel3dFileFormats();
				GameObject* obj = nullptr;

				if (std::find(formats.begin(), formats.end(), ext) != formats.end())
				{
					obj = Model3DLoader::load3DModel(Engine::getSingleton()->getAssetsPath(), path);
					if (obj != nullptr && objTo != nullptr)
					{
						Transform* t = obj->getTransform();
						t->setParent(objTo);
					}

					AssetsWindow::getSingleton()->reloadFiles();
				}

				if (ext == "prefab")
				{
					Prefab* prefab = Prefab::load(Engine::getSingleton()->getAssetsPath(), path);
					obj = Scene::loadPrefab(prefab, glm::vec3(0.0f), glm::identity<glm::quat>());
					if (obj != nullptr && objTo != nullptr)
					{
						Transform* t = obj->getTransform();
						t->setParent(objTo);
					}
				}

				if (ext == "terrain")
				{
					obj = Engine::getSingleton()->createGameObject();
					obj->setName(IO::GetFileName(from->alias));
					obj->setLayer(15, true);

					Terrain* terrain = new Terrain();
					obj->addComponent(terrain);
					terrain->load(path);

					if (objTo != nullptr)
					{
						Transform* t = obj->getTransform();
						t->setParent(objTo);
					}
				}

				if (obj != nullptr)
				{
					obj->getTransform()->setLocalPosition(glm::vec3(0));
					insertNode(obj->getTransform(), objTo);

					MainWindow::getSingleton()->addGameObjectUndo({ obj }, "Add object");
				}
			}
		}
	}

	void HierarchyWindow::onReorder(TreeNode* node, int newIndex)
	{
		GameObject* reorderNode = Engine::getSingleton()->getGameObject(node->name);

		if (reorderNode != nullptr)
		{
			Transform* parent = reorderNode->getTransform()->getParent();
			if (parent != nullptr)
				parent->setChildIndex(reorderNode->getTransform(), newIndex);
			else
				Engine::getSingleton()->setRootObjectIndex(reorderNode, newIndex);
		}
	}

	void HierarchyWindow::onReorder(std::map<TreeNode*, std::pair<TreeNode*, std::pair<int, int>>> nodes)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Reorder objects");
		undoData->intData.resize(2);
		undoData->objectData.resize(1);
		undoData->object2Data.resize(5);

		undoData->undoAction = [=](UndoData* data)
		{
			int offset = data->objectData[0].size() - 1;
			bool updCsg = false;

			std::vector<Component*> comps;

			for (auto& d : data->objectData[0])
			{
				Transform* trans = (Transform*)d;
				Transform* parent = (Transform*)data->object2Data[0][trans];

				trans->iterateChildren([&updCsg, &comps](Transform* child) -> bool
					{
						CSGBrush* csgBrush = (CSGBrush*)child->getGameObject()->getComponent(CSGBrush::COMPONENT_TYPE);
						if (csgBrush != nullptr)
						{
							comps.push_back(csgBrush);

							updCsg = true;
							return false;
						}

						return true;
					},
					true);

				if (trans->getParent() != parent)
					trans->setParent(parent);

				int idx = data->intData[0][trans];
				int _idx1 = parent != nullptr ? parent->getChildIndex(trans) : Engine::getSingleton()->getRootObjectIndex(trans->getGameObject());

				if (idx > _idx1)
					idx += offset;

				--offset;

				if (parent != nullptr)
					parent->setChildIndex(trans, idx);
				else
					Engine::getSingleton()->setRootObjectIndex(trans->getGameObject(), idx);

				TreeNode* node = (TreeNode*)data->object2Data[2][trans];
				TreeNode* _parent = (TreeNode*)data->object2Data[3][trans];
				if (node->parent != _parent)
				{
					node->parent->removeChild(node);
					_parent->addChild(node);
				}

				auto it = std::find(_parent->children.begin(), _parent->children.end(), node);
				int ii1 = std::distance(_parent->children.begin(), it);
				if (ii1 != idx)
					VectorUtils::move(_parent->children, ii1, idx);

				updateTreeNodes(node);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();

			if (updCsg)
				MainWindow::addOnEndUpdateCallback([=]() { CSGGeometry::getSingleton()->rebuild(comps); });
		};

		undoData->redoAction = [=](UndoData* data)
		{
			bool updCsg = false;

			std::vector<Component*> comps;

			for (auto& d : data->objectData[0])
			{
				Transform* trans = (Transform*)d;
				Transform* parent = (Transform*)data->object2Data[1][trans];

				trans->iterateChildren([&updCsg, &comps](Transform* child) -> bool
					{
						CSGBrush* csgBrush = (CSGBrush*)child->getGameObject()->getComponent(CSGBrush::COMPONENT_TYPE);
						if (csgBrush != nullptr)
						{
							comps.push_back(csgBrush);

							updCsg = true;
							return false;
						}

						return true;
					},
					true);

				if (trans->getParent() != parent)
					trans->setParent(parent);

				int idx = data->intData[1][trans];

				if (parent != nullptr)
					parent->setChildIndex(trans, idx);
				else
					Engine::getSingleton()->setRootObjectIndex(trans->getGameObject(), idx);

				TreeNode* node = (TreeNode*)data->object2Data[2][trans];
				TreeNode* _parent = (TreeNode*)data->object2Data[4][trans];
				if (node->parent != _parent)
				{
					node->parent->removeChild(node);
					_parent->addChild(node);
				}

				auto it = std::find(_parent->children.begin(), _parent->children.end(), node);
				int ii1 = std::distance(_parent->children.begin(), it);
				if (ii1 != idx)
					VectorUtils::move(_parent->children, ii1, idx);

				updateTreeNodes(node);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();

			if (updCsg)
				MainWindow::addOnEndUpdateCallback([=]() { CSGGeometry::getSingleton()->rebuild(comps); });
		};
		//

		bool updCsg = false;

		std::vector<Component*> components;

		for (auto& x : nodes)
		{
			GameObject* obj = Engine::getSingleton()->getGameObject(x.first->name);
			GameObject* par = Engine::getSingleton()->getGameObject(x.second.first->name);
			Transform* trans = obj->getTransform();
			Transform* transPar = trans->getParent();

			trans->iterateChildren([&updCsg, &components](Transform* child) -> bool
				{
					CSGBrush* csgBrush = (CSGBrush*)child->getGameObject()->getComponent(CSGBrush::COMPONENT_TYPE);
					if (csgBrush != nullptr)
					{
						components.push_back(csgBrush);

						updCsg = true;
						return false;
					}

					return true;
				},
				true);

			bool add = false;
			if (par != nullptr)
			{
				if (!isNodeChildOf(par, obj) && par != obj)
				{
					add = true;
				}
			}
			else
				add = true;

			if (add)
			{
				undoData->intData[0][trans] = x.second.second.first;
				undoData->intData[1][trans] = x.second.second.second;
				undoData->object2Data[0][trans] = par != nullptr ? par->getTransform() : nullptr;
				undoData->object2Data[1][trans] = transPar;
				undoData->object2Data[2][trans] = x.first;
				undoData->object2Data[3][trans] = x.second.first;
				undoData->object2Data[4][trans] = x.first->parent;
				undoData->objectData[0].push_back(trans);
			}
		}

		std::sort(undoData->objectData[0].begin(), undoData->objectData[0].end(), [=](void* a, void* b) -> bool
			{
				return undoData->intData[0][a] < undoData->intData[0][b];
			}
		);

		if (undoData->objectData[0].size() == 0)
			Undo::removeUndo(undoData);

		if (updCsg)
			MainWindow::addOnEndUpdateCallback([=]() { CSGGeometry::getSingleton()->rebuild(components); });
	}

	void HierarchyWindow::updateTreeNodes(TreeNode* root)
	{
		std::vector<TreeNode*> nstack;
		nstack.push_back(root);

		auto gameObjects = Engine::getSingleton()->getGameObjects();

		while (nstack.size() > 0)
		{
			TreeNode* child = *nstack.begin();
			nstack.erase(nstack.begin());

			GameObject* node = nullptr;

			for (auto& obj : gameObjects)
			{
				if (obj->getGuidHash() == child->hash)
				{
					node = obj;
					break;
				}
			}

			if (node != nullptr)
				child->setColor(node->getActive() ? 1.0f : 0.5f, 3);

			//

			int j = 0;
			for (auto it = child->children.begin(); it != child->children.end(); ++it, ++j)
			{
				TreeNode* ch = *it;
				nstack.insert(nstack.begin() + j, ch);
			}
		}
	}

	std::vector<bool> HierarchyWindow::onNodesMoved(std::vector<TreeNode*> items, TreeNode* moveto)
	{
		Transform* nodeto = nullptr;
		GameObject * nodetoObj = Engine::getSingleton()->getGameObject(moveto->name);

		if (nodetoObj != nullptr)
			nodeto = nodetoObj->getTransform();

		bool updCsg = false;

		std::vector<Component*> comps;

		for (auto it = items.begin(); it != items.end(); ++it)
		{
			GameObject* node = Engine::getSingleton()->getGameObject((*it)->name);

			if (nodetoObj != nullptr)
			{
				if (!isNodeChildOf(nodetoObj, node) && nodeto != node->getTransform()->getParent() && nodetoObj != node)
					node->getTransform()->setParent(nodeto);
			}
			else
			{
				node->getTransform()->setParent(nullptr);
			}

			node->getTransform()->iterateChildren([&updCsg, &comps](Transform* child) -> bool
				{
					CSGBrush* csgBrush = (CSGBrush*)child->getGameObject()->getComponent(CSGBrush::COMPONENT_TYPE);
					if (csgBrush != nullptr)
					{
						comps.push_back(csgBrush);

						updCsg = true;
						return false;
					}

					return true;
				},
				true);

			updateTreeNodes(*it);
		}

		MainWindow::addOnEndUpdateCallback([=]() {
			updateTreeNodes(moveto);
			MainWindow::getInspectorWindow()->updateCurrentEditor();
		});

		if (updCsg)
			MainWindow::addOnEndUpdateCallback([=]() { CSGGeometry::getSingleton()->rebuild(comps); });

		return { true };
	}

	bool HierarchyWindow::onNodeRenamed(TreeNode* item, std::string oldName)
	{
		if (item->alias.empty())
			return false;

		//Undo
		UndoData* undoData = Undo::addUndo("Rename object");
		undoData->stringData.resize(2);

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

		GameObject* obj = Engine::getSingleton()->getGameObject(item->name);
		
		undoData->stringData[0][obj] = obj->getName();
		undoData->stringData[1][obj] = CP_SYS(item->alias);

		obj->setName(CP_SYS(item->alias));

		MainWindow::getInspectorWindow()->updateCurrentEditor();

		return true;
	}

	void HierarchyWindow::onNodePopupMenu(TreeNode* node, int val)
	{
		if (val == 0)
		{
			MainWindow::getSingleton()->onExportSelected();
		}

		if (val == 1)
		{
			if (treeView->getSelectedNodes().size() <= 1)
			{
				node->setEditMode(true);
			}
		}

		if (val == 2)
		{
			MainWindow::addOnEndUpdateCallback([=]()
				{
					MainWindow::getSingleton()->onDuplicate();
				}
			);
		}

		if (val == 3)
		{
			auto nodes = treeView->getSelectedNodes();

			if (nodes.size() == 0)
				nodes = { node };

			MainWindow::addOnEndUpdateCallback([=]() {
				for (auto it = nodes.begin(); it != nodes.end(); ++it)
				{
					TreeNode* _node = *it;
					
					GameObject* obj = Engine::getSingleton()->getGameObject((*it)->name);
					if (obj != nullptr)
					{
						Engine::getSingleton()->destroyGameObject(obj);
					}

					treeView->deleteNode(_node);
				}

				Gizmo* gizmo = MainWindow::getSingleton()->getSceneWindow()->getGizmo();
				gizmo->clearSelection();
				MainWindow::getUIEditorWindow()->clearSelection();
				MainWindow::getInspectorWindow()->setEditor(nullptr);
			});

			treeView->getSelectedNodes().clear();
		}
	}

	void HierarchyWindow::refreshHierarchy()
	{
		MainWindow::getSingleton()->getSceneWindow()->getGizmo()->clearSelection();

		getTreeView()->clear();

		std::vector<Transform*> rootTransforms = Engine::getSingleton()->getRootTransforms();

		for (auto it = rootTransforms.begin(); it != rootTransforms.end(); ++it)
			insertNode(*it, nullptr);
	}

	TreeNode* HierarchyWindow::insertNode(Transform* t, Transform* parent, int index)
	{
		std::vector<std::pair<Transform*, TreeNode*>> nstack;
		nstack.push_back(std::make_pair(t, nullptr));

		TreeNode* ret = nullptr;

		while (nstack.size() > 0)
		{
			Transform* child = nstack.begin()->first;
			TreeNode* nodeParent = nstack.begin()->second;
			nstack.erase(nstack.begin());

			//
			if (child->getGameObject()->isSerializable())
			{
				TreeNode* node = nullptr;

				if (ret == nullptr)
					ret = node = insertNode1(child, parent, index);
				else
					node = insertNode2(child, nodeParent);

				int j = 0;
				std::vector<Transform*> children = child->getChildren();
				for (auto it = children.begin(); it != children.end(); ++it, ++j)
				{
					Transform* ch = *it;
					nstack.insert(nstack.begin() + j, std::make_pair(ch, node));
				}
			}
			//
		}

		return ret;
	}

	Texture* getIconByType(GameObject* obj)
	{
		Texture* icon = Texture::load(Helper::ExePath(), "Editor/Icons/Hierarchy/empty.png", false, Texture::CompressionMethod::None, true);

		int compsCount = obj->getComponents().size() - obj->getMonoScripts().size();

		if (compsCount <= 2)
		{
			if (obj->getComponent(MeshRenderer::COMPONENT_TYPE) != nullptr)
				icon = Texture::load(Helper::ExePath(), "Editor/Icons/Hierarchy/mesh_renderer.png", false, Texture::CompressionMethod::None, true);
			else if (obj->getComponent(DecalRenderer::COMPONENT_TYPE) != nullptr)
				icon = Texture::load(Helper::ExePath(), "Editor/Icons/Hierarchy/decal_renderer.png", false, Texture::CompressionMethod::None, true);
			else if (obj->getComponent(Camera::COMPONENT_TYPE) != nullptr)
				icon = Texture::load(Helper::ExePath(), "Editor/Icons/Hierarchy/camera.png", false, Texture::CompressionMethod::None, true);
			else if (obj->getComponent(Light::COMPONENT_TYPE) != nullptr)
				icon = Texture::load(Helper::ExePath(), "Editor/Icons/Hierarchy/light.png", false, Texture::CompressionMethod::None, true);
			else if (obj->getComponent(Terrain::COMPONENT_TYPE) != nullptr)
				icon = Texture::load(Helper::ExePath(), "Editor/Icons/Inspector/terrain.png", false, Texture::CompressionMethod::None, true);
			else if (obj->getComponent(ParticleSystem::COMPONENT_TYPE) != nullptr)
				icon = Texture::load(Helper::ExePath(), "Editor/Icons/Inspector/particle_system.png", false, Texture::CompressionMethod::None, true);
			else if (obj->getComponent(Canvas::COMPONENT_TYPE) != nullptr)
				icon = Texture::load(Helper::ExePath(), "Editor/Icons/Hierarchy/canvas.png", false, Texture::CompressionMethod::None, true);
			else if (obj->getComponent(Image::COMPONENT_TYPE) != nullptr)
				icon = Texture::load(Helper::ExePath(), "Editor/Icons/Hierarchy/image.png", false, Texture::CompressionMethod::None, true);
			else if (obj->getComponent(Text::COMPONENT_TYPE) != nullptr)
				icon = Texture::load(Helper::ExePath(), "Editor/Icons/Hierarchy/text.png", false, Texture::CompressionMethod::None, true);
			else if (obj->getComponent(Button::COMPONENT_TYPE) != nullptr)
				icon = Texture::load(Helper::ExePath(), "Editor/Icons/Hierarchy/button.png", false, Texture::CompressionMethod::None, true);
			else if (obj->getComponent(TextInput::COMPONENT_TYPE) != nullptr)
				icon = Texture::load(Helper::ExePath(), "Editor/Icons/Hierarchy/text_input.png", false, Texture::CompressionMethod::None, true);
			else if (obj->getComponent(Mask::COMPONENT_TYPE) != nullptr)
				icon = Texture::load(Helper::ExePath(), "Editor/Icons/Hierarchy/mask.png", false, Texture::CompressionMethod::None, true);
			else if (obj->getComponent(AudioSource::COMPONENT_TYPE) != nullptr)
				icon = Texture::load(Helper::ExePath(), "Editor/Icons/Inspector/audio_source.png", false, Texture::CompressionMethod::None, true);
			else if (obj->getComponent(AudioListener::COMPONENT_TYPE) != nullptr)
				icon = Texture::load(Helper::ExePath(), "Editor/Icons/Inspector/audio_listener.png", false, Texture::CompressionMethod::None, true);
			else if (obj->getComponent(Water::COMPONENT_TYPE) != nullptr)
				icon = Texture::load(Helper::ExePath(), "Editor/Icons/Inspector/water.png", false, Texture::CompressionMethod::None, true);
			else if (obj->getComponent(Spline::COMPONENT_TYPE) != nullptr)
				icon = Texture::load(Helper::ExePath(), "Editor/Icons/Inspector/spline.png", false, Texture::CompressionMethod::None, true);
			else if (obj->getComponent(VideoPlayer::COMPONENT_TYPE) != nullptr)
				icon = Texture::load(Helper::ExePath(), "Editor/Icons/Inspector/video_player.png", false, Texture::CompressionMethod::None, true);
			else if (obj->getComponent(CSGBrush::COMPONENT_TYPE) != nullptr)
			{
				CSGBrush* csgBrush = (CSGBrush*)obj->getComponent(CSGBrush::COMPONENT_TYPE);

				if (csgBrush->getBrushType() == CSGBrush::BrushType::Sphere)
					icon = Texture::load(Helper::ExePath(), "Editor/Icons/CSG/brush_sphere.png", false, Texture::CompressionMethod::None, true);
				else if (csgBrush->getBrushType() == CSGBrush::BrushType::Cone)
					icon = Texture::load(Helper::ExePath(), "Editor/Icons/CSG/brush_cone.png", false, Texture::CompressionMethod::None, true);
				else if (csgBrush->getBrushType() == CSGBrush::BrushType::Cylinder)
					icon = Texture::load(Helper::ExePath(), "Editor/Icons/CSG/brush_cylinder.png", false, Texture::CompressionMethod::None, true);
				else
					icon = Texture::load(Helper::ExePath(), "Editor/Icons/CSG/brush_cube.png", false, Texture::CompressionMethod::None, true);
			}
		}

		return icon;
	}

	TreeNode* HierarchyWindow::insertNode1(Transform* t, Transform* parent, int index)
	{
		auto formats = Engine::getModel3dFileFormats();
		formats.push_back("*.");
		formats.push_back("::GameObject");
		formats.push_back("prefab");
		formats.push_back("terrain");

		TreeNode* node = new TreeNode(treeView);
		node->alias = t->getGameObject()->getName();
		node->name = t->getGameObject()->getGuid();
		node->hash = t->getGameObject()->getGuidHash();
		node->icon = getIconByType(t->getGameObject());
		node->setColor(t->getGameObject()->getActive() ? 1.0f : 0.5f, 3);
		node->enableDrag = true;
		node->format = "::GameObject";
		node->setEditable(true);
		node->setPopupMenu({ "-", "Export...", "-", "Rename", "Duplicate", "Delete"}, [=](TreeNode* node, int val) { onNodePopupMenu(node, val); });
		node->setPopupMenuIcons({ exportIcon, renameIcon, duplicateIcon, deleteIcon });
		node->setSupportedFormats(formats);
		node->setOnDropCallback([=](TreeNode* node, TreeNode* from) { onDropNode(node, from); });
		node->setOnEndEditCallback([=](TreeNode* node, std::string prevAlias) -> bool { return onNodeRenamed(node, prevAlias); });
		node->setCustomPopupMenuPosition(TreeNode::CustomPopupMenuPosition::CPMP_BEGIN);
		node->setCustomPopupMenuProcedure([=](TreeNode* node) { MainWindow::updateObjectsMenu(false, true); });

		if (parent == nullptr)
			treeView->getRootNode()->addChild(node);
		else
		{
			TreeNode* parentNode = treeView->getNodeByName(parent->getGameObject()->getGuid(), treeView->getRootNode());
			if (parentNode == nullptr)
				treeView->getRootNode()->addChild(node);
			else
				parentNode->addChild(node);
		}

		return node;
	}

	TreeNode* HierarchyWindow::insertNode2(Transform* t, TreeNode* nodeParent, int index)
	{
		auto formats = Engine::getModel3dFileFormats();
		formats.push_back("*.");
		formats.push_back("::GameObject");
		formats.push_back("prefab");
		formats.push_back("terrain");

		TreeNode* node = new TreeNode(treeView);
		node->alias = t->getGameObject()->getName();
		node->name = t->getGameObject()->getGuid();
		node->hash = t->getGameObject()->getGuidHash();
		node->icon = getIconByType(t->getGameObject());
		node->setColor(t->getGameObject()->getActive() ? 1.0f : 0.5f, 3);
		node->enableDrag = true;
		node->format = "::GameObject";
		node->setEditable(true);
		node->setPopupMenu({ "-", "Export...", "-", "Rename", "Duplicate", "Delete" }, [=](TreeNode* node, int val) { onNodePopupMenu(node, val); });
		node->setPopupMenuIcons({ exportIcon, renameIcon, duplicateIcon, deleteIcon });
		node->setSupportedFormats(formats);
		node->setOnDropCallback([=](TreeNode* node, TreeNode* from) { onDropNode(node, from); });
		node->setOnEndEditCallback([=](TreeNode* node, std::string prevAlias) -> bool { return onNodeRenamed(node, prevAlias); });
		node->setCustomPopupMenuPosition(TreeNode::CustomPopupMenuPosition::CPMP_BEGIN);
		node->setCustomPopupMenuProcedure([=](TreeNode* node) { MainWindow::updateObjectsMenu(false, true); });

		if (nodeParent == nullptr)
			treeView->getRootNode()->addChild(node);
		else
			nodeParent->addChild(node);

		return node;
	}

	void HierarchyWindow::updateNodeIcon(Transform* t)
	{
		TreeNode* node = treeView->getNodeByName(t->getGameObject()->getGuid(), treeView->getRootNode());
		if (node != nullptr)
			node->icon = getIconByType(t->getGameObject());
	}

	void GX::HierarchyWindow::setIgnoreObject(GameObject* object, bool value)
	{
		auto it = std::find(ignoreObjects.begin(), ignoreObjects.end(), object);

		if (it != ignoreObjects.end())
		{
			if (!value)
				ignoreObjects.erase(it);
		}
		else
		{
			if (value)
				ignoreObjects.push_back(object);
		}
	}
}