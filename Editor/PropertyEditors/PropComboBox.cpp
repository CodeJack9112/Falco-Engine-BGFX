#include "PropComboBox.h"

#include <boost/algorithm/string.hpp>

#include <imgui.h>
#include <imgui_internal.h>

namespace GX
{
	PropComboBox::PropComboBox(PropertyEditor* ed, std::string name, std::vector<std::string> val) : Property(ed, name)
	{
		setValue(val);

		flags = ImGuiTreeNodeFlags_AllowItemOverlap;
		fullSpaceWidth = false;

		guid1 = "##" + genGuid();
	}

	PropComboBox::~PropComboBox()
	{
		setIsNested(false);
	}

	void PropComboBox::update(bool opened)
	{
		Property::update(opened);

		//ImGui::SameLine();

		const char* current = "";
		const char* _label = "";

		if (item_current < value.size())
			current = value[item_current].c_str();

		if (!label.empty())
			_label = label.c_str();
		else
			_label = current;

		ImGui::PushItemWidth(-1);
		if (ImGui::BeginCombo(guid1.c_str(), _label))
		{
			if (!isNested)
			{
				for (int n = 0; n < value.size(); n++)
				{
					bool is_selected = false;

					if (n < valuesEnabled.size())
						is_selected = valuesEnabled[n];
					else
						is_selected = (current == value[n]);

					if (ImGui::MenuItem(value[n].c_str(), "", is_selected))
					{
						current = value[n].c_str();
						item_current = n;

						if (onChangeCallback != nullptr)
						{
							onChangeCallback(this, value[n]);
						}

						if (onChangeCallback2 != nullptr)
						{
							onChangeCallback2(this, n);
						}
					}

					/*if (is_selected)
						ImGui::SetItemDefaultFocus();*/
				}
			}
			else
			{
				int n = 0;
				for (auto it = nodes.begin(); it != nodes.end(); ++it)
				{
					if ((*it)->value == "Hidden")
						continue;

					updateList(*it, current, n);
				}
			}

			ImGui::EndCombo();
		}
		ImGui::PopItemWidth();
	}

	void PropComboBox::updateList(Node* node, const char*& current, int& n)
	{
		std::string path = node->getPath();
		bool is_selected = current == path;

		bool menu = false;
		if (node->children.size() > 0)
			menu = ImGui::BeginMenu(node->value.c_str(), "");
		else
			menu = ImGui::MenuItem(node->value.c_str(), "", is_selected);

		if (menu)
		{
			if (node->children.size() > 0)
			{
				++n;

				for (auto it = node->children.begin(); it != node->children.end(); ++it)
					updateList(*it, current, n);

				ImGui::EndMenu();
			}
			else
			{
				current = node->getPath().c_str();
				item_current = n;

				if (onChangeCallback != nullptr)
				{
					onChangeCallback(this, path);
				}

				if (onChangeCallback2 != nullptr)
				{
					onChangeCallback2(this, n);
				}

				++n;
			}
		}
	}

	void PropComboBox::setValue(std::vector<std::string> val)
	{
		value = val;
	}

	void PropComboBox::setValue(int index, std::string val)
	{
		if (index >= value.size())
			value.resize(index + 1);

		value[index] = val;
	}

	void PropComboBox::setValueEnabled(std::vector<bool> val)
	{
		valuesEnabled = val;
	}

	void PropComboBox::setValueEnabled(int index, bool val)
	{
		if (index >= valuesEnabled.size())
			valuesEnabled.resize(index + 1);

		valuesEnabled[index] = val;
	}

	bool PropComboBox::getValueEnabled(int index)
	{
		return valuesEnabled[index];
	}

	void PropComboBox::setCurrentItem(std::string val)
	{
		auto it = find(value.begin(), value.end(), val);

		if (it != value.end())
			item_current = distance(value.begin(), it);
	}

	void PropComboBox::setIsNested(bool value)
	{
		isNested = value;
		rebuildNestedList();
	}

	void PropComboBox::rebuildNestedList()
	{
		if (isNested)
		{
			for (auto& val : value)
			{
				std::vector<std::string> results;
				boost::split(results, val, [](char c) { return c == '/' || c == '\\'; });

				Node* node = nullptr;
				for (auto& r : results)
				{
					std::vector<Node*> nodeList = nodes;
					if (node != nullptr)
						nodeList = node->children;

					Node* foundNode = nullptr;
					for (auto& n : nodeList)
					{
						if (n->value == r)
						{
							foundNode = n;
							break;
						}
					}
					if (foundNode == nullptr)
					{
						Node* _n = new Node();
						_n->value = r;
						_n->parent = node;

						if (node != nullptr)
							node->children.push_back(_n);
						else
							nodes.push_back(_n);

						node = _n;
					}
					else
					{
						node = foundNode;
					}
				}
			}
		}
		else
		{
			std::vector<Node*> nstack;
			for (auto it = nodes.begin(); it != nodes.end(); ++it)
				nstack.push_back(*it);

			while (nstack.size() > 0)
			{
				Node* child = *nstack.begin();
				nstack.erase(nstack.begin());

				int j = 0;
				for (auto it = child->children.begin(); it != child->children.end(); ++it, ++j)
				{
					Node* ch = *it;
					nstack.insert(nstack.begin() + j, ch);
				}

				child->children.clear();
				delete child;
			}

			nodes.clear();
		}
	}

	std::string PropComboBox::Node::getPath()
	{
		std::string path = value;
		
		Node* _parent = parent;
		while (_parent != nullptr)
		{
			path = _parent->value + "/" + path;
			_parent = _parent->parent;
		}

		return path;
	}
}