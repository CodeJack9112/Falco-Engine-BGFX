#include "UIEditorWindow.h"

#include <imgui_internal.h>
#include <algorithm>

#include "../Engine/Core/Engine.h"
#include "../Engine/Core/Time.h"
#include "../Engine/Core/GameObject.h"
#include "../Engine/Renderer/Renderer.h"
#include "../Engine/Components/Transform.h"
#include "../Engine/Components/Canvas.h"
#include "../Engine/Components/Image.h"
#include "../Engine/Components/Text.h"
#include "../Engine/Components/Button.h"
#include "../Engine/Components/Mask.h"
#include "../Engine/Assets/Texture.h"
#include "../Engine/Classes/StringConverter.h"
#include "../Engine/Core/InputManager.h"
#include "../Engine/Gizmo/Gizmo.h"

#include "MainWindow.h"
#include "SceneWindow.h"
#include "InspectorWindow.h"
#include "HierarchyWindow.h"
#include "GameWindow.h"

#include "../Classes/Undo.h"
#include "../Classes/TreeView.h"
#include "../Serialization/EditorSettings.h"

#include <SDL2/SDL.h>

namespace GX
{
	int uiEditorIdHash = 0;

	glm::vec2 UIEditorWindow::transformVector(glm::vec2 vec, Canvas* canvas)
	{
		ImGuiWindow* win = ImGui::GetCurrentWindow();
		glm::vec2 offset = glm::vec2((win->Size.x / 2) - (canvas->getRefScreenWidth() * zoom) / 2, (win->Size.y / 2) - (canvas->getRefScreenHeight() * zoom) / 2);
		glm::vec2 pos = glm::vec2(win->Pos.x, win->Pos.y) + glm::vec2(1, 20);
		glm::vec2 ret = (camPos + pos + offset + vec * zoom);

		return ret;
	}

	glm::vec2 UIEditorWindow::transformVector(glm::vec2 vec, float width)
	{
		ImGuiWindow* win = ImGui::GetCurrentWindow();
		glm::vec2 offset = glm::vec2((win->Size.x / 2) - (width * zoom) / 2, (win->Size.y / 2) - (width * zoom) / 2);
		glm::vec2 pos = glm::vec2(win->Pos.x, win->Pos.y) + glm::vec2(1, 20);
		glm::vec2 ret = (camPos + pos + offset + vec * zoom);

		return ret;
	}

	ImVec2 imVec(glm::vec2 vec)
	{
		return ImVec2(vec.x, vec.y);
	}

	UIEditorWindow::UIEditorWindow()
	{
		uiEditorIdHash = ImHashStr("UI Editor");
	}

	UIEditorWindow::~UIEditorWindow()
	{
	}

	void UIEditorWindow::init()
	{
	}

	void UIEditorWindow::update()
	{
		if (opened)
		{
			ImGuiID tab_id = 0;

			if (ImGui::IsMouseReleased(ImGuiMouseButton_Middle))
				mmbPressed = false;
			if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
			{
				lmbPressed = false;
				checkDragging = true;
				isAnySelectedElementWasHovered = false;
			}

			EditorSettings* settings = MainWindow::getSettings();

			//Camera control
			ImGui::PushStyleColor(ImGuiCol_WindowBg, bgColor);
			ImGui::PushStyleColor(ImGuiCol_ChildBg, bgColor);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
			bool wmopen = ImGui::Begin("UI Editor", &opened, ImGuiWindowFlags_NoCollapse);
			ImGui::PopStyleVar();
			if (wmopen)
			{
				InputManager* inputMgr = InputManager::getSingleton();

				focused = ImGui::IsWindowFocused();
				hovered = ImGui::IsWindowHovered();

				auto _mp = inputMgr->getMouseRelativePosition();
				glm::vec2 mousePosition = glm::vec2(_mp.first, _mp.second);
				bool mousePressed = false;
				bool mouseReleased = false;
				bool mouseMoved = false;
				glm::vec2 mouseOffset = mousePosition - prevMousePosition;

				if (hovered)
				{
					if (ImGui::IsMouseDown(ImGuiMouseButton_Middle))
						mmbPressed = true;
					if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
						lmbPressed = true;

					mousePressed = ImGui::IsMouseDown(ImGuiMouseButton_Left);
					mouseReleased = ImGui::IsMouseReleased(ImGuiMouseButton_Left);

					if (checkDragging)
					{
						mouseMoved = (mouseOffset.x != 0.0f || mouseOffset.y != 0.0f) && lmbPressed;
						if (mouseMoved)
						{
							checkDragging = false;
							isDragging = true;
						}
					}
				}

				ImGuiWindow* win = ImGui::GetCurrentWindow();

				bool active = true;

				if (win->DockIsActive)
				{
					tab_id = win->DockNode->TabBar->SelectedTabId;

					if (tab_id != uiEditorIdHash)
					{
						active = false;
					}
				}

				if (!Engine::getSingleton()->getIsRuntimeMode())
				{
					if (active)
					{
						if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle) && mmbPressed)
						{
							ImVec2 dragDelta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Middle);
							camPos.x += dragDelta.x;
							camPos.y += dragDelta.y;
							ImGui::ResetMouseDragDelta(ImGuiMouseButton_Middle);
						}

						if (hovered)
							zoom += ImGui::GetIO().MouseWheel * 0.1f;

						if (zoom < 0.125f) zoom = 0.125f;
						if (zoom > 2.0f) zoom = 2.0f;
					}
				}

