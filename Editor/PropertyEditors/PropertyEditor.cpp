#include "PropertyEditor.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <algorithm>

#include "../Engine/Core/InputManager.h"

#include "../Classes/TreeView.h"
#include "../Classes/TreeNode.h"
#include "../Engine/UI/ImGUIWidgets.h"

#include "Property.h"

#include <SDL2/SDL.h>

namespace GX
{
	float PropertyEditor::previewHeight = 280.0f;

	PropertyEditor::PropertyEditor()
	{
		treeView = new TreeView();
		treeView->setAllowMultipleSelection(false);
		treeView->setUseColumns(true);
	}

	PropertyEditor::~PropertyEditor()
	{
		delete treeView;
		properties.clear();
	}

	void recursiveSearch(TreeNode* root, bool & result)
	{
		if (result)
			return;

		if (TreeView::getLastClickedNode() == root)
		{
			result = true;
			return;
		}

		for (auto it = root->children.begin(); it != root->children.end(); ++it)
		{
			recursiveSearch(*it, result);
		}
	}

	void PropertyEditor::update()
	{
		if (cntSize1 < 0)
			cntSize1 = ImGui::GetContentRegionAvail().y - previewHeight - 4;

		if (updatePreviewFunc != nullptr)
		{
			ImGui::Splitter(false, 4.0f, &cntSize1, &previewHeight, 10.0f, 10.0f, -1.0f, "##Console_content_splitter");

			if (ImGui::BeginChild("Properties", ImVec2(-1, cntSize1)))
				treeView->update();

			ImGui::EndChild();
		}
		else
			treeView->update();

		if (updatePreviewFunc != nullptr)
		{
			if (ImGui::BeginChild("Preview", ImVec2(0, previewHeight)))
			{
				updatePreviewFunc();
			}

			ImGui::EndChild();
		}

		if (ImGui::IsMouseDown(0) && ImGui::IsMouseDragging(0))
		{
			isWindowDragging = true;
		}

		if (ImGui::IsWindowHovered() && ImGui::IsMouseReleased(0))
		{
			if (!isWindowDragging)
			{
				if (!ImGui::IsAnyItemHovered())
				{
					treeView->selectNodes({  });
				}
				else
				{
					bool treeClicked = false;
					recursiveSearch(treeView->getRootNode(), treeClicked);

					if (!treeClicked)
						treeView->selectNodes({  });
				}
			}
		}

		if (ImGui::IsMouseReleased(0))
		{
			isWindowDragging = false;
		}

		if (InputManager::getSingleton()->getKeyDown(SDL_SCANCODE_DELETE))
		{
			std::vector<TreeNode*> & nodes = treeView->getSelectedNodes();
			if (nodes.size() > 0)
			{
				Property* node = (Property*)nodes[0];
				if (node != nullptr)
					node->callClearCallback();
			}
		}
	}

	void PropertyEditor::addProperty(Property* prop)
	{
		properties.push_back(prop);
		treeView->getRootNode()->addChild(prop);
	}

	void PropertyEditor::removeProperty(Property* prop)
	{
		auto it = std::find(properties.begin(), properties.end(), prop);
		if (it != properties.end())
			properties.erase(it);

		treeView->deleteNode(prop);
	}

	bool PropertyEditor::isPreviewHovered()
	{
		ImVec2 pos = ImGui::GetWindowPos();
		ImVec2 wsz = ImGui::GetWindowSize();

		int y = ImGui::GetItemRectSize().y;

		if (ImGui::IsMouseHoveringRect(ImVec2(pos.x, pos.y + y + 4), ImVec2(pos.x + wsz.x, pos.y + y + previewHeight)))
			return true;

		return false;
	}
}