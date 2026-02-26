#include "TreeView.h"
#include "TreeNode.h"

#include <imgui.h>
#include <imgui_internal.h>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/algorithm/string.hpp>
#include <iostream>

#include "../Windows/MainWindow.h"
#include "../PropertyEditors/PropEditorHost.h"
#include "../PropertyEditors/PropertyEditor.h"

#include "../Engine/Core/Engine.h"
#include "../Engine/Core/GameObject.h"
#include "../Engine/Core/Time.h"
#include "../Engine/Renderer/Renderer.h"
#include "../Engine/UI/ImGUIWidgets.h"
#include "../Engine/Classes/IO.h"
#include "../Engine/Classes/StringConverter.h"
#include "../Engine/Core/InputManager.h"
#include "../Engine/Assets/Texture.h"

#include "../Engine/Classes/Hash.h"

#include <SDL2/SDL.h>

namespace GX
{
	TreeNode* TreeView::lastClickedNode = nullptr;

	TreeView::TreeView()
	{
		boost::uuids::random_generator uuid_gen;
		boost::uuids::uuid u = uuid_gen();

		std::string rootName = boost::uuids::to_string(u);

		rootNode = new TreeNode(this);
		rootNode->name = rootName;
	}

	TreeView::~TreeView()
	{
		deleteNode(rootNode);
	}

	void TreeView::deleteNode(TreeNode* node)
	{
		TreeNode* parent = node->parent;

		if (parent != nullptr)
		{
			auto it = std::find(parent->children.begin(), parent->children.end(), node);

			if (it != parent->children.end())
				parent->children.erase(it);
		}

		auto it = std::find(selectedNodes.begin(), selectedNodes.end(), node);
		if (it != selectedNodes.end())
			selectedNodes.erase(it);

		if (lastClickedNode == node)
			lastClickedNode = nullptr;

		if (shiftSelectNode == node)
			shiftSelectNode = nullptr;

		if (editNode == node)
			editNode = nullptr;

		if (reorderNode == node)
			reorderNode = nullptr;

		if (focusNode == node)
			focusNode = nullptr;

		if (move_to == node)
			move_to = nullptr;

		if (move_from == node)
			move_from = nullptr;

		delete node;

		setFilter(getFilter());
	}