				ImDrawList* drawList = ImGui::GetWindowDrawList();

				if (!Engine::getSingleton()->getIsRuntimeMode())
				{
					//----------------Render grid begin-----------------//
					int cellSize = 32;
					int cellCount = std::round(128 / zoom / 2) * 2;
					float startX = -camPos.x / cellSize / zoom;
					float startY = -camPos.y / cellSize / zoom;

					ImColor lineColor = ImGui::ColorConvertFloat4ToU32(ImVec4(0.6f, 0.6f, 0.6f, 0.7f));
					ImColor anchorColor1 = ImGui::ColorConvertFloat4ToU32(ImVec4(0.63f, 0.89f, 1.0f, 1.0f));
					ImColor anchorColor2 = ImGui::ColorConvertFloat4ToU32(ImVec4(0.0f, 0.71f, 1.0f, 1.0f));

					for (int j = startX; j < startX + cellCount; ++j)
					{
						glm::vec2 p1 = glm::vec2((j * cellSize), (startY * cellSize));
						glm::vec2 p2 = glm::vec2((j * cellSize), (startY * cellSize + cellCount * cellSize));

						drawList->AddLine(imVec(transformVector(p1, cellCount * cellSize)), imVec(transformVector(p2, cellCount * cellSize)), lineColor);
					}

					for (int i = startY; i < startY + cellCount; ++i)
					{
						glm::vec2 p1 = glm::vec2((startX * cellSize), (i * cellSize));
						glm::vec2 p2 = glm::vec2((startX * cellSize + cellCount * cellSize), (i * cellSize));

						drawList->AddLine(imVec(transformVector(p1, cellCount * cellSize)), imVec(transformVector(p2, cellCount * cellSize)), lineColor);
					}
					//-----------------Render grid end------------------//

					//----------------Render UI begin-----------------//
					std::vector<Canvas*>& canvases = Renderer::getSingleton()->getCanvases();
					std::vector<GameObject*> objects = Engine::getSingleton()->getGameObjects();

					bool isAnyElementHovered = false;

					bool gameWndActive = MainWindow::getGameWindow()->isMousePressed();

					for (auto canv = canvases.begin(); canv != canvases.end(); ++canv)
					{
						Canvas* canvas = *canv;
						if (!canvas->getEnabled() || !canvas->getGameObject()->getActive())
							continue;

						glm::vec2 winoffset = glm::vec2((win->Size.x / 2) - (canvas->getRefScreenWidth() * zoom) / 2, (win->Size.y / 2) - (canvas->getRefScreenHeight() * zoom) / 2);
						glm::vec2 winpos = glm::vec2(win->Pos.x, win->Pos.y) + glm::vec2(1, 20);
						glm::vec2 offset = (camPos + winpos + winoffset);

						glm::vec2 cposMin = transformVector(glm::vec2(0, 0), canvas);
						glm::vec2 cposMax = transformVector(canvas->getRefScreenSize(), canvas);

						canvas->setWindow(win);
						canvas->setZoom(zoom);
						canvas->setOffset(offset);
						canvas->setRealScreenSize(canvas->getRefScreenSize(), win);

						std::vector<UIElement*>& uiElements = canvas->getUIElements();
						std::sort(uiElements.begin(), uiElements.end(), [=](UIElement* elem1, UIElement* elem2) -> bool
							{
								auto e1 = std::find(objects.begin(), objects.end(), elem1->getGameObject());
								auto e2 = std::find(objects.begin(), objects.end(), elem2->getGameObject());
								int eidx1 = std::distance(objects.begin(), e1) + (int)elem1->getTransform()->getPosition().z;
								int eidx2 = std::distance(objects.begin(), e2) + (int)elem2->getTransform()->getPosition().z;
								return eidx1 < eidx2;
							}
						);

						for (auto elem = uiElements.begin(); elem != uiElements.end(); ++elem)
						{
							UIElement* element = *elem;
							if (!element->getEnabled() || !element->getGameObject()->getActive())
								continue;

							if (element->getCanvas() == nullptr)
								continue;

							int pushCount = 0;
							Transform* par = element->getTransform()->getParent();
							while (par != nullptr)
							{
								GameObject* ob = par->getGameObject();
								Mask* mask = (Mask*)ob->getComponent(Mask::COMPONENT_TYPE);
								if (mask != nullptr)
								{
									UIElement::Properties& props = mask->getProperties();
									props.isEditor = true;

									++pushCount;
									glm::vec4 rect = mask->getRect();
									win->DrawList->PushClipRect(ImVec2(rect.x, rect.y), ImVec2(rect.z, rect.w), true);
								}

								par = par->getParent();
							}

							element->clearClipRects();
							UIElement::Properties& props = element->getProperties();
							props.isEditor = true;

							auto& clipRects = props.clipRects;
							auto& clipRects1 = win->DrawList->_ClipRectStack;
							for (auto& clip : clipRects1)
								clipRects.push_back(glm::vec4(clip.x, clip.y, clip.z, clip.w));

							element->onUpdate(Time::getDeltaTime());

							element->onRender(drawList);

							while (pushCount > 0)
							{
								--pushCount;
								win->DrawList->PopClipRect();
							}

							//Draw element rect
							glm::vec4 elemRect = element->getRect();

							if (props.isHovered)
							{
								isAnyElementHovered = true;

								if (mouseReleased && !isDragging)
									selectObject(element, inputMgr->getKey(SDL_SCANCODE_LCTRL));
							}

							bool isSelected = isElementSelected(element);

							if (hovered)
							{
								if (selectedElements.size() == 1)
								{
									if (isSelected)
									{
										float wd = 7.0f;
										bool isInRect = props.isHoveredGlobal;
										bool leftSide = (mousePosition.x > elemRect.x && mousePosition.x < elemRect.x + wd && isInRect);
										bool rightSide = (mousePosition.x > elemRect.z - wd && mousePosition.x < elemRect.z&& isInRect);
										bool topSide = (mousePosition.y > elemRect.y && mousePosition.y < elemRect.y + wd && isInRect);
										bool bottomSide = (mousePosition.y > elemRect.w - wd && mousePosition.y < elemRect.w&& isInRect);

										if (!leftSide && !rightSide) //Check for inversion in left and right sides
										{
											rightSide = (mousePosition.x > elemRect.z && mousePosition.x < elemRect.z + wd && isInRect);
											leftSide = (mousePosition.x > elemRect.x - wd && mousePosition.x < elemRect.x&& isInRect);
										}
										if (!topSide && !bottomSide) //Check for inversion in top and bottom sides
										{
											bottomSide = (mousePosition.y > elemRect.w && mousePosition.y < elemRect.w + wd && isInRect);
											topSide = (mousePosition.y > elemRect.y - wd && mousePosition.y < elemRect.y&& isInRect);
										}

										bool topLeftCorner = (leftSide && topSide);
										bool bottomLeftCorner = (leftSide && bottomSide);
										bool topRightCorner = (rightSide && topSide);
										bool bottomRightCorner = (rightSide && bottomSide);

										if (resizeDirection == ResizeDirection::TopLeft || resizeDirection == ResizeDirection::BottomRight)
											ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNWSE);
										else if (resizeDirection == ResizeDirection::TopRight || resizeDirection == ResizeDirection::BottomLeft)
											ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNESW);
										else if (resizeDirection == ResizeDirection::Left || resizeDirection == ResizeDirection::Right)
											ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
										else if (resizeDirection == ResizeDirection::Top || resizeDirection == ResizeDirection::Bottom)
											ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
										else
											ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);

										if (resizeDirection == ResizeDirection::None)
										{
											if (!isDragging)
											{
												if (topLeftCorner || bottomRightCorner)
												{
													ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNWSE);
													if (lmbPressed)
													{
														if (topLeftCorner) resizeDirection = ResizeDirection::TopLeft;
														if (bottomRightCorner) resizeDirection = ResizeDirection::BottomRight;
													}
												}
												else if (topRightCorner || bottomLeftCorner)
												{
													ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNESW);
													if (lmbPressed)
													{
														if (topRightCorner) resizeDirection = ResizeDirection::TopRight;
														if (bottomLeftCorner) resizeDirection = ResizeDirection::BottomLeft;
													}
												}
												else if (leftSide || rightSide)
												{
													ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
													if (lmbPressed)
													{
														if (leftSide) resizeDirection = ResizeDirection::Left;
														if (rightSide) resizeDirection = ResizeDirection::Right;
													}
												}
												else if (topSide || bottomSide)
												{
													ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
													if (lmbPressed)
													{
														if (topSide) resizeDirection = ResizeDirection::Top;
														if (bottomSide) resizeDirection = ResizeDirection::Bottom;
													}
												}
											}
										}
										else
										{
											if (mouseReleased)
											{
												ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
												resizeDirection = ResizeDirection::None;
											}
										}
									}
								}
							}
						}

						for (auto it = selectedElements.begin(); it != selectedElements.end(); ++it)
						{
							Transform* par = (*it)->getTransform()->getParent();
							while (par != nullptr)
							{
								auto& comps = par->getGameObject()->getComponents();
								for (auto& comp : comps)
								{
									if (comp->isUiComponent())
									{
										glm::vec4 elemRect = ((UIElement*)comp)->getRect();
										drawList->AddRect(ImVec2(elemRect.x, elemRect.y), ImVec2(elemRect.z, elemRect.w), 0xffffffff, 0.0f, ImDrawCornerFlags_All, 1.0f);
									}
								}

								par = par->getParent();
							}

							glm::vec4 elemRect = (*it)->getRect();
							drawList->AddRect(ImVec2(elemRect.x, elemRect.y), ImVec2(elemRect.z, elemRect.w), anchorColor2, 0.0f, ImDrawCornerFlags_All, 2.0f);

							glm::vec2 elemCenter = transformVector((*it)->getGameObject()->getTransform()->getPosition(), canvas);
							float zm = 0.8f;
							drawList->AddCircle(ImVec2(elemCenter.x, elemCenter.y), 8.0f * zm, anchorColor1, 0, 2.0f * zm);
							drawList->AddCircleFilled(ImVec2(elemCenter.x, elemCenter.y), 6.0f * zm, anchorColor2);
						}

						//Draw canvas rect
						drawList->AddRect(imVec(cposMin), imVec(cposMax), 0xffffffff, 0.0f);

						/*glm::vec2 wpos = glm::vec2(win->Pos.x, win->Pos.y) + glm::vec2(1, 20);
						auto mpos = InputManager::getSingleton()->getMouseRelativePosition();
						glm::vec2 mp = glm::vec2(mpos.first, mpos.second);
						drawList->AddText(ImVec2(wpos.x + 10, wpos.y + 10), 0xffffffff, ("X: " + std::to_string(mp.x) + ", Y: " + std::to_string(mp.y)).c_str());*/
					}
					//----------------Render UI end-----------------//

					bool isAnySelectedHovered = false;
					for (auto cc = selectedElements.begin(); cc != selectedElements.end(); ++cc)
					{
						UIElement::Properties& props = (*cc)->getProperties();

						if (props.isHoveredGlobal)
						{
							isAnySelectedHovered = true;
							break;
						}
					}

					if (/*!inputMgr->getKey(SDL_SCANCODE_LCTRL) && */((isAnySelectedHovered && mouseMoved) || isAnySelectedElementWasHovered) && resizeDirection == ResizeDirection::None)
					{
						if (lmbPressed)
						{
							if (selectedElements.size() > 0)
							{
								isAnySelectedElementWasHovered = true;

								if (!isDraggingElements)
								{
									isDraggingElements = true;

									for (auto it : selectedElements)
										roundedPositions[it] = it->getTransform()->getPosition();

									onStartMove();
								}

								for (auto it = selectedElements.begin(); it != selectedElements.end(); ++it)
								{
									UIElement* element = *it;
									Transform* trans = element->getGameObject()->getTransform();
									Transform* parent = trans->getParent();
									bool isParentSelected = false;

									while (parent != nullptr)
									{
										auto sl = std::find_if(selectedElements.begin(), selectedElements.end(), [=](UIElement* el) -> bool
											{
												return el->getGameObject() == parent->getGameObject();
											}
										);
										if (sl != selectedElements.end())
										{
											isParentSelected = true;
											break;
										}
										else
											parent = parent->getParent();
									}

									if (!isParentSelected)
									{
										if (mouseOffset.x != 0.0f || mouseOffset.y != 0.0f)
										{
											glm::vec3 newPos = trans->getPosition() + glm::vec3(mouseOffset / zoom, 0.0f);
											glm::vec3& rpos = roundedPositions[element];
											rpos += glm::vec3(mouseOffset / zoom, 0.0f);
											if ((inputMgr->getKey(SDL_SCANCODE_LCTRL) && !settings->snapToGrid) ||
												(!inputMgr->getKey(SDL_SCANCODE_LCTRL) && settings->snapToGrid))
											{
												float snapSize = std::max(1.0f, settings->snapToGridUIMoveSize);

												newPos = glm::vec3(glm::round(rpos.x / snapSize) * snapSize,
													glm::round(rpos.y / snapSize) * snapSize,
													glm::round(rpos.z / snapSize) * snapSize);
											}

											trans->setPosition(newPos);

											MainWindow::getInspectorWindow()->updateObjectEditorTransform();
										}
									}
								}
							}
						}
					}
					else
					{
						if (selectedElements.size() > 0)
						{
							UIElement* element = selectedElements[0];
							Transform* elTrans = element->getTransform();

							if (resizeDirection != ResizeDirection::None)
							{
								if (!isResizingElements)
								{
									isResizingElements = true;

									for (auto it : selectedElements)
									{
										roundedPositions[it] = it->getTransform()->getPosition();
										roundedSizes[it] = it->getSize();
									}

									onStartResize();
								}
							}

							float snapSize = std::max(1.0f, settings->snapToGridUIMoveSize);

							if (resizeDirection == ResizeDirection::TopLeft)
							{
								glm::vec2 offs = (mouseOffset / zoom);

								glm::vec3& rpos = roundedPositions[element];
								glm::vec2& rsz = roundedSizes[element];

								rpos += glm::vec3(offs, 0.0f) - glm::vec3(offs * element->getAnchor(), 0.0f);
								rsz -= offs;

								if ((inputMgr->getKey(SDL_SCANCODE_LCTRL) && !settings->snapToGrid) ||
									(!inputMgr->getKey(SDL_SCANCODE_LCTRL) && settings->snapToGrid))
								{
									glm::vec2 newSz = glm::vec2(glm::round(rsz.x / snapSize) * snapSize,
										glm::round(rsz.y / snapSize) * snapSize);

									glm::vec3 newPos = glm::vec3(glm::round(rpos.x / snapSize) * snapSize,
										glm::round(rpos.y / snapSize) * snapSize,
										glm::round(rpos.z / snapSize) * snapSize);

									element->setSize(newSz);
									elTrans->setPosition(newPos);
								}
								else
								{
									element->setSize(element->getSize() - offs);
									elTrans->setPosition(elTrans->getPosition() + glm::vec3(offs, 0.0f) - glm::vec3(offs * element->getAnchor(), 0.0f));
								}
							}
							if (resizeDirection == ResizeDirection::TopRight)
							{
								glm::vec2 offs = (glm::vec2(mouseOffset.x, -mouseOffset.y) / zoom);

								glm::vec3& rpos = roundedPositions[element];
								glm::vec2& rsz = roundedSizes[element];

								rpos += glm::vec3(glm::vec2(0.0f, -offs.y), 0.0f) - glm::vec3(glm::vec2(-offs.x, -offs.y) * element->getAnchor(), 0.0f);
								rsz += offs;

								if ((inputMgr->getKey(SDL_SCANCODE_LCTRL) && !settings->snapToGrid) ||
									(!inputMgr->getKey(SDL_SCANCODE_LCTRL) && settings->snapToGrid))
								{
									glm::vec2 newSz = glm::vec2(glm::round(rsz.x / snapSize) * snapSize,
										glm::round(rsz.y / snapSize) * snapSize);

									glm::vec3 newPos = glm::vec3(glm::round(rpos.x / snapSize) * snapSize,
										glm::round(rpos.y / snapSize) * snapSize,
										glm::round(rpos.z / snapSize) * snapSize);

									element->setSize(newSz);
									elTrans->setPosition(newPos);
								}
								else
								{
									element->setSize(element->getSize() + offs);
									elTrans->setPosition(elTrans->getPosition() + glm::vec3(glm::vec2(0.0f, -offs.y), 0.0f) - glm::vec3(glm::vec2(-offs.x, -offs.y) * element->getAnchor(), 0.0f));
								}
							}
							if (resizeDirection == ResizeDirection::BottomLeft)
							{
								glm::vec2 offs = (glm::vec2(mouseOffset.x, -mouseOffset.y) / zoom);

								glm::vec3& rpos = roundedPositions[element];
								glm::vec2& rsz = roundedSizes[element];

								rpos += glm::vec3(glm::vec2(offs.x, 0), 0.0f) - glm::vec3(glm::vec2(offs.x, offs.y) * element->getAnchor(), 0.0f);
								rsz -= offs;

								if ((inputMgr->getKey(SDL_SCANCODE_LCTRL) && !settings->snapToGrid) ||
									(!inputMgr->getKey(SDL_SCANCODE_LCTRL) && settings->snapToGrid))
								{
									glm::vec2 newSz = glm::vec2(glm::round(rsz.x / snapSize) * snapSize,
										glm::round(rsz.y / snapSize) * snapSize);

									glm::vec3 newPos = glm::vec3(glm::round(rpos.x / snapSize) * snapSize,
										glm::round(rpos.y / snapSize) * snapSize,
										glm::round(rpos.z / snapSize) * snapSize);

									element->setSize(newSz);
									elTrans->setPosition(newPos);
								}
								else
								{
									element->setSize(element->getSize() - offs);
									elTrans->setPosition(elTrans->getPosition() + glm::vec3(glm::vec2(offs.x, 0), 0.0f) - glm::vec3(glm::vec2(offs.x, offs.y) * element->getAnchor(), 0.0f));
								}
							}
							if (resizeDirection == ResizeDirection::BottomRight)
							{
								glm::vec2 offs = (mouseOffset / zoom);

								glm::vec3& rpos = roundedPositions[element];
								glm::vec2& rsz = roundedSizes[element];

								rpos += glm::vec3(offs * element->getAnchor(), 0.0f);
								rsz += offs;

								if ((inputMgr->getKey(SDL_SCANCODE_LCTRL) && !settings->snapToGrid) ||
									(!inputMgr->getKey(SDL_SCANCODE_LCTRL) && settings->snapToGrid))
								{
									glm::vec2 newSz = glm::vec2(glm::round(rsz.x / snapSize) * snapSize,
										glm::round(rsz.y / snapSize) * snapSize);

									glm::vec3 newPos = glm::vec3(glm::round(rpos.x / snapSize) * snapSize,
										glm::round(rpos.y / snapSize) * snapSize,
										glm::round(rpos.z / snapSize) * snapSize);

									element->setSize(newSz);
									elTrans->setPosition(newPos);
								}
								else
								{
									element->setSize(element->getSize() + offs);
									elTrans->setPosition(elTrans->getPosition() + glm::vec3(offs * element->getAnchor(), 0.0f));
								}
							}
							if (resizeDirection == ResizeDirection::Left)
							{
								glm::vec2 offs = (glm::vec2(mouseOffset.x, 0.0f) / zoom);

								glm::vec3& rpos = roundedPositions[element];
								glm::vec2& rsz = roundedSizes[element];

								rpos += glm::vec3(offs, 0.0f) - glm::vec3(offs * element->getAnchor(), 0.0f);
								rsz -= offs;

								if ((inputMgr->getKey(SDL_SCANCODE_LCTRL) && !settings->snapToGrid) ||
									(!inputMgr->getKey(SDL_SCANCODE_LCTRL) && settings->snapToGrid))
								{
									glm::vec2 newSz = glm::vec2(glm::round(rsz.x / snapSize) * snapSize,
										glm::round(rsz.y / snapSize) * snapSize);

									glm::vec3 newPos = glm::vec3(glm::round(rpos.x / snapSize) * snapSize,
										glm::round(rpos.y / snapSize) * snapSize,
										glm::round(rpos.z / snapSize) * snapSize);

									element->setSize(newSz);
									elTrans->setPosition(newPos);
								}
								else
								{
									element->setSize(element->getSize() - offs);
									elTrans->setPosition(elTrans->getPosition() + glm::vec3(offs, 0.0f) - glm::vec3(offs * element->getAnchor(), 0.0f));
								}
							}
							if (resizeDirection == ResizeDirection::Right)
							{
								glm::vec2 offs = (glm::vec2(mouseOffset.x, 0) / zoom);

								glm::vec3& rpos = roundedPositions[element];
								glm::vec2& rsz = roundedSizes[element];

								rpos += glm::vec3(offs * element->getAnchor(), 0.0f);
								rsz += offs;

								if ((inputMgr->getKey(SDL_SCANCODE_LCTRL) && !settings->snapToGrid) ||
									(!inputMgr->getKey(SDL_SCANCODE_LCTRL) && settings->snapToGrid))
								{
									glm::vec2 newSz = glm::vec2(glm::round(rsz.x / snapSize) * snapSize,
										glm::round(rsz.y / snapSize) * snapSize);

									glm::vec3 newPos = glm::vec3(glm::round(rpos.x / snapSize) * snapSize,
										glm::round(rpos.y / snapSize) * snapSize,
										glm::round(rpos.z / snapSize) * snapSize);

									element->setSize(newSz);
									elTrans->setPosition(newPos);
								}
								else
								{
									element->setSize(element->getSize() + offs);
									elTrans->setPosition(elTrans->getPosition() + glm::vec3(offs * element->getAnchor(), 0.0f));
								}
							}
							if (resizeDirection == ResizeDirection::Top)
							{
								glm::vec2 offs = (glm::vec2(0.0f, mouseOffset.y) / zoom);

								glm::vec3& rpos = roundedPositions[element];
								glm::vec2& rsz = roundedSizes[element];

								rpos += glm::vec3(offs, 0.0f) - glm::vec3(offs * element->getAnchor(), 0.0f);
								rsz -= offs;

								if ((inputMgr->getKey(SDL_SCANCODE_LCTRL) && !settings->snapToGrid) ||
									(!inputMgr->getKey(SDL_SCANCODE_LCTRL) && settings->snapToGrid))
								{
									glm::vec2 newSz = glm::vec2(glm::round(rsz.x / snapSize) * snapSize,
										glm::round(rsz.y / snapSize) * snapSize);

									glm::vec3 newPos = glm::vec3(glm::round(rpos.x / snapSize) * snapSize,
										glm::round(rpos.y / snapSize) * snapSize,
										glm::round(rpos.z / snapSize) * snapSize);

									element->setSize(newSz);
									elTrans->setPosition(newPos);
								}
								else
								{
									element->setSize(element->getSize() - offs);
									elTrans->setPosition(elTrans->getPosition() + glm::vec3(offs, 0.0f) - glm::vec3(offs * element->getAnchor(), 0.0f));
								}
							}
							if (resizeDirection == ResizeDirection::Bottom)
							{
								glm::vec2 offs = (glm::vec2(0.0f, -mouseOffset.y) / zoom);

								glm::vec3& rpos = roundedPositions[element];
								glm::vec2& rsz = roundedSizes[element];

								rpos += glm::vec3(-offs * element->getAnchor(), 0.0f);
								rsz -= offs;

								if ((inputMgr->getKey(SDL_SCANCODE_LCTRL) && !settings->snapToGrid) ||
									(!inputMgr->getKey(SDL_SCANCODE_LCTRL) && settings->snapToGrid))
								{
									glm::vec2 newSz = glm::vec2(glm::round(rsz.x / snapSize) * snapSize,
										glm::round(rsz.y / snapSize) * snapSize);

									glm::vec3 newPos = glm::vec3(glm::round(rpos.x / snapSize) * snapSize,
										glm::round(rpos.y / snapSize) * snapSize,
										glm::round(rpos.z / snapSize) * snapSize);

									element->setSize(newSz);
									elTrans->setPosition(newPos);
								}
								else
								{
									element->setSize(element->getSize() - offs);
									elTrans->setPosition(elTrans->getPosition() + glm::vec3(-offs * element->getAnchor(), 0.0f));
								}
							}
							if (resizeDirection != ResizeDirection::None)
							{
								if (mouseOffset.x != 0.0f || mouseOffset.y != 0.0f)
									MainWindow::getInspectorWindow()->updateObjectEditorTransform();
							}
						}
					}

					if (mouseReleased && !isDragging)
					{
						if (!inputMgr->getKey(SDL_SCANCODE_LCTRL))
						{
							if (!(isAnySelectedHovered || isAnySelectedElementWasHovered)/* && !isDragging*/)
							{
								clearSelection();
								Gizmo* gizmo = MainWindow::getSceneWindow()->getGizmo();
								gizmo->clearSelection();
								gizmo->callSelectCallback();
							}
						}

						isAnySelectedElementWasHovered = false;
					}

					prevMousePosition = mousePosition;
				}
				else
				{
					uint32_t colorRect = Color::packABGR(Color(0.15f, 0.15f, 0.15f, 0.9f));
					uint32_t colorText = Color::packABGR(Color(0.8f, 0.8f, 0.8f, 1.0f));

					glm::vec2 pos = glm::vec2(win->Pos.x + (win->Size.x / 2.0f), win->Pos.y + (win->Size.y / 2.0f));// transformVector(glm::vec2(0.0f, 0.0f), 100.0f);

					drawList->AddRectFilled(ImVec2(pos.x - 220.0f, pos.y - 50.0f), ImVec2(pos.x + 220.0f, pos.y + 50.0f), colorRect, 20.0f);
					drawList->AddText(MainWindow::getSingleton()->getFontBig(), 24.0f, ImVec2(pos.x - 188.0f, pos.y - 12.0f), colorText, "UI Editor is not available in runtime mode");
				}
			}
			ImGui::End();
			ImGui::PopStyleColor(2);

			if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
			{
				isDragging = false;

				if (isDraggingElements)
				{
					isDraggingElements = false;
					roundedPositions.clear();
					onEndMove();
				}

				if (isResizingElements)
				{
					isResizingElements = false;
					roundedPositions.clear();
					onEndResize();
				}
			}
		}
	}

	bool UIEditorWindow::isElementSelected(UIElement* value)
	{
		return std::find(selectedElements.begin(), selectedElements.end(), value) != selectedElements.end();
	}

	void UIEditorWindow::onStartMove()
	{
		//Undo
		moveUndo = Undo::addUndo("Move objects");
		moveUndo->vec3Data.resize(2);

		moveUndo->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec3Data[0])
			{
				UIElement* el = (UIElement*)d.first;
				Transform* trans = el->getTransform();
				trans->setLocalPosition(d.second);
			}
		};

		moveUndo->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec3Data[1])
			{
				UIElement* el = (UIElement*)d.first;
				Transform* trans = el->getTransform();
				trans->setLocalPosition(d.second);
			}
		};

		for (auto& sel : selectedElements)
		{
			Transform* trans = sel->getTransform();
			moveUndo->vec3Data[0][sel] = trans->getLocalPosition();
		}
		//
	}

	void UIEditorWindow::onEndMove()
	{
		if (moveUndo != nullptr)
		{
			for (auto& sel : selectedElements)
			{
				Transform* trans = sel->getTransform();
				moveUndo->vec3Data[1][sel] = trans->getLocalPosition();
			}

			moveUndo = nullptr;
		}
	}

	void UIEditorWindow::onStartResize()
	{
		//Undo
		sizeUndo = Undo::addUndo("Resize objects");
		sizeUndo->vec2Data.resize(2);
		sizeUndo->vec3Data.resize(2);

		sizeUndo->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec3Data[0])
			{
				UIElement* el = (UIElement*)d.first;
				Transform* trans = el->getTransform();
				trans->setLocalPosition(d.second);
				el->setSize(data->vec2Data[0][el]);
			}
		};

		sizeUndo->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec3Data[1])
			{
				UIElement* el = (UIElement*)d.first;
				Transform* trans = el->getTransform();
				trans->setLocalPosition(d.second);
				el->setSize(data->vec2Data[1][el]);
			}
		};

		for (auto& sel : selectedElements)
		{
			Transform* trans = sel->getTransform();
			sizeUndo->vec2Data[0][sel] = sel->getSize();
			sizeUndo->vec3Data[0][sel] = trans->getLocalPosition();
		}
		//
	}

	void UIEditorWindow::onEndResize()
	{
		if (sizeUndo != nullptr)
		{
			for (auto& sel : selectedElements)
			{
				Transform* trans = sel->getTransform();
				sizeUndo->vec2Data[1][sel] = sel->getSize();
				sizeUndo->vec3Data[1][sel] = trans->getLocalPosition();
			}

			sizeUndo = nullptr;
		}
	}

	void UIEditorWindow::selectObject(UIElement* value, bool multiple, bool callCallback)
	{
		Gizmo* gizmo = MainWindow::getSceneWindow()->getGizmo();

		if (multiple)
		{
			if (value != nullptr)
			{
				auto it = std::find(selectedElements.begin(), selectedElements.end(), value);

				if (it == selectedElements.end())
					selectedElements.push_back(value);
				else
					selectedElements.erase(it);
			}
		}
		else
		{
			selectedElements.clear();
			gizmo->clearSelection();
			selectedElements.push_back(value);
		}

		if (callCallback)
		{
			gizmo->selectObject(value->getGameObject()->getTransform(), multiple, true, gizmo);
		}
	}

	void UIEditorWindow::selectObjects(std::vector<UIElement*> value, bool callCallback)
	{
		selectedElements.clear();
		Gizmo* gizmo = MainWindow::getSceneWindow()->getGizmo();
		std::vector<Transform*> select;

		for (auto it = value.begin(); it != value.end(); ++it)
		{
			selectedElements.push_back(*it);
			select.push_back((*it)->getGameObject()->getTransform());
		}

		if (callCallback)
			gizmo->selectObjects(select, gizmo);

		select.clear();
	}

	void UIEditorWindow::clearSelection()
	{
		selectedElements.clear();
		MainWindow::getInspectorWindow()->setEditor(nullptr);
		MainWindow::getHierarchyWindow()->getTreeView()->selectNodes({ }, false);
	}
}