#include "DialogCSGStairBuilder.h"

#include <imgui.h>
#include <imgui_internal.h>

#include "../Engine/Core/Engine.h"
#include "../Engine/Core/GameObject.h"
#include "../Engine/Gizmo/Gizmo.h"
#include "../Engine/Renderer/CSGGeometry.h"
#include "../Engine/Classes/IO.h"
#include "../Engine/UI/ImGUIWidgets.h"

#include "../Engine/Components/Transform.h"
#include "../Engine/Components/CSGBrush.h"

#include "../Engine/Assets/Material.h"
#include "../Engine/Assets/Texture.h"

#include "../Windows/MainWindow.h"
#include "../Windows/HierarchyWindow.h"
#include "../Windows/SceneWindow.h"
#include "../Windows/InspectorWindow.h"
#include "../Windows/AssetsWindow.h"

#include "../Classes/TreeView.h"
#include "../Classes/TreeNode.h"
#include "../Classes/Undo.h"

namespace GX
{
	DialogCSGStairBuilder::DialogCSGStairBuilder()
	{
	}

	DialogCSGStairBuilder::~DialogCSGStairBuilder()
	{
	}

	void DialogCSGStairBuilder::show(GameObject* root)
	{
		materialIcon = MainWindow::loadEditorIcon("Assets/material.png");
		rootObject = root;
		visible = true;

		HierarchyWindow::setIgnoreObject(rootObject, true);

		rebuild();
	}

	void DialogCSGStairBuilder::clearUndo()
	{
		std::vector<UndoData*> del;
		auto& undo = Undo::getStack();

		for (auto& data : undo)
		{
			if (data->hasObject(rootObject->getTransform()))
				del.push_back(data);
		}

		while (del.size() > 0)
		{
			Undo::removeUndo(del[0]);
			del.erase(del.begin());
		}
	}

	void DialogCSGStairBuilder::rebuild()
	{
		MainWindow::addOnEndUpdateCallback([=]()
			{
				Gizmo* gizmo = MainWindow::getSceneWindow()->getGizmo();
				gizmo->clearSelection();

				MainWindow::getInspectorWindow()->setEditor(nullptr);

				/// Rebuild stair

				Transform* t = rootObject->getTransform();
				auto& children = t->getChildren();
				while (children.size() > 0)
				{
					gizmo->setIgnoreObject(children[0], false);
					HierarchyWindow::setIgnoreObject(children[0]->getGameObject(), false);
					Engine::getSingleton()->destroyGameObject(children[0]->getGameObject());
				}

				float stepHeight = height / (float)numSteps;
				float stepLength = length / (float)numSteps;

				for (int i = 0; i < numSteps; ++i)
				{
					GameObject* stepObj = Engine::getSingleton()->createGameObject();
					stepObj->setName("Step " + std::to_string(i));

					gizmo->setIgnoreObject(stepObj->getTransform(), true);
					HierarchyWindow::setIgnoreObject(stepObj, true);

					CSGBrush* brush = new CSGBrush();
					stepObj->addComponent(brush);

					glm::vec3 scale = glm::vec3(1.0f);
					glm::vec3 position = glm::vec3(0.0f);

					if (closed)
					{
						float _stepLength = stepLength * (numSteps - i);
						scale = glm::vec3(width, stepHeight, _stepLength);
						position = glm::vec3(0.0f, (float)i * stepHeight * 2.0f, _stepLength - length + stepLength);
					}
					else
					{
						scale = glm::vec3(width, stepHeight, stepLength);
						position = glm::vec3(0.0f, (float)i * stepHeight * 2.0f, length - ((float)i * stepLength * 2.0f));
					}

					auto& faces = brush->getFaces();
					for (int j = 0; j < faces.size(); ++j)
					{
						brush->setMaterial(j, material);
					}

					brush->setUVScale(0, glm::vec2(scale.x, scale.y) * uvScale);
					brush->setUVScale(1, glm::vec2(scale.x, scale.y) * uvScale);
					brush->setUVScale(2, glm::vec2(scale.z, scale.x) * uvScale);
					brush->setUVScale(3, glm::vec2(scale.z, scale.x) * uvScale);
					brush->setUVScale(4, glm::vec2(scale.z, scale.y) * uvScale);
					brush->setUVScale(5, glm::vec2(scale.z, scale.y) * uvScale);

					Transform* stepT = stepObj->getTransform();
					stepT->setParent(t);

					stepT->setLocalScale(scale);
					stepT->setLocalPosition(position);

					brush->rebuild();
				}

				CSGModel* model = CSGGeometry::getSingleton()->getModel(rootObject);
				if (model != nullptr)
					CSGGeometry::getSingleton()->rebuild(model);
			}
		);
	}

