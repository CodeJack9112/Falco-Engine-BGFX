#include "TreeNode.h"
#include "TreeView.h"
#include "../Windows/MainWindow.h"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/algorithm/string.hpp>

#include <imgui.h>
#include <dear-imgui/misc/cpp/imgui_stdlib.h>

#include "../Engine/Classes/StringConverter.h"
#include "../Engine/Assets/Texture.h"

namespace GX
{
	TreeNode::TreeNode(TreeView* owner)
	{
		treeView = owner;
		guid1 = genGuid();
	}

	TreeNode::~TreeNode()
	{
		if (userData != nullptr)
			delete userData;

		for (auto it = children.begin(); it != children.end(); ++it)
			delete* it;

		children.clear();
	}

	void TreeNode::update(bool opened)
	{
		bool _btn = false;

		if (hasButton)
		{
			ImGui::SameLine();
			if (ImGui::Button(buttonText.c_str(), ImVec2(0, 18)))
			{
				_btn = true;
				if (useButtonForPopup && popupMenuItems.size() > 0)
					ImGui::OpenPopup((name + "_popup").c_str());
				else
				{
					if (onButtonClickCallback != nullptr)
					{
						onButtonClickCallback(this);
					}
				}
			}
		}
		else
		{
			_btn = true;
		}

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 4));

		if (popupMenuItems.size() > 0)
		{
			bool _popup = false;
			if (hasButton && useButtonForPopup)
				_popup = ImGui::BeginPopup((name + "_popup").c_str());
			else
				_popup = ImGui::BeginPopupContextItem((name + "_popup").c_str(), popupButton);

			if (_popup)
			{
				if (customPopupMenuPos == CustomPopupMenuPosition::CPMP_BEGIN)
				{
					if (customPopupMenuProcedure != nullptr)
						customPopupMenuProcedure(this);
				}

				int i = 0;
				for (auto it = popupMenuItems.begin(); it != popupMenuItems.end(); ++it)
				{
					bool checked = popupCheckedItem == i;

					if (*it == "-")
						ImGui::Separator();
					else
					{
						if (i < popupMenuIcons.size())
						{
							if (popupMenuIcons[i] != nullptr)
							{
								ImGui::Image((void*)popupMenuIcons[i]->getHandle().idx, ImVec2(16, 16), ImVec2(0, 1), ImVec2(1, 0));
								ImGui::SameLine();
							}
						}

						if (ImGui::Selectable((*it).c_str(), checked))
						{
							if (onPopupSelectedCallback != nullptr)
								onPopupSelectedCallback(this, i);
						}

						++i;
					}
				}

				if (customPopupMenuPos == CustomPopupMenuPosition::CPMP_END)
				{
					if (customPopupMenuProcedure != nullptr)
						customPopupMenuProcedure(this);
				}

				ImGui::EndPopup();
			}
		}
		else
		{
			if (customPopupMenuProcedure != nullptr)
			{
				bool _popup = false;
				if (hasButton && useButtonForPopup)
					_popup = ImGui::BeginPopup((name + "_popup").c_str());
				else
					_popup = ImGui::BeginPopupContextItem((name + "_popup").c_str(), popupButton);

				if (_popup)
				{
					customPopupMenuProcedure(this);

					ImGui::EndPopup();
				}
			}
		}

		ImGui::PopStyleVar();

		if (hasButton && !useButtonForPopup)
		{
			ImGui::SameLine();
			if (ImGui::Button(buttonText.c_str(), ImVec2(0, 18)))
			{
				if (onButtonClickCallback != nullptr)
				{
					onButtonClickCallback(this);
				}
			}
		}

		if (editMode)
		{
			ImGui::SameLine();

			ImGui::SetKeyboardFocusHere(0);

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(3, 1));
			ImGui::SetNextItemWidth(-1);
			ImGui::InputText(("##" + name + "_edit").c_str(), &editText);
			ImGui::PopStyleVar();

			ImGuiIO& io = ImGui::GetIO();

			if (ImGui::IsKeyDown(io.KeyMap[ImGuiKey_Escape]))
			{
				setEditMode(false, true);
			}

			if (ImGui::IsKeyDown(io.KeyMap[ImGuiKey_Enter]))
			{
				setEditMode(false);
			}

			if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsItemHovered())
			{
				setEditMode(false);
			}
		}
	}

	void TreeNode::addChild(TreeNode* child)
	{
		children.push_back(child);
		child->parent = this;
	}

	void TreeNode::removeChild(TreeNode* child)
	{
		auto it = find(children.begin(), children.end(), child);
		if (it != children.end())
			children.erase(it);
	}

	void TreeNode::genName(std::string name)
	{
		boost::uuids::random_generator uuid_gen;
		boost::uuids::uuid u = uuid_gen();

		std::string uName = boost::uuids::to_string(u);

		this->alias = name;
		this->name = uName;
	}

	std::string TreeNode::genGuid()
	{
		boost::uuids::random_generator uuid_gen;
		boost::uuids::uuid u = uuid_gen();

		return boost::uuids::to_string(u);
	}

	void TreeNode::setPopupMenu(std::vector<std::string> items, std::function<void(TreeNode * node, int val)> callback)
	{
		popupMenuItems = items;
		onPopupSelectedCallback = callback;
	}

	bool TreeNode::isFormatSupported(std::string fmt, TreeView* tree)
	{
		if (supportedFormats.size() > 0)
		{
			if (supportedFormats[0] == "*.")
			{
				if (this->treeView == tree)
					return true;
			}

			if (supportedFormats[0] == "::Asset")
			{
				if (tree->getTag() == "Assets")
					return true;
			}

			return std::find(supportedFormats.begin(), supportedFormats.end(), fmt) != supportedFormats.end();
		}

		return false;
	}

	bool TreeNode::isChildOf(TreeNode* node)
	{
		TreeNode* p = parent;

		bool result = false;
		while (p != nullptr)
		{
			if (node == p)
			{
				result = true;
				break;
			}
			else
				p = p->parent;
		}

		return result;
	}

	std::string TreeNode::getPath()
	{
		return treeView->getNodePath(this);
	}

	void TreeNode::setEditMode(bool mode, bool cancel)
	{
		if (!editable)
			return;

		editMode = mode;

		if (!editMode)
		{
			flags = _oldFlags;
			fullSpaceWidth = _oldFullSpaceWidth;

			std::string prevAlias = alias;

			if (alias != editText && !cancel)
			{	
				alias = editText;
			}

			if (endEditCallback != nullptr)
			{
				MainWindow::addOnEndUpdateCallback([=]() { 
					bool result = endEditCallback(this, CP_SYS(prevAlias));
					if (!result)
						alias = prevAlias;
				});
			}
		}
		else
		{
			_oldFlags = flags;
			_oldFullSpaceWidth = fullSpaceWidth;

			editText = alias;
			flags = ImGuiTreeNodeFlags_AllowItemOverlap;
			fullSpaceWidth = false;
		}
	}

	void TreeNode::setColor(float c, int i)
	{
		color[i] = c;
	}
}