	void TreeView::update()
	{
		if (hasTopPadding)
			ImGui::Dummy(ImVec2(0, 2));

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 2));
		for (auto it = rootNode->children.begin(); it != rootNode->children.end(); ++it)
			updateRecursive(*it, wasClicked);
		ImGui::PopStyleVar();

		if (editNode != nullptr)
		{
			if (clickTime < 4.0f)
				clickTime += 1.0f * Time::getDeltaTime();
		}

		if (!processDragDropInTarget)
		{
			processDragDrop();
			updateSelection(wasClicked);
			isWindowDragging = false;

			if (endUpdateCallback != nullptr)
				endUpdateCallback();

			if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
				wasClicked = false;
		}
	}

	void TreeView::clear()
	{
		lastClickedNode = nullptr;
		shiftSelectNode = nullptr;
		editNode = nullptr;
		reorderNode = nullptr;
		focusNode = nullptr;
		move_to = nullptr;
		move_from = nullptr;

		for (auto it = rootNode->children.begin(); it != rootNode->children.end(); ++it)
			delete *it;

		rootNode->children.clear();

		setFilter(getFilter());

		selectedNodes.clear();
		lastSelectedNodes.clear();
	}

	void TreeView::updateSelection(bool wasClicked)
	{
		if (ImGui::IsMouseReleased(0))
		{
			if (ImGui::IsWindowHovered(/*ImGuiHoveredFlags_AllowWhenOverlapped | */ImGuiHoveredFlags_ChildWindows) && !ImGui::IsAnyItemHovered())
			{
				if (!isWindowDragging)
				{
					if (!wasClicked)
					{
						bool edit = false;

						for (auto it = selectedNodes.begin(); it != selectedNodes.end(); ++it)
						{
							TreeNode* n = *it;
							if (n != nullptr)
							{
								if (n->editMode)
								{
									edit = true;
									break;
								}
							}
						}

						if (!edit)
						{
							if (!InputManager::getSingleton()->getKey(SDL_SCANCODE_LCTRL) && !InputManager::getSingleton()->getKey(SDL_SCANCODE_LSHIFT))
							{
								selectedNodes.clear();
								selectNodes({ });

								editNode = nullptr;
								clickTime = 0.0f;
							}
						}
					}
				}
			}
		}
	}

	void TreeView::updateDragDropTarget()
	{
		if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
		{
			if (ImGui::BeginDragDropTarget())
			{
				isWindowDragging = true;

				const ImGuiPayload* pl = ImGui::GetDragDropPayload();
				if (pl != nullptr && pl->IsDataType("DND_TreeView"))
				{
					if (move_from == nullptr && move_to == nullptr)
					{
						move_to = rootNode;
						move_from = (TreeNode*)pl->Data;

						/*if (move_to->treeView != move_from->treeView)
						{
							move_to = nullptr;
							move_from = nullptr;
						}*/
					}
				}

				ImGui::EndDragDropTarget();
			}
		}
		
		if (processDragDropInTarget)
		{
			processDragDrop();
			updateSelection(wasClicked);
			isWindowDragging = false;

			if (endUpdateCallback != nullptr)
				endUpdateCallback();
		}

		if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
			wasClicked = false;
	}

	std::map<std::string, bool> TreeView::getItemsExpandStates()
	{
		std::map<std::string, bool> _map;

		getItemsExpandStatesRecursive(rootNode, _map);

		return _map;
	}

	void TreeView::setItemsExpandStates(std::map<std::string, bool> states)
	{
		for (auto it = states.begin(); it != states.end(); ++it)
		{
			TreeNode* node = getNodeByName(it->first, rootNode);

			if (node != nullptr)
				node->expanded = it->second;
		}
	}

	void TreeView::getItemsExpandStatesRecursive(TreeNode* root, std::map<std::string, bool>& outMap)
	{
		for (auto it = root->children.begin(); it != root->children.end(); ++it)
		{
			outMap[(*it)->name] = (*it)->expanded;

			getItemsExpandStatesRecursive(*it, outMap);
		}
	}

	void getPlainNodeList(TreeNode* root, std::vector<TreeNode*>& outList)
	{
		for (auto it = root->children.begin(); it != root->children.end(); ++it)
		{
			outList.push_back(*it);
			if ((*it)->expanded)
				getPlainNodeList(*it, outList);
		}
	}

	std::vector<TreeNode*> getNodesRange(TreeNode* root, TreeNode* start, TreeNode* end, std::string filter, std::vector<TreeNode*> filteredNodes)
	{
		root->treeView->expandParent(start);
		root->treeView->expandParent(end);

		std::vector<TreeNode*> plainList;
		std::vector<TreeNode*> selList;

		getPlainNodeList(root, plainList);

		auto s = std::find(plainList.begin(), plainList.end(), start);
		auto e = std::find(plainList.begin(), plainList.end(), end);

		if (std::distance(plainList.begin(), s) > std::distance(plainList.begin(), e))
		{
			auto m = s;
			s = e;
			e = m;
			selList.push_back(start);
		}
		else
		{
			selList.push_back(end);
		}

		for (auto it = s; it != e; ++it)
		{
			TreeNode* cur = *it;

			bool nodeFiltered = true;

			if (!filter.empty())
			{
				if (std::find(filteredNodes.begin(), filteredNodes.end(), cur) == filteredNodes.end())
					nodeFiltered = false;
			}

			if (nodeFiltered)
				selList.push_back(cur);
		}

		plainList.clear();

		return selList;
	}

	void filterNodesRecursive(TreeNode* rootMain, TreeNode* root, std::string filter, std::vector<TreeNode*> & outList, bool incParents)
	{
		//for each (TreeNode * node in root->children)
		for (auto it = root->children.begin(); it != root->children.end(); ++it)
		{
			TreeNode* node = *it;

			std::string cmp1 = boost::algorithm::to_lower_copy(node->alias);
			std::string cmp2 = boost::algorithm::to_lower_copy(filter);

			if (cmp1.find(cmp2) != std::string::npos)
			{
				TreeNode* par = node->parent;
				while (par != nullptr && par != rootMain)
				{
					if (incParents)
					{
						if (find(outList.begin(), outList.end(), par) == outList.end())
							outList.push_back(par);
					}

					if (!filter.empty())
					{
						par->expanded = true;
						par = par->parent;
					}
				}

				outList.push_back(node);
			}

			filterNodesRecursive(rootMain, node, filter, outList, incParents);
		}
	}

	void TreeView::setFilter(std::string value)
	{
		filter = value;
		filteredNodes.clear();
		if (!filter.empty())
			filterNodesRecursive(rootNode, rootNode, value, filteredNodes, filterIncludeParents);
	}

	void TreeView::updateRecursive(TreeNode* root, bool& wasClicked)
	{
		if (root == nullptr)
			return;

		std::vector<TreeNode*> nstack;
		nstack.push_back(root);

		while (nstack.size() > 0)
		{
			TreeNode* curNode = *nstack.begin();
			nstack.erase(nstack.begin());

			if (curNode == nullptr)
			{
				ImGui::TreePop();
				continue;
			}

			if (!curNode->visible)
				continue;

			bool nodeFiltered = true;

			if (!filter.empty())
			{
				if (find(filteredNodes.begin(), filteredNodes.end(), curNode) == filteredNodes.end())
					nodeFiltered = false;
			}

			if (!nodeFiltered)
			{
				int j = 0;
				for (auto it = curNode->children.begin(); it != curNode->children.end(); ++it, ++j)
				{
					TreeNode* nd = *it;
					nstack.insert(nstack.begin() + j, nd);
				}

				continue;
			}

			if (curNode->skipNode)
			{
				curNode->update(false);
				continue;
			}

			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
			
			bool showLeaf = false;
			for (auto& c : curNode->children)
			{
				if (c->visible)
				{
					showLeaf = true;
					break;
				}
			}

			if (curNode->children.size() == 0 || !showLeaf)
			{
				if (!curNode->alwaysShowArrow)
				{
					flags |= ImGuiTreeNodeFlags_Leaf;
				}
			}

			if (!useColumns)
			{
				if (curNode->fullSpaceWidth)
					flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
			}
			else
			{
				flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
			}

			flags |= curNode->flags;

			if (std::find(selectedNodes.begin(), selectedNodes.end(), curNode) != selectedNodes.end())
			{
				if (!curNode->editMode)
					flags |= ImGuiTreeNodeFlags_Selected;
			}

			if (separatorOnTopNode == curNode)
			{
				ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(1, 1, 0, 1));
				ImGui::Separator();
				ImGui::PopStyleColor();

				separatorOnTopNode = nullptr;
			}

			if (useColumns)
			{
				if (!curNode->fullSpaceWidth)
				{
					ImGui::BeginColumns(std::string(curNode->name + "_columns").c_str(), 2, ImGuiColumnsFlags_NoResize | ImGuiColumnsFlags_NoBorder);
				}
			}

			if (curNode->showCheckBox)
			{
				bool clicked = ImGui::Checkbox(("##" + curNode->name + "_checkbox").c_str(), &curNode->checked);

				if (curNode->checkChildren)
				{
					if (clicked)
					{
						if (curNode->checked)
							checkNodeParent(curNode, curNode->checked);

						checkNodeChildren(curNode, curNode->checked);
					}
				}

				if (clicked)
				{
					if (curNode->onCheckedCallback != nullptr)
						curNode->onCheckedCallback(curNode, curNode->checked);
				}

				ImGui::SameLine();
			}

			if (curNode->icon != nullptr)
			{
				ImGui::Image((void*)curNode->icon->getHandle().idx, ImVec2(16.0f, 16.0f), ImVec2(0, 1), ImVec2(1, 0));
				ImGui::SameLine();
			}

			if (curNode->expanded)
				ImGui::SetNextItemOpen(true);
			else
				ImGui::SetNextItemOpen(false);

			bool nodeOpen = false;

			std::string _nalias = curNode->alias;
			if (curNode->editMode)
				_nalias = "";

			float* col = curNode->getColor();
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(col[0], col[1], col[2], col[3]));
			if (!curNode->fullSpaceWidth)
			{
				nodeOpen = ImGui::TreeNodeEx2(curNode->name.c_str(), flags, _nalias.c_str());
			}
			else
			{
				nodeOpen = ImGui::TreeNodeEx(curNode->name.c_str(), flags, _nalias.c_str());
			}
			ImGui::PopStyleColor();

			if (focusNode == curNode)
			{
				ImGui::SetScrollHereY();
				focusNode = nullptr;
			}

			if (ImGui::IsItemHovered())
				curNode->hovered = true;
			else
				curNode->hovered = false;

			ImGuiWindow* window = GImGui->CurrentWindow;
			ImRect dndRect = ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
			if (useColumns)
				dndRect.Max.x = window->Pos.x + window->Size.x - 8 - window->Scroll.x;

			ImGuiDragDropFlags src_flags = 0;
			src_flags |= ImGuiDragDropFlags_SourceNoDisableHover;

			if (curNode->enableDrag)
			{
				if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoDisableHover))
				{
					if (!(src_flags & ImGuiDragDropFlags_SourceNoPreviewTooltip))
						ImGui::Text("...");

					ImGui::SetDragDropPayload("DND_TreeView", curNode, sizeof(TreeNode));
					ImGui::EndDragDropSource();

					if (!dragStarted)
					{
						reorderNode = curNode;
						dragStarted = true;
						onDragStarted(curNode);
					}
				}

				/*-----REORDERING BEGIN-----*/
				if (allowReorder && dragStarted && ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
				{
					ImVec2 min = ImGui::GetItemRectMin();
					ImVec2 max = ImGui::GetItemRectMax();
					ImVec2 mouse = ImGui::GetMousePos();

					if (mouse.x > min.x && mouse.x < max.x)
					{
						//Top for root
						if (rootNode->children.size() > 0)
						{
							if (curNode == rootNode->children[0])
							{
								if (mouse.y < min.y && mouse.y > min.y - 10)
								{
									separatorOnTopNode = curNode;

									if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
									{
										tempDisableDragDrop = true;

										std::vector<TreeNode*> nds = selectedNodes;

										MainWindow::addOnEndUpdateCallback([=]()
										{
											auto _nds = *const_cast<std::vector<TreeNode*>*>(&nds);// selectedNodes;

											if (reorderNode != nullptr)
											{
												if (_nds.size() == 0)
													_nds.push_back(reorderNode);
												else
												{
													if (find(_nds.begin(), _nds.end(), reorderNode) == _nds.end())
													{
														_nds.clear();
														_nds.push_back(reorderNode);
													}
												}
											}

											int _idx = 0;
											TreeNode* _to = curNode;
											
											std::map<TreeNode*, std::pair<TreeNode*, std::pair<int, int>>> nodesOrder;

											for (auto _it = _nds.begin(); _it != _nds.end(); ++_it)
											{
												TreeNode* _n1 = *_it;
												TreeNode* _par = _n1->parent;
												auto __1 = find(_par->children.begin(), _par->children.end(), _n1);
												nodesOrder[_n1].first = _par;
												nodesOrder[_n1].second.first = std::distance(_par->children.begin(), __1);
											}

											if (find(_nds.begin(), _nds.end(), _to) == _nds.end())
											{
												isReorderingInProgress = true;

												for (auto _it = _nds.begin(); _it != _nds.end(); ++_it)
												{
													TreeNode* _n1 = *_it;

													if (_n1->treeView != _to->treeView)
														continue;

													if (_n1->parent != _to->parent)
													{
														if (_to->parent->dropCallback != nullptr)
															_to->parent->dropCallback(_to->parent, _n1);

														if (dropCallback != nullptr)
															dropCallback(_n1, _to->parent);

														moveNodes(_to->parent, _n1, false);
													}

													TreeNode* _par = _to->parent;

													auto __1 = find(_par->children.begin(), _par->children.end(), _n1);
													//auto __2 = find(_par->children.begin(), _par->children.end(), _to);

													nodesOrder[_n1].second.second = _idx;

													_par->children.erase(__1);
													_par->children.insert(_par->children.begin() + _idx, _n1);

													if (reorderCallback != nullptr)
														reorderCallback(_n1, _idx);

													++_idx;
												}

												isReorderingInProgress = false;

												if (reorderCallback2 != nullptr)
													reorderCallback2(nodesOrder);
											}

											tempDisableDragDrop = false;
											reorderNode = nullptr;

											std::vector<std::string> sel;
											for (auto ii = nds.begin(); ii != nds.end(); ++ii)
												sel.push_back((*ii)->name);

											selectNodes(sel, false);
										});
									}
								}
							}
						}

						//Bottom for others
						if (mouse.y > max.y - 1 && mouse.y < max.y + 4)
						{
							ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(1, 1, 0, 1));
							ImGui::Separator();
							ImGui::PopStyleColor();

							if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
							{
								tempDisableDragDrop = true;

								std::vector<TreeNode*> nds = selectedNodes;

								MainWindow::addOnEndUpdateCallback([=]()
								{
									auto _nds = *const_cast<std::vector<TreeNode*>*>(&nds);

									if (reorderNode != nullptr)
									{
										if (_nds.size() == 0)
											_nds.push_back(reorderNode);
										else
										{
											if (find(_nds.begin(), _nds.end(), reorderNode) == _nds.end())
											{
												_nds.clear();
												_nds.push_back(reorderNode);
											}
										}
									}

									int _idx = 0;
									TreeNode* _to = curNode;
									if (find(_nds.begin(), _nds.end(), _to) == _nds.end())
									{
										bool move_to_begin = false;

										if (_to->children.size() > 0 && _to->expanded)
										{
											_to = _to->children[0];
											move_to_begin = true;
										}

										std::map<TreeNode*, std::pair<TreeNode*, std::pair<int, int>>> nodesOrder;

										for (auto _it = _nds.begin(); _it != _nds.end(); ++_it)
										{
											TreeNode* _n1 = *_it;
											TreeNode* _par = _n1->parent;
											auto __1 = find(_par->children.begin(), _par->children.end(), _n1);
											nodesOrder[_n1].first = _par;
											nodesOrder[_n1].second.first = std::distance(_par->children.begin(), __1);
										}

										isReorderingInProgress = true;

										for (auto _it = _nds.begin(); _it != _nds.end(); ++_it)
										{
											TreeNode* _n1 = *_it;

											if (_n1->treeView != _to->treeView)
												continue;

											if (_n1 == _to)
												continue;

											bool _mtChild = false;
											TreeNode* tmp = _to->parent;
											while (tmp != nullptr)
											{
												if (_n1 == tmp)
												{
													_mtChild = true;
													tmp = nullptr;
													break;
												}

												tmp = tmp->parent;
											}

											if (_mtChild)
												continue;

											if (_n1->parent != _to->parent)
											{
												if (_to->parent->dropCallback != nullptr)
													_to->parent->dropCallback(_to->parent, _n1);

												if (dropCallback != nullptr)
													dropCallback(_n1, _to->parent);

												moveNodes(_to->parent, _n1, false);
											}

											TreeNode* _par = _to->parent;

											auto __1 = find(_par->children.begin(), _par->children.end(), _n1);
											auto __2 = find(_par->children.begin(), _par->children.end(), _to);

											nodesOrder[_n1].second.second = _idx;

											_par->children.erase(__1);

											if (__1 > __2)
											{
												if (move_to_begin)
												{
													_par->children.insert(_par->children.begin() + _idx, _n1);

													if (reorderCallback != nullptr)
														reorderCallback(_n1, _idx);
												}
												else if (*__2 == _par->children[0])
												{
													nodesOrder[_n1].second.second = _idx + 1;

													_par->children.insert(_par->children.begin() + 1 + _idx, _n1);

													if (reorderCallback != nullptr)
														reorderCallback(_n1, 1 + _idx);
												}
												else
												{
													int _i = distance(_par->children.begin(), __2);
													nodesOrder[_n1].second.second = _idx + _i + 1;

													_par->children.insert(__2 + _idx + 1, _n1);

													if (reorderCallback != nullptr)
														reorderCallback(_n1, _i + _idx + 1);
												}
											}
											else
											{
												int _i = distance(_par->children.begin(), __2);
												nodesOrder[_n1].second.second = _idx + _i;

												_par->children.insert(__2 + _idx, _n1);

												if (reorderCallback != nullptr)
													reorderCallback(_n1, _i + _idx);
											}

											++_idx;
										}

										isReorderingInProgress = false;

										if (reorderCallback2 != nullptr)
											reorderCallback2(nodesOrder);
									}

									tempDisableDragDrop = false;
									reorderNode = nullptr;

									std::vector<std::string> sel;
									for (auto ii = nds.begin(); ii != nds.end(); ++ii)
										sel.push_back((*ii)->name);

									selectNodes(sel, false);
								});
							}
						}
					}
				}
				/*-----REORDERING END-----*/
			}

			if (useColumns)
			{
				if (!curNode->fullSpaceWidth)
				{
					ImGui::NextColumn();
				}
			}

			if (ImGui::IsItemToggledOpen())
			{
				curNode->expanded = !curNode->expanded;
			}

			curNode->update(nodeOpen);

			if (useColumns)
			{
				if (!curNode->fullSpaceWidth)
				{
					ImGui::EndColumns();
				}
			}

			if (useColumns)
			{
				if (!curNode->fullSpaceWidth)
				{
					dndRect.Max.y = ImGui::GetItemRectMax().y;
					ImVec2 cp = ImGui::GetCursorPos();
					ImGui::SetCursorPos(ImVec2(dndRect.Min.x - window->Pos.x + window->Scroll.x, dndRect.Min.y - window->Pos.y + window->Scroll.y));
					if (dndRect.Min.x < dndRect.Max.x && dndRect.Min.y < dndRect.Max.y)
						ImGui::InvisibleButton2(curNode->guid1.c_str(), dndRect.GetSize());
					ImGui::SetCursorPos(cp);
				}
			}

			if (curNode->supportedFormats.size() > 0)
			{
				//if (ImGui::BeginDragDropTarget())
				ImGuiID dndId = ImGui::GetCurrentWindow()->GetIDNoKeepAlive(("##" + curNode->name + "_dnd_target").c_str());
				if (ImGui::BeginDragDropTargetCustom(dndRect, dndId))
				{
					isWindowDragging = true;

					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_TreeView"))
					{
						if (move_from == nullptr && move_to == nullptr)
						{
							move_from = (TreeNode*)payload->Data;
							move_to = curNode;
						}
					}

					ImGui::EndDragDropTarget();
				}
			}

			ImGuiContext& g = *GImGui;
			bool checkArrow = false;
			if (curNode->children.size() > 0)
			{
				if (g.ActiveIdClickOffset.x > g.FontSize + g.Style.FramePadding.x * 2)
					checkArrow = true;
			}
			else
			{
				checkArrow = true;
			}

			bool dblClick = false;

			if (ImGui::IsMouseReleased(ImGuiMouseButton_Right) && ImGui::IsItemHovered() && !isWindowDragging && checkArrow && !dragStarted)
			{
				auto sn = std::find(selectedNodes.begin(), selectedNodes.end(), curNode);
				if (sn == selectedNodes.end())
				{
					selectNodes({ curNode->name });
				}
			}

			if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && ImGui::IsItemHovered() && !isWindowDragging && !dragStarted)
			{
				wasClicked = true;
			}

			if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && ImGui::IsItemHovered() && !isWindowDragging && checkArrow && !dragStarted)
			{
				dblClick = true;

				editNode = nullptr;
				clickTime = 0;

				MainWindow::addOnEndUpdateCallback([=]() {
					if (curNode->doubleClickCallback != nullptr)
						curNode->doubleClickCallback(curNode);
				});
			}

			if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && ImGui::IsItemHovered() && !isWindowDragging && checkArrow && !dragStarted)
			{
				lastClickedNode = curNode;
				bool _mulSel = false;

				if (allowMultipleSelection)
				{
					if (InputManager::getSingleton()->getKey(SDL_SCANCODE_LCTRL))
					{
						_mulSel = true;

						auto cur = find(selectedNodes.begin(), selectedNodes.end(), curNode);

						if (cur == selectedNodes.end())
							selectedNodes.push_back(curNode);
						else
							selectedNodes.erase(cur);

						std::sort(selectedNodes.begin(), selectedNodes.end(), [=](TreeNode * a, TreeNode * b) -> bool {
							if (tag == "Hierarchy")
							{
								size_t h1 = Hash::getHash(a->name);
								size_t h2 = Hash::getHash(b->name);

								GameObject* _a = Engine::getSingleton()->getGameObject(h1);
								GameObject* _b = Engine::getSingleton()->getGameObject(h2);

								int id1 = Engine::getSingleton()->getGameObjectIndex(_a);
								int id2 = Engine::getSingleton()->getGameObjectIndex(_b);

								return id1 < id2;
							}

							if (tag == "Assets")
							{
								return a->alias < b->alias;
							}

							return false;
						});

						std::vector<std::string> sel;
						for (auto ii = selectedNodes.begin(); ii != selectedNodes.end(); ++ii)
							sel.push_back((*ii)->name);

						MainWindow::addOnEndUpdateCallback([=]() { selectNodes(sel); });
					}
					else if (InputManager::getSingleton()->getKey(SDL_SCANCODE_LSHIFT))
					{
						_mulSel = true;

						if (shiftSelectNode != nullptr)
						{
							selectedNodes.clear();

							auto sel = getNodesRange(rootNode, shiftSelectNode, curNode, filter, filteredNodes);
							selectedNodes = sel;
							sel.clear();

							auto gameObjects = Engine::getSingleton()->getGameObjects();

							std::sort(selectedNodes.begin(), selectedNodes.end(), [=](TreeNode* a, TreeNode* b) -> bool {
								if (tag == "Hierarchy")
								{
									size_t h1 = a->hash;
									size_t h2 = b->hash;

									GameObject* _a = nullptr;
									GameObject* _b = nullptr;

									int id1 = 0;
									int id2 = 0;

									for (auto& obj : gameObjects)
									{
										if (obj->getGuidHash() == h1)
											_a = obj;
										else
											++id1;

										if (obj->getGuidHash() == h2)
											_b = obj;
										else
											++id2;
										
										if (_a != nullptr && _b != nullptr)
											break;
									}

									return id1 < id2;
								}

								if (tag == "Assets")
								{
									return a->alias < b->alias;
								}

								return false;
							});

							std::vector<std::string> sel1;
							for (auto ii = selectedNodes.begin(); ii != selectedNodes.end(); ++ii)
								sel1.push_back((*ii)->name);

							MainWindow::addOnEndUpdateCallback([=]() { selectNodes(sel1); });
						}
					}
				}

				if (!_mulSel)
				{
					if (curNode != nullptr)
					{
						selectedNodes.clear();
						selectedNodes = { curNode };

						shiftSelectNode = curNode;

						std::vector<std::string> sel;
						for (auto ii = selectedNodes.begin(); ii != selectedNodes.end(); ++ii)
							sel.push_back((*ii)->name);

						MainWindow::addOnEndUpdateCallback([=]() { selectNodes(sel); });
					}

					if (!dblClick)
					{
						if (editNode == nullptr)
						{
							editNode = curNode;
							clickTime = 0.0f;
						}
						else
						{
							if (editNode == curNode)
							{
								if (clickTime > 0.4f && clickTime < 1.2f)
								{
									editNode->setEditMode(true);
									editNode = nullptr;
									clickTime = 0.0f;
								}
								else
								{
									clickTime = 0.0f;
								}
							}
							else
							{
								editNode = curNode;
								clickTime = 0.0f;
							}
						}
					}
					else
					{
						clickTime = 0.0f;
					}
				}
			}

			if (nodeOpen)
			{
				int j = 0;
				for (auto it = curNode->children.begin(); it != curNode->children.end(); ++it, ++j)
				{
					TreeNode* nd = *it;
					nstack.insert(nstack.begin() + j, nd);
				}

				nstack.insert(nstack.begin() + j, nullptr);
			}
		}
	}

	void TreeView::selectNodes(std::vector<std::string> names, bool callback, void* userData)
	{
		selectedNodes.clear();

		for (auto it = names.begin(); it != names.end(); ++it)
		{
			TreeNode* node = getNodeByName(*it, rootNode);
			selectedNodes.push_back(node);
			if (node != nullptr)
				expandParent(node);
		}

		if (callback)
		{
			if (selectCallback != nullptr)
				selectCallback(names, userData);
		}
	}

	void TreeView::setSelectCallback(void(*callback)(std::vector<std::string>names, void* userData))
	{
		selectCallback = callback;
	}

	void TreeView::expandParent(TreeNode* node)
	{
		if (node->parent != rootNode)
		{
			node->parent->expanded = true;

			expandParent(node->parent);
		}
	}

	TreeNode* TreeView::getNodeByName(std::string name, TreeNode* root, bool recursive)
	{
		TreeNode* node = nullptr;
		getNodeByCondRecursive(name, root, node, SearchCond::SC_NAME, recursive);

		return node;
	}

	TreeNode* TreeView::getNodeByName(std::string name, bool recursive)
	{
		return getNodeByName(name, rootNode, recursive);
	}

	TreeNode* TreeView::getNodeByAlias(std::string alias, TreeNode* root, bool recursive)
	{
		TreeNode* node = nullptr;
		getNodeByCondRecursive(alias, root, node, SearchCond::SC_ALIAS, recursive);

		return node;
	}

	TreeNode* TreeView::getNodeByPath(std::string path, TreeNode* root)
	{
		std::vector<std::string> results;
		boost::split(results, path, [](char c) { return c == '/' || c == '\\'; });

		TreeNode * item = root;
		for (auto it = results.begin(); it != results.end(); ++it)
		{
			bool found = false;
			auto children = item->children;

			if (item->getClassName() == "PropEditorHost")
			{
				PropertyEditor* hosted = ((PropEditorHost*)item)->getHostedEditor();
				if (hosted != nullptr)
				{
					TreeView* hostedTree = hosted->getTreeView();
					children = hostedTree->getRootNode()->children;
				}
			}

			for (auto it2 = children.begin(); it2 != children.end(); ++it2)
			{
				if ((*it2)->alias == *it)
				{
					item = *it2;
					found = true;
					break;
				}
			}

			if (!found)
			{
				item = root;
				break;
			}
		}

		if (item == root)
			item = nullptr;

		return item;
	}

	std::string TreeView::getNodePath(TreeNode* node)
	{
		std::string path = "";

		TreeNode* parent = nullptr;

		if (node != nullptr)
		{
			parent = node->parent;

			if (parent == node->treeView->getRootNode())
			{
				if (node->getClassName() == "Property" || node->getClassName() == "PropEditorHost")
				{
					TreeNode* parentEditorNode = ((Property*)node)->getEditor()->getParentEditorNode();
					if (parentEditorNode != nullptr)
						parent = parentEditorNode;
				}
			}
		}

		//if (IO::GetFileExtension(CP_SYS(node->alias)).empty())
		//	path = "/";

		while (parent != nullptr)
		{
			path = node->alias + path;
			node = parent;
			parent = node->parent;

			if (parent == node->treeView->getRootNode())
			{
				if (node->getClassName() == "Property" || node->getClassName() == "PropEditorHost")
				{
					TreeNode* parentEditorNode = ((Property*)node)->getEditor()->getParentEditorNode();
					if (parentEditorNode != nullptr)
						parent = parentEditorNode;
				}
			}

			if (parent != nullptr)
				path = "/" + path;
		}

		return path;
	}

	void TreeView::getNodeByCondRecursive(std::string search, TreeNode* root, TreeNode*& outNode, SearchCond cond, bool recursive)
	{
		if (recursive)
		{
			if (outNode != nullptr)
			{
				if (cond == SearchCond::SC_NAME)
				{
					if (outNode->name == search)
						return;
				}
				else
				{
					if (outNode->alias == search)
						return;
				}
			}

			if (cond == SearchCond::SC_NAME)
			{
				if (root->name == search)
				{
					outNode = root;
				}
				else
				{
					for (auto it = root->children.begin(); it != root->children.end(); ++it)
					{
						getNodeByCondRecursive(search, *it, outNode, cond, recursive);
					}
				}
			}
			else
			{
				if (root->alias == search)
				{
					outNode = root;
				}
				else
				{
					for (auto it = root->children.begin(); it != root->children.end(); ++it)
					{
						getNodeByCondRecursive(search, *it, outNode, cond, recursive);
					}
				}
			}
		}
		else
		{
			if (cond == SearchCond::SC_NAME)
			{
				for (auto it = root->children.begin(); it != root->children.end(); ++it)
				{
					if ((*it)->name == search)
					{
						outNode = *it;
						break;
					}
				}
			}
			else
			{
				for (auto it = root->children.begin(); it != root->children.end(); ++it)
				{
					if ((*it)->alias == search)
					{
						outNode = *it;
						break;
					}
				}
			}
		}
	}

	void TreeView::checkNodeChildren(TreeNode* root, bool check)
	{
		for (auto it = root->children.begin(); it != root->children.end(); ++it)
		{
			TreeNode* node = *it;
			node->checked = check;

			checkNodeChildren(node, check);
		}
	}

	void TreeView::checkNodeParent(TreeNode* root, bool check)
	{
		if (root->parent != nullptr)
		{
			if (root->parent != rootNode)
			{
				root->parent->checked = check;
				checkNodeParent(root->parent, check);
			}
		}
	}

	void TreeView::moveNodes(TreeNode* moveto, TreeNode* movefrom, bool useSelected)
	{
		if (movefrom->treeView == this)
		{
			auto sel = selectedNodes;
			
			if (useSelected)
			{
				for (auto it = sel.begin(); it != sel.end(); ++it)
				{
					TreeNode* fn = *it;

					if (!moveto->isChildOf(fn) && moveto != fn->parent && moveto != fn)
					{
						fn->parent->removeChild(fn);
						moveto->addChild(fn);
					}
				}
			}
			else
			{
				if (!moveto->isChildOf(movefrom) && moveto != movefrom->parent && moveto != movefrom)
				{
					movefrom->parent->removeChild(movefrom);
					moveto->addChild(movefrom);
				}
			}
		}

		setFilter(getFilter());
	}

	void TreeView::onDragStarted(TreeNode* node)
	{
		std::sort(selectedNodes.begin(), selectedNodes.end(), [=](TreeNode* a, TreeNode* b) -> bool {
			if (tag == "Hierarchy")
			{
				size_t h1 = Hash::getHash(a->name);
				size_t h2 = Hash::getHash(b->name);

				GameObject* _a = Engine::getSingleton()->getGameObject(h1);
				GameObject* _b = Engine::getSingleton()->getGameObject(h2);

				int id1 = Engine::getSingleton()->getGameObjectIndex(_a);
				int id2 = Engine::getSingleton()->getGameObjectIndex(_b);

				return id1 < id2;
			}

			if (tag == "Assets")
			{
				return a->alias < b->alias;
			}

			return false;
		});

		if (beginDragCallback != nullptr)
		{
			beginDragCallback(node);
		}

		auto it = find(selectedNodes.begin(), selectedNodes.end(), node);

		if (it == selectedNodes.end())
		{
			resetLastNodes = true;
			lastSelectedNodes.clear();
			lastSelectedNodes = selectedNodes;
			selectedNodes.clear();
			selectedNodes.push_back(node);
		}
		else
		{
			//resetLastNodes = true;
			//lastSelectedNodes.clear();
			//lastSelectedNodes = selectedNodes;
		}
	}

	void TreeView::onDragEnded()
	{
		dragStarted = false;

		if (resetLastNodes)
		{
			resetLastNodes = false;

			selectedNodes.clear();
			selectedNodes = lastSelectedNodes;
			lastSelectedNodes.clear();
		}
	}

	void TreeView::processDragDrop()
	{
		if (move_from != nullptr/* && move_to != nullptr*/)
		{
			if (!tempDisableDragDrop)
			{
				if (move_to->isFormatSupported(move_from->format, move_from->treeView))
				{
					if (move_to->dropCallback != nullptr)
					{
						move_to->dropCallback(move_to, move_from);
					}

					if (dropCallback != nullptr)
					{
						dropCallback(move_from, move_to);
					}
				}

				if (allowMoveNodes)
				{
					moveNodes(move_to, move_from);
				}
			}

			move_from = nullptr;
			move_to = nullptr;
		}

		if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
		{
			if (dragStarted)
				onDragEnded();
		}
	}
}