	void DialogCSGStairBuilder::update()
	{
		if (!visible)
			return;

		ImGui::SetNextWindowSize(ImVec2(250, 220));
		if (ImGui::Begin("Create Stair", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking))
		{
			ImGui::BeginColumns("##col1", 2, ImGuiOldColumnFlags_NoBorder | ImGuiOldColumnFlags_NoResize);
			ImGui::Text("Steps count");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1);
			if (ImGui::InputInt("##steps", &numSteps))
			{
				if (numSteps < 2)
					numSteps = 2;

				rebuild();
			}
			ImGui::EndColumns();

			ImGui::BeginColumns("##col2", 2, ImGuiOldColumnFlags_NoBorder | ImGuiOldColumnFlags_NoResize);
			ImGui::Text("Width");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1);
			if (ImGui::DragFloat("##width", &width))
			{
				if (width < 0.1f)
					width = 0.1f;

				rebuild();
			}
			ImGui::EndColumns();

			ImGui::BeginColumns("##col3", 2, ImGuiOldColumnFlags_NoBorder | ImGuiOldColumnFlags_NoResize);
			ImGui::Text("Height");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1);
			if (ImGui::DragFloat("##height", &height))
			{
				if (height < 0.1f)
					height = 0.1f;

				rebuild();
			}
			ImGui::EndColumns();

			ImGui::BeginColumns("##col4", 2, ImGuiOldColumnFlags_NoBorder | ImGuiOldColumnFlags_NoResize);
			ImGui::Text("Length");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1);
			if (ImGui::DragFloat("##length", &length))
			{
				if (length < 0.1f)
					length = 0.1f;

				rebuild();
			}
			ImGui::EndColumns();

			ImGui::BeginColumns("##col5", 2, ImGuiOldColumnFlags_NoBorder | ImGuiOldColumnFlags_NoResize);
			ImGui::Text("UV scale");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1);
			if (ImGui::DragFloat("##uv_scale", &uvScale))
			{
				if (uvScale < 0.01f)
					uvScale = 0.01f;

				rebuild();
			}
			ImGui::EndColumns();

			ImGui::BeginColumns("##col6", 2, ImGuiOldColumnFlags_NoBorder | ImGuiOldColumnFlags_NoResize);
			ImGui::Text("Closed");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1);
			if (ImGui::Checkbox("##closed", &closed))
			{
				rebuild();
			}
			ImGui::EndColumns();

			ImGui::BeginColumns("##col7", 2, ImGuiOldColumnFlags_NoBorder | ImGuiOldColumnFlags_NoResize);
			ImGui::Text("Material");
			ImGui::NextColumn();
			std::string mfileName = "None";
			if (material != nullptr)
				mfileName = IO::GetFileNameWithExt(material->getName());

			float szX = ImGui::GetContentRegionAvail().x;

			if (ImGui::ImageButtonWithText((void*)materialIcon->getHandle().idx, mfileName.c_str(), ImVec2(16, 16), ImVec2(0, 1), ImVec2(1, 0), ImVec2(szX, -1)))
			{
				if (material != nullptr)
					MainWindow::getAssetsWindow()->focusOnFile(material->getName());
			}
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_TreeView"))
				{
					TreeNode* move_from = (TreeNode*)payload->Data;

					if (move_from->treeView->getTag() == "Assets")
					{
						if (IO::GetFileExtension(move_from->alias) == "material")
						{
							std::string path = move_from->getPath();
							material = Material::load(Engine::getSingleton()->getAssetsPath(), path);
							rebuild();
						}
					}
				}
				ImGui::EndDragDropTarget();
			}
			ImGui::EndColumns();

			// Buttons

			if (ImGui::Button("Cancel"))
			{
				MainWindow::addOnEndUpdateCallback([=]()
					{
						HierarchyWindow::setIgnoreObject(rootObject, false);

						clearUndo();

						CSGModel* model = CSGGeometry::getSingleton()->getModel(rootObject);

						Gizmo* gizmo = MainWindow::getSceneWindow()->getGizmo();
						gizmo->clearSelection();

						auto& children = rootObject->getTransform()->getChildren();
						for (auto t : children)
						{
							gizmo->setIgnoreObject(t, false);
							HierarchyWindow::setIgnoreObject(t->getGameObject(), false);
						}

						MainWindow::getInspectorWindow()->setEditor(nullptr);
						Engine::getSingleton()->destroyGameObject(rootObject);
						MainWindow::getHierarchyWindow()->refreshHierarchy();

						if (model != nullptr)
							CSGGeometry::getSingleton()->rebuild(model);

						rootObject = nullptr;
					}
				);

				visible = false;
			}

			ImGui::SameLine();

			if (ImGui::Button("Create"))
			{
				MainWindow::addOnEndUpdateCallback([=]()
					{
						HierarchyWindow::setIgnoreObject(rootObject, false);

						clearUndo();

						Gizmo* gizmo = MainWindow::getSceneWindow()->getGizmo();

						auto& children = rootObject->getTransform()->getChildren();
						for (auto t : children)
						{
							gizmo->setIgnoreObject(t, false);
							HierarchyWindow::setIgnoreObject(t->getGameObject(), false);
						}

						MainWindow::getSingleton()->addGameObjectUndo({ rootObject }, "Create CSG stair");
						MainWindow::getHierarchyWindow()->refreshHierarchy();
						gizmo->selectObject(rootObject->getTransform());

						visible = false;

						rootObject = nullptr;
					}
				);
			}
		}

		ImGui::End();
	}
}