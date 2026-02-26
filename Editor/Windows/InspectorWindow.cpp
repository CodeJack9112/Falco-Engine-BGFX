#include "InspectorWindow.h"

#include <algorithm>

#include <imgui.h>
#include <imgui_internal.h>

#include "MainWindow.h"

#include "../PropertyEditors/PropertyEditor.h"
#include "../Classes/TreeNode.h"
#include "../Classes/TreeView.h"
#include "../PropertyEditors/PropEditorHost.h"
#include "../PropertyEditors/ObjectEditor.h"
#include "../Serialization/EditorSettings.h"

#include "../Engine/Core/Engine.h"

namespace GX
{
	InspectorWindow::InspectorWindow()
	{
		
	}

	InspectorWindow::~InspectorWindow()
	{
		
	}

	void InspectorWindow::init()
	{
	}

	void InspectorWindow::update()
	{
		bool rootOpened = true;

		if (opened)
		{
			if (ImGui::Begin("Inspector", &opened, ImGuiWindowFlags_NoCollapse))
			{
				ImVec2 rmin = ImGui::GetWindowPos();
				ImVec2 rmax = ImGui::GetWindowSize();

				focused = ImGui::IsMouseHoveringRect(ImGui::GetWindowPos(), ImVec2(rmin.x + rmax.x, rmin.y + rmax.y));

				if (setScrollPosValue > -1)
				{
					ImGui::SetScrollY(setScrollPosValue);
					setScrollPosValue = -1;
				}

				if (currentEditor != nullptr)
				{
					ImGuiWindow* wnd = ImGui::GetCurrentWindow();

					if (wnd->Size.x >= 200.0f)
						currentEditor->update();
				}
			}
			else
				focused = false;

			scrollPos = ImGui::GetScrollY();

			ImGui::End();
		}
		else
			focused = false;
	}

	void InspectorWindow::setEditor(PropertyEditor* editor)
	{
		saveCollapsedProperties();

		if (currentEditor != nullptr)
			delete currentEditor;

		currentEditor = editor;

		loadCollapsedProperties();
	}

	void InspectorWindow::updateCurrentEditor()
	{
		if (currentEditor != nullptr)
			currentEditor->updateEditor();
	}

	void InspectorWindow::updateObjectEditorTransform()
	{
		if (currentEditor != nullptr)
		{
			if (currentEditor->getEditorName() == "ObjectEditor")
			{
				ObjectEditor* ed = (ObjectEditor*)currentEditor;
				ed->updateTransform();
			}
		}
	}

	void InspectorWindow::saveCollapsedProperties()
	{
		if (currentEditor != nullptr)
		{
			TreeView* tree = currentEditor->getTreeView();

			if (tree != nullptr)
			{
				EditorSettings* settings = MainWindow::getSettings();

				auto editors = settings->propertyEditorsData;
				auto i = std::find_if(editors.begin(), editors.end(), [=](PropertyEditorData& ed) -> bool { return ed.editorName == currentEditor->getEditorName(); });

				PropertyEditorData dt;

				if (i != editors.end())
				{
					dt = *i;
					editors.erase(i);
				}

				dt.editorName = currentEditor->getEditorName();

				std::vector<TreeNode*> propList;
				getAllProperties(currentEditor->getTreeView()->getRootNode(), propList);

				for (auto it = propList.begin(); it != propList.end(); ++it)
				{
					TreeNode* node = *it;
					std::string path = node->getPath();

					PropertyState state;

					auto j = std::find_if(dt.properties.begin(), dt.properties.end(), [=](PropertyState& st) -> bool { return st.path == path; });

					if (j != dt.properties.end())
					{
						state = *j;
						dt.properties.erase(j);
					}

					state.path = path;
					state.expanded = node->expanded;

					dt.properties.push_back(state);
				}

				editors.push_back(dt);
				settings->propertyEditorsData = editors;

				settings->save();
			}
		}
	}

	void InspectorWindow::loadCollapsedProperties()
	{
		if (currentEditor != nullptr)
		{
			TreeView* tree = currentEditor->getTreeView();

			if (tree != nullptr)
			{
				EditorSettings* settings = MainWindow::getSettings();

				auto props = settings->propertyEditorsData;
				auto editorData = std::find_if(props.begin(), props.end(), [=](PropertyEditorData& ed) -> bool { return ed.editorName == currentEditor->getEditorName(); });

				if (editorData != props.end())
				{
					PropertyEditorData dt = *editorData;

					for (auto it = dt.properties.begin(); it != dt.properties.end(); ++it)
					{
						std::string path = it->path;

						TreeNode* node = currentEditor->getTreeView()->getNodeByPath(path, tree->getRootNode());
						if (node != nullptr)
							node->expanded = it->expanded;
					}
				}
			}
		}
	}

	void InspectorWindow::getAllProperties(TreeNode* root, std::vector<TreeNode*>& list)
	{
		auto children = root->children;

		for (auto it = children.begin(); it != children.end(); ++it)
		{
			TreeNode* node = *it;

			list.push_back(node);

			if (dynamic_cast<PropEditorHost*>(node))
			{
				PropertyEditor* hosted = ((PropEditorHost*)node)->getHostedEditor();
				TreeView* hostedTree = hosted->getTreeView();

				getAllProperties(hostedTree->getRootNode(), list);
			}
			else
				getAllProperties(*it, list);
		}
	}
}