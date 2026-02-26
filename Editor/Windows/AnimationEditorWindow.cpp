#include "AnimationEditorWindow.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <dear-imgui/misc/cpp/imgui_stdlib.h>

#include "MainWindow.h"

#include "../PropertyEditors/ObjectEditor.h"

#include "../Classes/Toast.h"
#include "../Classes/Undo.h"

#include "../Engine/UI/ImGUIWidgets.h"
#include "../Engine/Classes/Helpers.h"
#include "../Engine/Classes/IO.h"
#include "../Engine/Core/InputManager.h"
#include "../Engine/Assets/AnimationClip.h"
#include "../Engine/Assets/Texture.h"
#include "../Engine/Math/Mathf.h"

#include <SDL2/SDL.h>

namespace GX
{
	AnimationEditorWindow::AnimationEditorWindow()
	{
	}

	AnimationEditorWindow::~AnimationEditorWindow()
	{
	}

	void AnimationEditorWindow::init()
	{
		addKeyframeTex = Texture::load(Helper::ExePath(), "Editor/Icons/Toolbar/add.png", false, Texture::CompressionMethod::None, true);
		pasteTex = Texture::load(Helper::ExePath(), "Editor/Icons/Other/clipboard.png", false, Texture::CompressionMethod::None, true);
		saveTex = Texture::load(Helper::ExePath(), "Editor/Icons/Toolbar/save.png", false, Texture::CompressionMethod::None, true);
	}

	void AnimationEditorWindow::update()
	{
		if (opened)
		{
			int maxW = MainWindow::getSingleton()->getWidth() * 0.9f;
			int maxH = MainWindow::getSingleton()->getHeight() * 0.9f;

			int minW = 220;
			int minH = 120;

			ImGui::SetNextWindowSize(ImVec2(640, 480), ImGuiCond_FirstUseEver);
			ImGui::SetNextWindowSizeConstraints(ImVec2(minW, minH), ImVec2(maxW, maxH));

			if (ImGui::Begin("Animation Editor", &opened, ImGuiWindowFlags_NoCollapse))
			{
				ImVec2 rmin = ImGui::GetWindowPos();
				ImVec2 rmax = ImGui::GetWindowSize();

				//focused = ImGui::IsMouseHoveringRect(ImGui::GetWindowPos(), ImVec2(rmin.x + rmax.x, rmin.y + rmax.y));

				focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);

				updateEditor();
			}
			else
				focused = false;

			ImGui::End();
		}
		else
			focused = false;
	}

	void AnimationEditorWindow::openAnimation(AnimationClip* anim)
	{
		//Reset
		openedAnimationNode = nullptr;
		currentFrame = 0;

		dragFramePos = nullptr;
		dragFrameRot = nullptr;
		dragFrameScl = nullptr;

		mouseDownTimeline = false;
		mouseDownPos = false;
		mouseDownRot = false;
		mouseDownScl = false;

		selFramePos = nullptr;
		selFrameRot = nullptr;
		selFrameScl = nullptr;

		//Open
		openedAnimation = anim;
		fileName = IO::GetFileNameWithExt(anim->getName());

		show(true);
	}

	void AnimationEditorWindow::closeAnimation()
	{
		openedAnimation = nullptr;
		openedAnimationNode = nullptr;
		currentFrame = 0;

		dragFramePos = nullptr;
		dragFrameRot = nullptr;
		dragFrameScl = nullptr;

		mouseDownTimeline = false;
		mouseDownPos = false;
		mouseDownRot = false;
		mouseDownScl = false;

		selFramePos = nullptr;
		selFrameRot = nullptr;
	}

	void AnimationEditorWindow::updateEditor()
	{
		if (openedAnimation == nullptr)
			return;

		//Save
		if (ImGui::ImageButtonWithText((void*)saveTex->getHandle().idx, "Save", ImVec2(12, 12), ImVec2(0, 1), ImVec2(1, 0)))
		{
			openedAnimation->save();
			Toast::showMessage("Animation \"" + fileName + "\" saved", ToastIcon::TI_SAVE);
		}

		ImGui::SameLine();

		//Name
		ImGui::Text(fileName.c_str());

		ImGui::Dummy(ImVec2(10, 5));
		ImGui::Separator();
		ImGui::Dummy(ImVec2(10, 5));

		//////

		std::vector<AnimationClipNode*>& nodes = openedAnimation->getAnimationClipNodes();

		if (ImGui::ImageButtonWithText((void*)addKeyframeTex->getHandle().idx, "Add node", ImVec2(12, 12), ImVec2(0, 1), ImVec2(1, 0)))
		{
			//Undo
			UndoData* undoData = Undo::addUndo("Add animation clip node");
			undoData->stringData.resize(2);

			undoData->undoAction = [=](UndoData* data)
			{
				AnimationClip* anim = AnimationClip::load(data->stringData[0][nullptr], data->stringData[1][nullptr]);
				if (anim != nullptr)
				{
					auto& _nodes = anim->getAnimationClipNodes();
					if (_nodes.size() > 0)
						anim->deleteAnimationClipNode(_nodes[_nodes.size() - 1]);
				}
			};

			undoData->redoAction = [=](UndoData* data)
			{
				AnimationClip* anim = AnimationClip::load(data->stringData[0][nullptr], data->stringData[1][nullptr]);
				if (anim != nullptr)
				{
					auto& _nodes = anim->getAnimationClipNodes();
					int num = _nodes.size();
					AnimationClipNode* node = new AnimationClipNode();
					node->setName("Node " + std::to_string(num));
					anim->addAnimationClipNode(node);
				}
			};

			undoData->stringData[0][nullptr] = openedAnimation->getLocation();
			undoData->stringData[1][nullptr] = openedAnimation->getName();
			//

			int num = nodes.size();

			AnimationClipNode* node = new AnimationClipNode();
			node->setName("Node " + std::to_string(num));

			openedAnimation->addAnimationClipNode(node);
		}

		//Frames count
		ImGui::SameLine();

		int frameCount = (int)openedAnimation->getDuration();
		ImGui::Text("Frames count");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(60);
		ImGui::InputInt("##anim_editor_frame_count", &frameCount, 0);
		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			if (frameCount >= 0)
			{
				//Undo
				UndoData* undoData = Undo::addUndo("Change animation clip duration");
				undoData->stringData.resize(2);
				undoData->intData.resize(2);

				undoData->undoAction = [=](UndoData* data)
				{
					AnimationClip* anim = AnimationClip::load(data->stringData[0][nullptr], data->stringData[1][nullptr]);
					if (anim != nullptr)
						anim->setDuration(data->intData[0][nullptr]);
				};

				undoData->redoAction = [=](UndoData* data)
				{
					AnimationClip* anim = AnimationClip::load(data->stringData[0][nullptr], data->stringData[1][nullptr]);
					if (anim != nullptr)
						anim->setDuration(data->intData[1][nullptr]);
				};

				undoData->stringData[0][nullptr] = openedAnimation->getLocation();
				undoData->stringData[1][nullptr] = openedAnimation->getName();
				undoData->intData[0][nullptr] = openedAnimation->getDuration();
				undoData->intData[1][nullptr] = frameCount;
				//

				openedAnimation->setDuration(frameCount);

				if (currentFrame > frameCount)
					currentFrame = frameCount;
			}
		}

		ImGui::SameLine();

		//Frames per second
		int fps = (int)openedAnimation->getFramesPerSecond();
		ImGui::Text("Frames per second");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(60);
		ImGui::InputInt("##anim_editor_frames_per_second", &fps, 0);
		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			if (fps > 0)
			{
				//Undo
				UndoData* undoData = Undo::addUndo("Change animation clip FPS");
				undoData->stringData.resize(2);
				undoData->intData.resize(2);

				undoData->undoAction = [=](UndoData* data)
				{
					AnimationClip* anim = AnimationClip::load(data->stringData[0][nullptr], data->stringData[1][nullptr]);
					if (anim != nullptr)
						anim->setFramesPerSecond(data->intData[0][nullptr]);
				};

				undoData->redoAction = [=](UndoData* data)
				{
					AnimationClip* anim = AnimationClip::load(data->stringData[0][nullptr], data->stringData[1][nullptr]);
					if (anim != nullptr)
						anim->setFramesPerSecond(data->intData[1][nullptr]);
				};

				undoData->stringData[0][nullptr] = openedAnimation->getLocation();
				undoData->stringData[1][nullptr] = openedAnimation->getName();
				undoData->intData[0][nullptr] = openedAnimation->getFramesPerSecond();
				undoData->intData[1][nullptr] = frameCount;
				//

				openedAnimation->setFramesPerSecond(fps);
			}
		}

		ImGui::Dummy(ImVec2(10, 5));

		//////

		ImU32 keyframeColor = ImGui::ColorConvertFloat4ToU32(ImVec4(1, 1, 1, 0.85f));

		ImGuiWindow* wnd = ImGui::GetCurrentWindow();

		if (wnd->Size.x != wndPrevSize)
		{
			wndPrevSize = wnd->Size.x;

			if (panel1Size == 0.0f)
				panel1Size = wnd->Size.x / 6.0f;

			panel2Size = wnd->Size.x - panel1Size - 23;
		}

		if (panel1Size > wnd->Size.x - 100)
			panel1Size = wnd->Size.x - 100;

		if (panel1Size < 100) panel1Size = 100;

		ImGui::Splitter(true, 4, &panel1Size, &panel2Size, 100.0f, 100.0f);
		
		if (ImGui::BeginChild("anim_editor_nodes_list", ImVec2(panel1Size, 0), true))
		{
			for (int i = 0; i < nodes.size(); ++i)
			{
				AnimationClipNode* node = nodes[i];

				std::string _txt = std::to_string(i) + ". " + node->getName();
				bool _sel = openedAnimationNode == node;

				if (renameNode != i)
				{
					if (ImGui::Selectable(_txt.c_str(), _sel))
					{
						//Undo
						auto& nodes = openedAnimation->getAnimationClipNodes();
						auto _it = std::find(nodes.begin(), nodes.end(), node);
						int nodeIdx = std::distance(nodes.begin(), _it);

						int prevNode = -1;
						if (openedAnimationNode != nullptr)
						{
							auto it = std::find(nodes.begin(), nodes.end(), openedAnimationNode);
							if (it != nodes.end())
								prevNode = std::distance(nodes.begin(), it);
						}

						if (prevNode != nodeIdx)
						{
							UndoData* undoData = Undo::addUndo("Select animation clip node");
							undoData->stringData.resize(2);
							undoData->intData.resize(3);

							undoData->undoAction = [=](UndoData* data)
							{
								AnimationClip* anim = AnimationClip::load(data->stringData[0][nullptr], data->stringData[1][nullptr]);
								if (anim != nullptr)
								{
									int idx = data->intData[0][nullptr];
									if (idx > -1)
									{
										AnimationClipNode* _node = anim->getAnimationClipNodes()[idx];
										if (_node != nullptr)
											openedAnimationNode = _node;
									}
									else
										openedAnimationNode = nullptr;

									dragFramePos = nullptr;
									dragFrameRot = nullptr;
									dragFrameScl = nullptr;

									selFramePos = nullptr;
									selFrameRot = nullptr;
									selFrameScl = nullptr;
								}
							};

							undoData->redoAction = [=](UndoData* data)
							{
								AnimationClip* anim = AnimationClip::load(data->stringData[0][nullptr], data->stringData[1][nullptr]);
								if (anim != nullptr)
								{
									int idx = data->intData[1][nullptr];
									if (idx > -1)
									{
										AnimationClipNode* _node = anim->getAnimationClipNodes()[idx];
										if (_node != nullptr)
											openedAnimationNode = _node;
									}
									else
										openedAnimationNode = nullptr;

									dragFramePos = nullptr;
									dragFrameRot = nullptr;
									dragFrameScl = nullptr;

									selFramePos = nullptr;
									selFrameRot = nullptr;
									selFrameScl = nullptr;
								}
							};

							undoData->stringData[0][nullptr] = openedAnimation->getLocation();
							undoData->stringData[1][nullptr] = openedAnimation->getName();
							undoData->intData[0][nullptr] = prevNode;
							undoData->intData[1][nullptr] = nodeIdx;
						}
						//

						openedAnimationNode = node;

						dragFramePos = nullptr;
						dragFrameRot = nullptr;
						dragFrameScl = nullptr;

						selFramePos = nullptr;
						selFrameRot = nullptr;
						selFrameScl = nullptr;
					}

					if (ImGui::IsItemHovered() &&
						ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
					{
						renameNode = i;
					}
				}
				else
				{
					std::string _val = node->getName();
					std::string _txt = std::to_string(i) + ".";

					ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
					ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(3, 1));

					ImGui::Text(_txt.c_str());
					ImGui::SameLine();

					ImGui::SetNextItemWidth(-1);
					ImGui::InputText("##anim_editor_nodes_list_rename_node", &_val);

					ImGui::PopStyleVar(2);

					if (ImGui::IsItemDeactivatedAfterEdit() ||
						(!ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) ||
						InputManager::getSingleton()->getKeyDown(SDL_SCANCODE_RETURN))
					{
						//Undo
						UndoData* undoData = Undo::addUndo("Change animation clip node name");
						undoData->stringData.resize(4);
						undoData->intData.resize(1);

						undoData->undoAction = [=](UndoData* data)
						{
							AnimationClip* anim = AnimationClip::load(data->stringData[0][nullptr], data->stringData[1][nullptr]);
							if (anim != nullptr)
							{
								int idx = data->intData[0][nullptr];
								AnimationClipNode* _node = anim->getAnimationClipNodes()[idx];
								if (_node != nullptr)
									_node->setName(data->stringData[2][nullptr]);
							}
						};

						undoData->redoAction = [=](UndoData* data)
						{
							AnimationClip* anim = AnimationClip::load(data->stringData[0][nullptr], data->stringData[1][nullptr]);
							if (anim != nullptr)
							{
								int idx = data->intData[0][nullptr];
								AnimationClipNode* _node = anim->getAnimationClipNodes()[idx];
								if (_node != nullptr)
									_node->setName(data->stringData[3][nullptr]);
							}
						};

						undoData->stringData[0][nullptr] = openedAnimation->getLocation();
						undoData->stringData[1][nullptr] = openedAnimation->getName();
						undoData->stringData[2][nullptr] = node->getName();
						undoData->stringData[3][nullptr] = _val;
						undoData->intData[0][nullptr] = i;
						//

						node->setName(_val);
						renameNode = -1;
					}

					if (InputManager::getSingleton()->getKeyDown(SDL_SCANCODE_ESCAPE))
					{
						renameNode = -1;
					}
				}

				std::string popupId = "##anim_editor_nodes_list_popup_" + std::to_string(i);
				if (ImGui::BeginPopupContextItem(popupId.c_str()))
				{
					if (ImGui::MenuItem("Rename"))
					{
						renameNode = i;
					}

					if (ImGui::MenuItem("Delete"))
					{
						deleteNode = i;
					}

					ImGui::EndPopup();
				}
			}
		}

		if (deleteNode > -1)
		{
			//Undo
			UndoData* undoData = Undo::addUndo("Remove animation clip node");
			undoData->stringData.resize(2);
			undoData->intData.resize(1);
			undoData->objectData.resize(1);

			undoData->undoAction = [=](UndoData* data)
			{
				AnimationClip* anim = AnimationClip::load(data->stringData[0][nullptr], data->stringData[1][nullptr]);
				if (anim != nullptr)
				{
					int idx = data->intData[0][nullptr];
					AnimationClipNode* _node = (AnimationClipNode*)data->objectData[0][0];
					auto& _nodes = anim->getAnimationClipNodes();
					_nodes.insert(_nodes.begin() + idx, _node);
				}
			};

			undoData->redoAction = [=](UndoData* data)
			{
				AnimationClip* anim = AnimationClip::load(data->stringData[0][nullptr], data->stringData[1][nullptr]);
				if (anim != nullptr)
				{
					int idx = data->intData[0][nullptr];
					auto& _nodes = anim->getAnimationClipNodes();
					AnimationClipNode* _node = _nodes[idx];
					if (openedAnimationNode == _node)
						openedAnimationNode = nullptr;

					_nodes.erase(_nodes.begin() + idx);
				}
			};

			AnimationClipNode* node = nodes[deleteNode];

			undoData->stringData[0][nullptr] = openedAnimation->getLocation();
			undoData->stringData[1][nullptr] = openedAnimation->getName();
			undoData->intData[0][nullptr] = deleteNode;
			undoData->objectData[0].push_back(node);
			//

			if (openedAnimationNode == node)
				openedAnimationNode = nullptr;

			nodes.erase(nodes.begin() + deleteNode);
			deleteNode = -1;
		}

		ImGui::EndChild();

		ImGui::SameLine();

		if (ImGui::BeginChild("anim_editor_keyframe_editor", ImVec2(panel2Size, 0), true))
		{
			if (openedAnimationNode != nullptr)
			{
				float segW = 8.0f;

				float toolsW = 180.0f;

				int minNumKeys = (ImGui::GetContentRegionAvail().x - toolsW) / segW;
				int numKeys = (int)openedAnimation->getDuration();

				numKeys = std::max(minNumKeys, numKeys);

				float timelineH = 26.0f;
				float timelineW = numKeys * segW;
				timelineW = std::max(1.0f, timelineW);

				ImVec2 frameLinePos = ImVec2(0, 0);

				ImGuiStyle style = ImGui::GetStyle();

				if (InputManager::getSingleton()->getKeyDown(SDL_SCANCODE_DELETE) && !ImGui::GetIO().WantTextInput)
				{
					if (selFramePos != nullptr)
					{
						//Undo
						UndoData* undoData = Undo::addUndo("Delete animation clip position key");
						undoData->stringData.resize(2);
						undoData->intData.resize(3);
						undoData->vec3Data.resize(1);

						undoData->undoAction = [=](UndoData* data)
						{
							AnimationClip* anim = AnimationClip::load(data->stringData[0][nullptr], data->stringData[1][nullptr]);
							if (anim != nullptr)
							{
								int idxNode = data->intData[0][nullptr];
								int idxKey = data->intData[1][nullptr];
								AnimationClipNode* _node = anim->getAnimationClipNodes()[idxNode];
								if (_node != nullptr)
								{
									auto& _posKeys = _node->getPositionKeys();
									_posKeys.insert(_posKeys.begin() + idxKey, TimeVector3(data->intData[2][nullptr], data->vec3Data[0][nullptr]));
								}
							}
						};

						undoData->redoAction = [=](UndoData* data)
						{
							AnimationClip* anim = AnimationClip::load(data->stringData[0][nullptr], data->stringData[1][nullptr]);
							if (anim != nullptr)
							{
								int idx = data->intData[0][nullptr];
								AnimationClipNode* _node = anim->getAnimationClipNodes()[idx];
								if (_node != nullptr)
								{
									auto& _posKeys = _node->getPositionKeys();
									auto it = std::find_if(_posKeys.begin(), _posKeys.end(), [=](TimeVector3& v) -> bool { return v.time == data->intData[2][nullptr]; });
									if (it != _posKeys.end())
										_posKeys.erase(it);

									selFramePos = nullptr;
								}
							}
						};

						auto& keys = openedAnimationNode->getPositionKeys();
						auto it = std::find_if(keys.begin(), keys.end(), [=](TimeVector3& v) -> bool { return v.time == selFramePos->time; });

						auto _it = std::find(nodes.begin(), nodes.end(), openedAnimationNode);
						int nodeIdx = std::distance(nodes.begin(), _it);
						int keyIdx = std::distance(keys.begin(), it);

						undoData->stringData[0][nullptr] = openedAnimation->getLocation();
						undoData->stringData[1][nullptr] = openedAnimation->getName();
						undoData->intData[0][nullptr] = nodeIdx;
						undoData->intData[1][nullptr] = keyIdx;
						undoData->intData[2][nullptr] = selFramePos->time;
						undoData->vec3Data[0][nullptr] = selFramePos->value;
						//

						if (it != keys.end())
							keys.erase(it);

						selFramePos = nullptr;
					}

					if (selFrameRot != nullptr)
					{
						//Undo
						UndoData* undoData = Undo::addUndo("Delete animation clip rotation key");
						undoData->stringData.resize(2);
						undoData->intData.resize(3);
						undoData->quatData.resize(1);

						undoData->undoAction = [=](UndoData* data)
						{
							AnimationClip* anim = AnimationClip::load(data->stringData[0][nullptr], data->stringData[1][nullptr]);
							if (anim != nullptr)
							{
								int idxNode = data->intData[0][nullptr];
								int idxKey = data->intData[1][nullptr];
								AnimationClipNode* _node = anim->getAnimationClipNodes()[idxNode];
								if (_node != nullptr)
								{
									auto& _rotKeys = _node->getRotationKeys();
									_rotKeys.insert(_rotKeys.begin() + idxKey, TimeQuaternion(data->intData[2][nullptr], data->quatData[0][nullptr]));
								}
							}
						};

						undoData->redoAction = [=](UndoData* data)
						{
							AnimationClip* anim = AnimationClip::load(data->stringData[0][nullptr], data->stringData[1][nullptr]);
							if (anim != nullptr)
							{
								int idx = data->intData[0][nullptr];
								AnimationClipNode* _node = anim->getAnimationClipNodes()[idx];
								if (_node != nullptr)
								{
									auto& _rotKeys = _node->getRotationKeys();
									auto it = std::find_if(_rotKeys.begin(), _rotKeys.end(), [=](TimeQuaternion& v) -> bool { return v.time == data->intData[2][nullptr]; });
									if (it != _rotKeys.end())
										_rotKeys.erase(it);

									selFrameRot = nullptr;
								}
							}
						};

						auto& keys = openedAnimationNode->getRotationKeys();
						auto it = std::find_if(keys.begin(), keys.end(), [=](TimeQuaternion& v) -> bool { return v.time == selFrameRot->time; });

						auto _it = std::find(nodes.begin(), nodes.end(), openedAnimationNode);
						int nodeIdx = std::distance(nodes.begin(), _it);
						int keyIdx = std::distance(keys.begin(), it);

						undoData->stringData[0][nullptr] = openedAnimation->getLocation();
						undoData->stringData[1][nullptr] = openedAnimation->getName();
						undoData->intData[0][nullptr] = nodeIdx;
						undoData->intData[1][nullptr] = keyIdx;
						undoData->intData[2][nullptr] = selFrameRot->time;
						undoData->quatData[0][nullptr] = selFrameRot->value;
						//

						if (it != keys.end())
							keys.erase(it);

						selFrameRot = nullptr;
					}

					if (selFrameScl != nullptr)
					{
						//Undo
						UndoData* undoData = Undo::addUndo("Delete animation clip scale key");
						undoData->stringData.resize(2);
						undoData->intData.resize(3);
						undoData->vec3Data.resize(1);

						undoData->undoAction = [=](UndoData* data)
						{
							AnimationClip* anim = AnimationClip::load(data->stringData[0][nullptr], data->stringData[1][nullptr]);
							if (anim != nullptr)
							{
								int idxNode = data->intData[0][nullptr];
								int idxKey = data->intData[1][nullptr];
								AnimationClipNode* _node = anim->getAnimationClipNodes()[idxNode];
								if (_node != nullptr)
								{
									auto& _sclKeys = _node->getScalingKeys();
									_sclKeys.insert(_sclKeys.begin() + idxKey, TimeVector3(data->intData[2][nullptr], data->vec3Data[0][nullptr]));
								}
							}
						};

						undoData->redoAction = [=](UndoData* data)
						{
							AnimationClip* anim = AnimationClip::load(data->stringData[0][nullptr], data->stringData[1][nullptr]);
							if (anim != nullptr)
							{
								int idx = data->intData[0][nullptr];
								AnimationClipNode* _node = anim->getAnimationClipNodes()[idx];
								if (_node != nullptr)
								{
									auto& _sclKeys = _node->getScalingKeys();
									auto it = std::find_if(_sclKeys.begin(), _sclKeys.end(), [=](TimeVector3& v) -> bool { return v.time == data->intData[2][nullptr]; });
									if (it != _sclKeys.end())
										_sclKeys.erase(it);

									selFrameScl = nullptr;
								}
							}
						};

						auto& keys = openedAnimationNode->getScalingKeys();
						auto it = std::find_if(keys.begin(), keys.end(), [=](TimeVector3& v) -> bool { return v.time == selFrameScl->time; });

						auto _it = std::find(nodes.begin(), nodes.end(), openedAnimationNode);
						int nodeIdx = std::distance(nodes.begin(), _it);
						int keyIdx = std::distance(keys.begin(), it);

						undoData->stringData[0][nullptr] = openedAnimation->getLocation();
						undoData->stringData[1][nullptr] = openedAnimation->getName();
						undoData->intData[0][nullptr] = nodeIdx;
						undoData->intData[1][nullptr] = keyIdx;
						undoData->intData[2][nullptr] = selFrameScl->time;
						undoData->vec3Data[0][nullptr] = selFrameScl->value;
						//

						if (it != keys.end())
							keys.erase(it);

						selFrameScl = nullptr;
					}
				}

				ImGui::BeginColumns("anim_editor_timeline", 3, ImGuiOldColumnFlags_NoBorder | ImGuiOldColumnFlags_NoResize);
				ImGui::SetColumnWidth(0, toolsW * 0.85f);
				ImGui::SetColumnWidth(1, toolsW * 0.15f);
				ImGui::NextColumn();
				ImGui::NextColumn();
				{
					//Timeline
					bool _wnd = ImGui::BeginChild("anim_editor_timeline_canvas", ImVec2(timelineW, timelineH), false);

					ImGuiWindow* childWnd = ImGui::GetCurrentWindow();
					frameLinePos = childWnd->DC.CursorPos;
					frameLinePos.y -= style.WindowBorderSize;

					if (_wnd)
					{
						ImDrawList* drawList = childWnd->DrawList;

						//Current frame line drag
						if (focused)
						{
							if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
							{
								ImRect rect = childWnd->Rect();
								if (ImGui::IsMouseHoveringRect(rect.Min, rect.Max))
								{
									if (!mouseDownTimeline)
										mouseDownTimeline = true;
								}
							}

							if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
							{
								if (mouseDownTimeline)
								{
									ImRect rect = childWnd->Rect();
									if (ImGui::IsMouseHoveringRect(rect.Min, rect.Max))
									{
										if (timelineW > 0.0f)
											currentFrame = (ImGui::GetMousePos().x + timelineScroll - ImGui::GetCursorScreenPos().x) / segW;
									}
								}
							}
						}
						//

						//Lines
						drawList->PushClipRect(
							ImVec2(childWnd->DC.CursorPos.x - style.WindowPadding.x - style.WindowBorderSize,
								childWnd->DC.CursorPos.y - style.WindowPadding.y),
							ImVec2(childWnd->DC.CursorPos.x - style.WindowPadding.x + numKeys * segW,
								childWnd->DC.CursorPos.y + timelineH), true);

						ImVec2 pos;
						pos.x = childWnd->DC.CursorPos.x + style.WindowBorderSize;
						pos.y = childWnd->DC.CursorPos.y - style.WindowBorderSize;

						for (int i = 0; i < numKeys + 1; ++i)
						{
							float h = 10.0f;

							ImU32 color = ImGui::ColorConvertFloat4ToU32(ImVec4(1, 1, 1, 0.5f));

							if ((i + 1) % 2 != 0.0f)
							{
								
								color = ImGui::ColorConvertFloat4ToU32(ImVec4(1, 1, 1, 0.25f));

								h = timelineH * 0.5f + 3.0f;
							}

							drawList->AddLine(ImVec2(pos.x - timelineScroll + i * segW, pos.y + h), ImVec2(pos.x - timelineScroll + i * segW, pos.y + timelineH), color, 1.0f);

							if ((i + 1) % 10 == 0 || i == 0)
							{
								if (i == 0)
									color = ImGui::ColorConvertFloat4ToU32(ImVec4(1, 1, 1, 0.5f));

								std::string txt = std::to_string(i + 1);
								drawList->AddText(ImVec2(pos.x - timelineScroll + i * segW + 2.0f, pos.y - 2.0f), color, txt.c_str());
							}
						}

						drawList->PopClipRect();
					}

					ImGui::EndChild();
				}
				ImGui::EndColumns();

				//---

				ImGui::BeginColumns("anim_editor_transform_1", 3, ImGuiOldColumnFlags_NoBorder | ImGuiOldColumnFlags_NoResize);
				ImGui::SetColumnWidth(0, toolsW * 0.85f);
				ImGui::SetColumnWidth(1, toolsW * 0.15f);
				{
					ImGui::Text("Position");
				}
				ImGui::NextColumn();
				{
					ImGui::PushID("##anim_editor_transform_1_btn_add");
					bool btn = ImGui::ImageButton((void*)addKeyframeTex->getHandle().idx, ImVec2(10, 10), ImVec2(0, 1), ImVec2(1, 0));
					ImGui::PopID();

					if (btn)
					{
						//Undo
						UndoData* undoData = Undo::addUndo("Add animation clip position key");
						undoData->stringData.resize(2);
						undoData->intData.resize(2);

						undoData->undoAction = [=](UndoData* data)
						{
							AnimationClip* anim = AnimationClip::load(data->stringData[0][nullptr], data->stringData[1][nullptr]);
							if (anim != nullptr)
							{
								int idx = data->intData[0][nullptr];
								AnimationClipNode* _node = anim->getAnimationClipNodes()[idx];
								if (_node != nullptr)
								{
									auto& _keys = _node->getPositionKeys();
									_keys.erase(_keys.begin() + (_keys.size() - 1));
								}
							}
						};

						undoData->redoAction = [=](UndoData* data)
						{
							AnimationClip* anim = AnimationClip::load(data->stringData[0][nullptr], data->stringData[1][nullptr]);
							if (anim != nullptr)
							{
								int idx = data->intData[0][nullptr];
								AnimationClipNode* _node = anim->getAnimationClipNodes()[idx];
								if (_node != nullptr)
								{
									auto& _keys = _node->getPositionKeys();
									_keys.push_back(TimeVector3(data->intData[1][nullptr], glm::vec3(0.0f)));

									selFramePos = nullptr;
									selFrameRot = nullptr;
									selFrameScl = nullptr;
								}
							}
						};

						auto _it = std::find(nodes.begin(), nodes.end(), openedAnimationNode);
						int nodeIdx = std::distance(nodes.begin(), _it);

						undoData->stringData[0][nullptr] = openedAnimation->getLocation();
						undoData->stringData[1][nullptr] = openedAnimation->getName();
						undoData->intData[0][nullptr] = nodeIdx;
						undoData->intData[1][nullptr] = currentFrame;
						//
						
						//Add position key frame
						auto& keys = openedAnimationNode->getPositionKeys();

						auto it = std::find_if(keys.begin(), keys.end(), [=](TimeVector3& v) -> bool { return v.time == currentFrame; });

						if (it == keys.end())
							keys.push_back(TimeVector3(currentFrame, glm::vec3(0.0f)));

						if (openedAnimation->getDuration() < currentFrame + 1)
							openedAnimation->setDuration(currentFrame + 1);

						selFramePos = nullptr;
						selFrameRot = nullptr;
						selFrameScl = nullptr;
					}

					MainWindow::HelpMarker("Add position key");
				}
				ImGui::NextColumn();
				{
					float sY = ImGui::GetScrollY();

					ImGui::PushClipRect(ImVec2(frameLinePos.x, frameLinePos.y + sY), ImVec2(frameLinePos.x + ImGui::GetContentRegionAvail().x + style.WindowPadding.x,
						frameLinePos.y + sY + ImGui::GetCursorScreenPos().y - style.WindowBorderSize * 3.0f), true);

					//Keyframes
					float x = ImGui::GetCursorPosX();
					float x1 = x;
					ImGui::SetCursorPosX(x - timelineScroll);

					ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, style.WindowPadding.y));
					bool _c = ImGui::BeginChild("anim_editor_curves_position", ImVec2(timelineW, 16), true);

					if (_c)
					{
						ImGuiWindow* childWnd = ImGui::GetCurrentWindow();
						ImDrawList* drawList = childWnd->DrawList;

						auto& keys = openedAnimationNode->getPositionKeys();

						ImU32 tintColor = keyframeColor;

						//Keyframe drag
						if (focused)
						{
							if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
							{
								ImRect rect = childWnd->Rect();
								if (ImGui::IsMouseHoveringRect(rect.Min, rect.Max))
								{
									if (!mouseDownPos)
									{
										mouseDownPos = true;

										int frm = (ImGui::GetMousePos().x - ImGui::GetCursorScreenPos().x + (segW * 0.5f)) / segW;
										auto it = std::find_if(keys.begin(), keys.end(), [=](TimeVector3& v) -> bool { return v.time == frm; });

										if (it != keys.end())
										{
											dragFramePos = &*it;
											selFramePos = &*it;
											selFramePosVal = it->value;
											selFrameRot = nullptr;
											selFrameScl = nullptr;

											dragStartTime = frm;
										}
										else
										{
											selFramePos = nullptr;
											selFrameRot = nullptr;
											selFrameScl = nullptr;

											dragStartTime = -1;
										}
									}
								}
							}

							if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
							{
								if (mouseDownPos)
								{
									//ImRect rect = childWnd->Rect();
									//if (ImGui::IsMouseHoveringRect(rect.Min, rect.Max))
									//{
									if (timelineW > 0.0f)
									{
										if (dragFramePos != nullptr)
										{
											int _t = (int)((ImGui::GetMousePos().x - ImGui::GetCursorScreenPos().x + (segW * 0.5f)) / segW);
											auto it = std::find_if(keys.begin(), keys.end(), [=](TimeVector3& v) -> bool { return v.time == _t; });
											if (it == keys.end())
												dragFramePos->time = _t;
										}
									}
									//}

									if (dragFramePos != nullptr)
									{
										tintColor = ImGui::ColorConvertFloat4ToU32(ImVec4(0.2f, 0.5f, 1.0f, 0.85f));
									}
								}
							}

							if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
							{
								//ImRect rect = childWnd->Rect();
								//if (ImGui::IsMouseHoveringRect(rect.Min, rect.Max) && selFramePos != nullptr)
								//{
								//int _t = (int)((ImGui::GetMousePos().x - ImGui::GetCursorScreenPos().x + (segW * 0.5f)) / segW);

								if (dragFramePos != nullptr && dragStartTime != -1 && dragStartTime != (int)dragFramePos->time)
								{
									//Undo
									UndoData* undoData = Undo::addUndo("Change animation clip key time");
									undoData->stringData.resize(2);
									undoData->intData.resize(4);

									undoData->undoAction = [=](UndoData* data)
									{
										AnimationClip* anim = AnimationClip::load(data->stringData[0][nullptr], data->stringData[1][nullptr]);
										if (anim != nullptr)
										{
											int idx = data->intData[0][nullptr];
											AnimationClipNode* _node = anim->getAnimationClipNodes()[idx];
											if (_node != nullptr)
											{
												auto& _keys = _node->getPositionKeys();
												TimeVector3& _key = _keys[data->intData[1][nullptr]];
												_key.time = data->intData[2][nullptr];
											}
										}
									};

									undoData->redoAction = [=](UndoData* data)
									{
										AnimationClip* anim = AnimationClip::load(data->stringData[0][nullptr], data->stringData[1][nullptr]);
										if (anim != nullptr)
										{
											int idx = data->intData[0][nullptr];
											AnimationClipNode* _node = anim->getAnimationClipNodes()[idx];
											if (_node != nullptr)
											{
												auto& _keys = _node->getPositionKeys();
												TimeVector3& _key = _keys[data->intData[1][nullptr]];
												_key.time = data->intData[3][nullptr];
											}
										}
									};

									auto _it = std::find(nodes.begin(), nodes.end(), openedAnimationNode);
									auto it = std::find_if(keys.begin(), keys.end(), [=](TimeVector3& v) -> bool { return v.time == (int)dragFramePos->time; });
									int nodeIdx = std::distance(nodes.begin(), _it);
									int keyIdx = std::distance(keys.begin(), it);

									undoData->stringData[0][nullptr] = openedAnimation->getLocation();
									undoData->stringData[1][nullptr] = openedAnimation->getName();
									undoData->intData[0][nullptr] = nodeIdx;
									undoData->intData[1][nullptr] = keyIdx;
									undoData->intData[2][nullptr] = dragStartTime;
									undoData->intData[3][nullptr] = (int)dragFramePos->time;
									//
								}
								//}
							}
						}
						//

						ImVec2 pos = childWnd->DC.CursorPos;

						for (int i = 0; i < numKeys; ++i)
						{
							for (int j = 0; j < keys.size(); ++j)
							{
								if ((int)keys[j].time == i)
								{
									tintColor = keyframeColor;

									if (selFramePos != nullptr && selFramePos->time == i)
										tintColor = ImGui::ColorConvertFloat4ToU32(ImVec4(0.2f, 0.5f, 1.0f, 0.85f));

									drawList->AddCircleFilled(ImVec2(pos.x + i * segW, pos.y), 3.0f, tintColor);

									break;
								}
							}
						}
					}

					ImGui::EndChild();
					ImGui::PopStyleVar();

					ImGui::SetCursorPosX(x1);

					ImGui::PopClipRect();
				}
				ImGui::EndColumns();

				//---

				ImGui::BeginColumns("anim_editor_transform_2", 3, ImGuiOldColumnFlags_NoBorder | ImGuiOldColumnFlags_NoResize);
				ImGui::SetColumnWidth(0, toolsW * 0.85f);
				ImGui::SetColumnWidth(1, toolsW * 0.15f);
				{
					ImGui::Text("Rotation");
				}
				ImGui::NextColumn();
				{
					ImGui::PushID("##anim_editor_transform_2_btn_add");
					bool btn = ImGui::ImageButton((void*)addKeyframeTex->getHandle().idx, ImVec2(10, 10), ImVec2(0, 1), ImVec2(1, 0));
					ImGui::PopID();

					if (btn)
					{
						//Undo
						UndoData* undoData = Undo::addUndo("Add animation clip rotation key");
						undoData->stringData.resize(2);
						undoData->intData.resize(2);

						undoData->undoAction = [=](UndoData* data)
						{
							AnimationClip* anim = AnimationClip::load(data->stringData[0][nullptr], data->stringData[1][nullptr]);
							if (anim != nullptr)
							{
								int idx = data->intData[0][nullptr];
								AnimationClipNode* _node = anim->getAnimationClipNodes()[idx];
								if (_node != nullptr)
								{
									auto& _keys = _node->getRotationKeys();
									_keys.erase(_keys.begin() + (_keys.size() - 1));
								}
							}
						};

						undoData->redoAction = [=](UndoData* data)
						{
							AnimationClip* anim = AnimationClip::load(data->stringData[0][nullptr], data->stringData[1][nullptr]);
							if (anim != nullptr)
							{
								int idx = data->intData[0][nullptr];
								AnimationClipNode* _node = anim->getAnimationClipNodes()[idx];
								if (_node != nullptr)
								{
									auto& _keys = _node->getRotationKeys();
									_keys.push_back(TimeQuaternion(data->intData[1][nullptr], glm::identity<glm::quat>()));

									selFramePos = nullptr;
									selFrameRot = nullptr;
									selFrameScl = nullptr;
								}
							}
						};

						auto _it = std::find(nodes.begin(), nodes.end(), openedAnimationNode);
						int nodeIdx = std::distance(nodes.begin(), _it);

						undoData->stringData[0][nullptr] = openedAnimation->getLocation();
						undoData->stringData[1][nullptr] = openedAnimation->getName();
						undoData->intData[0][nullptr] = nodeIdx;
						undoData->intData[1][nullptr] = currentFrame;
						//
						
						//Add rotation key frame
						auto& keys = openedAnimationNode->getRotationKeys();

						auto it = std::find_if(keys.begin(), keys.end(), [=](TimeQuaternion& v) -> bool { return v.time == currentFrame; });

						if (it == keys.end())
							keys.push_back(TimeQuaternion(currentFrame, glm::identity<glm::quat>()));

						if (openedAnimation->getDuration() < currentFrame + 1)
							openedAnimation->setDuration(currentFrame + 1);

						selFramePos = nullptr;
						selFrameRot = nullptr;
						selFrameScl = nullptr;
					}

					MainWindow::HelpMarker("Add rotation key");
				}
				ImGui::NextColumn();
				{
					float sY = ImGui::GetScrollY();

					ImGui::PushClipRect(ImVec2(frameLinePos.x, frameLinePos.y + sY), ImVec2(frameLinePos.x + ImGui::GetContentRegionAvail().x + style.WindowPadding.x,
						frameLinePos.y + sY + ImGui::GetCursorScreenPos().y - style.WindowBorderSize * 3.0f), true);

					//Keyframes
					float x = ImGui::GetCursorPosX();
					float x1 = x;
					ImGui::SetCursorPosX(x - timelineScroll);

					ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, style.WindowPadding.y));
					bool _c = ImGui::BeginChild("anim_editor_curves_rotation", ImVec2(timelineW, 16), true);

					if (_c)
					{
						ImGuiWindow* childWnd = ImGui::GetCurrentWindow();
						ImDrawList* drawList = childWnd->DrawList;

						auto& keys = openedAnimationNode->getRotationKeys();

						ImU32 tintColor = keyframeColor;

						//Keyframe drag
						if (focused)
						{
							if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
							{
								ImRect rect = childWnd->Rect();
								if (ImGui::IsMouseHoveringRect(rect.Min, rect.Max))
								{
									if (!mouseDownRot)
									{
										mouseDownRot = true;

										int frm = (ImGui::GetMousePos().x - ImGui::GetCursorScreenPos().x + (segW * 0.5f)) / segW;
										auto it = std::find_if(keys.begin(), keys.end(), [=](TimeQuaternion& v) -> bool { return v.time == frm; });

										if (it != keys.end())
										{
											dragFrameRot = &*it;
											selFrameRot = &*it;
											selFrameRotVal = Mathf::toEuler(it->value);
											selFramePos = nullptr;
											selFrameScl = nullptr;

											dragStartTime = frm;
										}
										else
										{
											selFrameRot = nullptr;
											selFramePos = nullptr;
											selFrameScl = nullptr;

											dragStartTime = -1;
										}
									}
								}
							}

							if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
							{
								if (mouseDownRot)
								{
									//ImRect rect = childWnd->Rect();
									//if (ImGui::IsMouseHoveringRect(rect.Min, rect.Max))
									//{
									if (timelineW > 0.0f)
									{
										if (dragFrameRot != nullptr)
										{
											int _t = (int)((ImGui::GetMousePos().x - ImGui::GetCursorScreenPos().x + (segW * 0.5f)) / segW);
											auto it = std::find_if(keys.begin(), keys.end(), [=](TimeQuaternion& v) -> bool { return v.time == _t; });
											if (it == keys.end())
												dragFrameRot->time = _t;
										}
									}
									//}

									if (dragFrameRot != nullptr)
									{
										tintColor = ImGui::ColorConvertFloat4ToU32(ImVec4(0.2f, 0.5f, 1.0f, 0.85f));
									}
								}
							}

							if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
							{
								//ImRect rect = childWnd->Rect();
								//if (ImGui::IsMouseHoveringRect(rect.Min, rect.Max) && selFrameRot != nullptr)
								//{
								//int _t = (int)((ImGui::GetMousePos().x - ImGui::GetCursorScreenPos().x + (segW * 0.5f)) / segW);

								if (dragFrameRot != nullptr && dragStartTime != -1 && dragStartTime != (int)dragFrameRot->time)
								{
									//Undo
									UndoData* undoData = Undo::addUndo("Change animation clip key time");
									undoData->stringData.resize(2);
									undoData->intData.resize(4);

									undoData->undoAction = [=](UndoData* data)
									{
										AnimationClip* anim = AnimationClip::load(data->stringData[0][nullptr], data->stringData[1][nullptr]);
										if (anim != nullptr)
										{
											int idx = data->intData[0][nullptr];
											AnimationClipNode* _node = anim->getAnimationClipNodes()[idx];
											if (_node != nullptr)
											{
												auto& _keys = _node->getRotationKeys();
												TimeQuaternion& _key = _keys[data->intData[1][nullptr]];
												_key.time = data->intData[2][nullptr];
											}
										}
									};

									undoData->redoAction = [=](UndoData* data)
									{
										AnimationClip* anim = AnimationClip::load(data->stringData[0][nullptr], data->stringData[1][nullptr]);
										if (anim != nullptr)
										{
											int idx = data->intData[0][nullptr];
											AnimationClipNode* _node = anim->getAnimationClipNodes()[idx];
											if (_node != nullptr)
											{
												auto& _keys = _node->getRotationKeys();
												TimeQuaternion& _key = _keys[data->intData[1][nullptr]];
												_key.time = data->intData[3][nullptr];
											}
										}
									};

									auto _it = std::find(nodes.begin(), nodes.end(), openedAnimationNode);
									auto it = std::find_if(keys.begin(), keys.end(), [=](TimeQuaternion& v) -> bool { return v.time == (int)dragFrameRot->time; });
									int nodeIdx = std::distance(nodes.begin(), _it);
									int keyIdx = std::distance(keys.begin(), it);

									undoData->stringData[0][nullptr] = openedAnimation->getLocation();
									undoData->stringData[1][nullptr] = openedAnimation->getName();
									undoData->intData[0][nullptr] = nodeIdx;
									undoData->intData[1][nullptr] = keyIdx;
									undoData->intData[2][nullptr] = dragStartTime;
									undoData->intData[3][nullptr] = (int)dragFrameRot->time;
									//
								}
								//}
							}
						}
						//

						ImVec2 pos = childWnd->DC.CursorPos;

						for (int i = 0; i < numKeys; ++i)
						{
							for (int j = 0; j < keys.size(); ++j)
							{
								if ((int)keys[j].time == i)
								{
									tintColor = keyframeColor;

									if (selFrameRot != nullptr && selFrameRot->time == i)
										tintColor = ImGui::ColorConvertFloat4ToU32(ImVec4(0.2f, 0.5f, 1.0f, 0.85f));

									drawList->AddCircleFilled(ImVec2(pos.x + i * segW, pos.y), 3.0f, tintColor);

									break;
								}
							}
						}
					}

					ImGui::EndChild();
					ImGui::PopStyleVar();

					ImGui::SetCursorPosX(x1);

					ImGui::PopClipRect();
				}
				ImGui::EndColumns();

				//---

				ImGui::BeginColumns("anim_editor_transform_3", 3, ImGuiOldColumnFlags_NoBorder | ImGuiOldColumnFlags_NoResize);
				ImGui::SetColumnWidth(0, toolsW * 0.85f);
				ImGui::SetColumnWidth(1, toolsW * 0.15f);
				{
					ImGui::Text("Scale");
				}
				ImGui::NextColumn();
				{
					ImGui::PushID("##anim_editor_transform_3_btn_add");
					bool btn = ImGui::ImageButton((void*)addKeyframeTex->getHandle().idx, ImVec2(10, 10), ImVec2(0, 1), ImVec2(1, 0));
					ImGui::PopID();

					if (btn)
					{
						//Undo
						UndoData* undoData = Undo::addUndo("Add animation clip scale key");
						undoData->stringData.resize(2);
						undoData->intData.resize(2);

						undoData->undoAction = [=](UndoData* data)
						{
							AnimationClip* anim = AnimationClip::load(data->stringData[0][nullptr], data->stringData[1][nullptr]);
							if (anim != nullptr)
							{
								int idx = data->intData[0][nullptr];
								AnimationClipNode* _node = anim->getAnimationClipNodes()[idx];
								if (_node != nullptr)
								{
									auto& _keys = _node->getScalingKeys();
									_keys.erase(_keys.begin() + (_keys.size() - 1));
								}
							}
						};

						undoData->redoAction = [=](UndoData* data)
						{
							AnimationClip* anim = AnimationClip::load(data->stringData[0][nullptr], data->stringData[1][nullptr]);
							if (anim != nullptr)
							{
								int idx = data->intData[0][nullptr];
								AnimationClipNode* _node = anim->getAnimationClipNodes()[idx];
								if (_node != nullptr)
								{
									auto& _keys = _node->getScalingKeys();
									_keys.push_back(TimeVector3(data->intData[1][nullptr], glm::vec3(1.0f)));

									selFramePos = nullptr;
									selFrameRot = nullptr;
									selFrameScl = nullptr;
								}
							}
						};

						auto _it = std::find(nodes.begin(), nodes.end(), openedAnimationNode);
						int nodeIdx = std::distance(nodes.begin(), _it);

						undoData->stringData[0][nullptr] = openedAnimation->getLocation();
						undoData->stringData[1][nullptr] = openedAnimation->getName();
						undoData->intData[0][nullptr] = nodeIdx;
						undoData->intData[1][nullptr] = currentFrame;
						//
						
						//Add scale key frame
						auto& keys = openedAnimationNode->getScalingKeys();

						auto it = std::find_if(keys.begin(), keys.end(), [=](TimeVector3& v) -> bool { return v.time == currentFrame; });

						if (it == keys.end())
							keys.push_back(TimeVector3(currentFrame, glm::vec3(1.0f)));

						if (openedAnimation->getDuration() < currentFrame + 1)
							openedAnimation->setDuration(currentFrame + 1);

						selFramePos = nullptr;
						selFrameRot = nullptr;
						selFrameScl = nullptr;
					}

					MainWindow::HelpMarker("Add scale key");
				}
				ImGui::NextColumn();
				{
					float sY = ImGui::GetScrollY();

					ImGui::PushClipRect(ImVec2(frameLinePos.x, frameLinePos.y + sY), ImVec2(frameLinePos.x + ImGui::GetContentRegionAvail().x + style.WindowPadding.x,
						frameLinePos.y + sY + ImGui::GetCursorScreenPos().y - style.WindowBorderSize * 3.0f), true);

					//Keyframes
					float x = ImGui::GetCursorPosX();
					float x1 = x;
					ImGui::SetCursorPosX(x - timelineScroll);

					ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, style.WindowPadding.y));
					bool _c = ImGui::BeginChild("anim_editor_curves_scale", ImVec2(timelineW, 16), true);

					if (_c)
					{
						ImGuiWindow* childWnd = ImGui::GetCurrentWindow();
						ImDrawList* drawList = childWnd->DrawList;

						auto& keys = openedAnimationNode->getScalingKeys();

						ImU32 tintColor = keyframeColor;

						//Keyframe drag
						if (focused)
						{
							if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
							{
								ImRect rect = childWnd->Rect();
								if (ImGui::IsMouseHoveringRect(rect.Min, rect.Max))
								{
									if (!mouseDownScl)
									{
										mouseDownScl = true;

										int frm = (ImGui::GetMousePos().x - ImGui::GetCursorScreenPos().x + (segW * 0.5f)) / segW;
										auto it = std::find_if(keys.begin(), keys.end(), [=](TimeVector3& v) -> bool { return v.time == frm; });

										if (it != keys.end())
										{
											dragFrameScl = &*it;
											selFrameScl = &*it;
											selFrameSclVal = it->value;
											selFrameRot = nullptr;
											selFramePos = nullptr;

											dragStartTime = frm;
										}
										else
										{
											selFrameScl = nullptr;
											selFrameRot = nullptr;
											selFramePos = nullptr;

											dragStartTime = -1;
										}
									}
								}
							}

							if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
							{
								if (mouseDownScl)
								{
									//ImRect rect = childWnd->Rect();
									//if (ImGui::IsMouseHoveringRect(rect.Min, rect.Max))
									//{
									if (timelineW > 0.0f)
									{
										if (dragFrameScl != nullptr)
										{
											int _t = (int)((ImGui::GetMousePos().x - ImGui::GetCursorScreenPos().x + (segW * 0.5f)) / segW);
											auto it = std::find_if(keys.begin(), keys.end(), [=](TimeVector3& v) -> bool { return v.time == _t; });
											if (it == keys.end())
												dragFrameScl->time = _t;
										}
									}
									//}

									if (dragFrameScl != nullptr)
									{
										tintColor = ImGui::ColorConvertFloat4ToU32(ImVec4(0.2f, 0.5f, 1.0f, 0.85f));
									}
								}
							}

							if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
							{
								//ImRect rect = childWnd->Rect();
								//if (ImGui::IsMouseHoveringRect(rect.Min, rect.Max) && selFrameScl != nullptr)
								//{
								//int _t = (int)((ImGui::GetMousePos().x - ImGui::GetCursorScreenPos().x + (segW * 0.5f)) / segW);

								if (dragFrameScl != nullptr && dragStartTime != -1 && dragStartTime != (int)dragFrameScl->time)
								{
									//Undo
									UndoData* undoData = Undo::addUndo("Change animation clip key time");
									undoData->stringData.resize(2);
									undoData->intData.resize(4);

									undoData->undoAction = [=](UndoData* data)
									{
										AnimationClip* anim = AnimationClip::load(data->stringData[0][nullptr], data->stringData[1][nullptr]);
										if (anim != nullptr)
										{
											int idx = data->intData[0][nullptr];
											AnimationClipNode* _node = anim->getAnimationClipNodes()[idx];
											if (_node != nullptr)
											{
												auto& _keys = _node->getScalingKeys();
												TimeVector3& _key = _keys[data->intData[1][nullptr]];
												_key.time = data->intData[2][nullptr];
											}
										}
									};

									undoData->redoAction = [=](UndoData* data)
									{
										AnimationClip* anim = AnimationClip::load(data->stringData[0][nullptr], data->stringData[1][nullptr]);
										if (anim != nullptr)
										{
											int idx = data->intData[0][nullptr];
											AnimationClipNode* _node = anim->getAnimationClipNodes()[idx];
											if (_node != nullptr)
											{
												auto& _keys = _node->getScalingKeys();
												TimeVector3& _key = _keys[data->intData[1][nullptr]];
												_key.time = data->intData[3][nullptr];
											}
										}
									};

									auto _it = std::find(nodes.begin(), nodes.end(), openedAnimationNode);
									auto it = std::find_if(keys.begin(), keys.end(), [=](TimeVector3& v) -> bool { return v.time == (int)dragFrameScl->time; });
									int nodeIdx = std::distance(nodes.begin(), _it);
									int keyIdx = std::distance(keys.begin(), it);

									undoData->stringData[0][nullptr] = openedAnimation->getLocation();
									undoData->stringData[1][nullptr] = openedAnimation->getName();
									undoData->intData[0][nullptr] = nodeIdx;
									undoData->intData[1][nullptr] = keyIdx;
									undoData->intData[2][nullptr] = dragStartTime;
									undoData->intData[3][nullptr] = (int)dragFrameScl->time;
									//
								}
								//}
							}
						}
						//

						ImVec2 pos = childWnd->DC.CursorPos;

						for (int i = 0; i < numKeys; ++i)
						{
							for (int j = 0; j < keys.size(); ++j)
							{
								if ((int)keys[j].time == i)
								{
									tintColor = keyframeColor;

									if (selFrameScl != nullptr && selFrameScl->time == i)
										tintColor = ImGui::ColorConvertFloat4ToU32(ImVec4(0.2f, 0.5f, 1.0f, 0.85f));

									drawList->AddCircleFilled(ImVec2(pos.x + i * segW, pos.y), 3.0f, tintColor);

									break;
								}
							}
						}
					}

					ImGui::EndChild();
					ImGui::PopStyleVar();

					ImGui::SetCursorPosX(x1);

					ImGui::PopClipRect();
				}
				ImGui::EndColumns();
				//---
				
				//Current frame line
				float _sY = ImGui::GetScrollY();
				wnd->DrawList->PushClipRect(ImVec2(frameLinePos.x, frameLinePos.y + _sY), ImVec2(frameLinePos.x + ImGui::GetContentRegionAvail().x - toolsW,
					frameLinePos.y + _sY + ImGui::GetCursorScreenPos().y - style.WindowBorderSize * 3.0f), true);

				ImVec2 pos = frameLinePos;
				wnd->DrawList->AddLine(ImVec2(pos.x - timelineScroll + currentFrame * segW, pos.y),
					ImVec2(pos.x - timelineScroll + currentFrame * segW, ImGui::GetCursorScreenPos().y - style.WindowBorderSize * 3.0f),
					ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 0.1f, 0.1f, 1.0f)), 2.0f);

				wnd->DrawList->PopClipRect();
				//---
				
				//Current frame number text
				ImGui::BeginColumns("anim_editor_transform_current_key_text", 3, ImGuiOldColumnFlags_NoBorder | ImGuiOldColumnFlags_NoResize);
				ImGui::SetColumnWidth(0, toolsW * 0.85f);
				if (selFramePos != nullptr)
				{
					ImGui::SetNextItemWidth(-1);
					ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 1));
					ImGui::InputFloat3("##anim_editor_transform_current_value", &selFramePosVal.x, "%.5f");
					if (ImGui::IsItemDeactivatedAfterEdit())
					{
						//Undo
						addPositionUndo(selFramePosVal);

						selFramePos->value = selFramePosVal;
					}
					ImGui::PopStyleVar();
				}
				else if (selFrameRot != nullptr)
				{
					glm::vec3 val = Mathf::toEuler(selFrameRot->value);
					ImGui::SetNextItemWidth(-1);
					ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 1));
					ImGui::InputFloat3("##anim_editor_transform_current_value", &selFrameRotVal.x, "%.5f");
					if (ImGui::IsItemDeactivatedAfterEdit())
					{
						//Undo
						addRotationUndo(selFrameRotVal);

						selFrameRot->value = Mathf::toQuaternion(selFrameRotVal);
					}
					ImGui::PopStyleVar();
				}
				else if (selFrameScl != nullptr)
				{
					glm::vec3 val = selFrameScl->value;
					ImGui::SetNextItemWidth(-1);
					ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 1));
					ImGui::InputFloat3("##anim_editor_transform_current_value", &selFrameSclVal.x, "%.5f");
					if (ImGui::IsItemDeactivatedAfterEdit())
					{
						//Undo
						addScaleUndo(selFrameSclVal);

						selFrameScl->value = selFrameSclVal;
					}
					ImGui::PopStyleVar();
				}
				ImGui::SetColumnWidth(1, toolsW * 0.15f);
				ImGui::NextColumn();
				{
					if (selFramePos != nullptr ||
						selFrameRot != nullptr ||
						selFrameScl != nullptr)
					{
						ImGui::PushID("##anim_editor_transform_3_btn_add");
						bool btn = ImGui::ImageButton((void*)pasteTex->getHandle().idx, ImVec2(10, 10), ImVec2(0, 1), ImVec2(1, 0));
						ImGui::PopID();

						if (btn)
						{
							//Paste value
							if (selFramePos != nullptr)
							{
								glm::vec3 bufVal = ObjectEditor::getBufferLocalPosition();
								if (bufVal != glm::vec3(FLT_MAX))
								{
									//Undo
									addPositionUndo(bufVal);

									selFramePos->value = bufVal;
									selFramePosVal = bufVal;
								}
							}
							else if (selFrameRot != nullptr)
							{
								glm::quat bufVal = ObjectEditor::getBufferLocalRotation();
								if (bufVal != glm::quat(FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX))
								{
									//Undo
									addRotationUndo(bufVal);

									selFrameRot->value = bufVal;
									selFrameRotVal = Mathf::toEuler(bufVal);
								}
							}
							else if (selFrameScl != nullptr)
							{
								glm::vec3 bufVal = ObjectEditor::getBufferLocalScale();
								if (bufVal != glm::vec3(FLT_MAX))
								{
									//Undo
									addScaleUndo(bufVal);

									selFrameScl->value = bufVal;
									selFrameSclVal = bufVal;
								}
							}
						}

						MainWindow::getSingleton()->HelpMarker("Paste value");
					}
					else
					{
						ImGui::InvisibleButton("##anim_editor_transform_3_btn_add", ImVec2(10, 16));
					}
				}
				ImGui::NextColumn();
				{
					float sY = ImGui::GetScrollY();

					ImGui::PushClipRect(ImVec2(frameLinePos.x, frameLinePos.y + sY), ImVec2(frameLinePos.x + ImGui::GetContentRegionAvail().x + style.WindowPadding.x,
						frameLinePos.y + sY + ImGui::GetCursorScreenPos().y - style.WindowBorderSize * 3.0f), true);

					std::string txt = std::to_string(currentFrame + 1);
					float x = ImGui::GetCursorPosX();
					ImGui::SetCursorPosX(toolsW - timelineScroll + currentFrame * segW + segW);
					ImGui::TextColored(ImVec4(1.0f, 0.1f, 0.1f, 1.0f), txt.c_str());
					ImGui::SetCursorPosX(x);

					ImGui::PopClipRect();
				}
				ImGui::EndColumns();
				//---

				ImVec2 cur = ImGui::GetCursorPos();
				ImGui::Dummy(ImVec2(10, 16));
				ImGui::SetCursorPos(cur);

				//Scrollbar
				ImGui::BeginColumns("anim_editor_transform_scrollbar", 3, ImGuiOldColumnFlags_NoBorder | ImGuiOldColumnFlags_NoResize);
				ImGui::SetColumnWidth(0, toolsW * 0.7f);
				ImGui::SetColumnWidth(1, toolsW * 0.3f);
				ImGui::NextColumn();
				ImGui::NextColumn();
				{
					float w = ImGui::GetContentRegionAvail().x;
					ImGuiWindow* childWnd = ImGui::GetCurrentWindow();
					ImVec2 cur = ImGui::GetCursorPos();

					float sY = ImGui::GetScrollY();

					ImRect bb = ImRect(wnd->DC.CursorPos.x + cur.x, wnd->DC.CursorPos.y + cur.y - sY, wnd->DC.CursorPos.x + cur.x + w, wnd->DC.CursorPos.y + cur.y - sY + 16.0f);
					ImGuiID id = wnd->GetID("anim_editor_transform_scrollbar_b");
					float scrollWSpace = timelineW - w;
					float scrollW = w - scrollWSpace;

					if (scrollW > w)
						scrollW = w;
					
					ImGui::ScrollbarEx(bb, id, ImGuiAxis_X, &timelineScroll, scrollW, w, ImDrawFlags_RoundCornersAll);

					if (scrollWSpace < timelineScroll)
						timelineScroll = scrollWSpace;
				}
				ImGui::EndColumns();
				//---
			}
		}

		ImGui::EndChild();

		if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
		{
			dragStartTime = -1;
			
			mouseDownTimeline = false;
			mouseDownPos = false;
			mouseDownRot = false;
			mouseDownScl = false;

			dragFramePos = nullptr;
			dragFrameRot = nullptr;
			dragFrameScl = nullptr;
		}
	}

	void AnimationEditorWindow::addPositionUndo(glm::vec3 val)
	{
		UndoData* undoData = Undo::addUndo("Change animation clip key value");
		undoData->stringData.resize(2);
		undoData->intData.resize(2);
		undoData->vec3Data.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			AnimationClip* anim = AnimationClip::load(data->stringData[0][nullptr], data->stringData[1][nullptr]);
			if (anim != nullptr)
			{
				int idx = data->intData[0][nullptr];
				AnimationClipNode* _node = anim->getAnimationClipNodes()[idx];
				if (_node != nullptr)
				{
					auto& _keys = _node->getPositionKeys();
					TimeVector3& _key = _keys[data->intData[1][nullptr]];
					_key.value = data->vec3Data[0][nullptr];

					if (selFramePos != nullptr)
						selFramePosVal = selFramePos->value;
				}
			}
		};

		undoData->redoAction = [=](UndoData* data)
		{
			AnimationClip* anim = AnimationClip::load(data->stringData[0][nullptr], data->stringData[1][nullptr]);
			if (anim != nullptr)
			{
				int idx = data->intData[0][nullptr];
				AnimationClipNode* _node = anim->getAnimationClipNodes()[idx];
				if (_node != nullptr)
				{
					auto& _keys = _node->getPositionKeys();
					TimeVector3& _key = _keys[data->intData[1][nullptr]];
					_key.value = data->vec3Data[1][nullptr];

					if (selFramePos != nullptr)
						selFramePosVal = selFramePos->value;
				}
			}
		};

		auto& nodes = openedAnimation->getAnimationClipNodes();
		auto& keys = openedAnimationNode->getPositionKeys();
		auto _it = std::find(nodes.begin(), nodes.end(), openedAnimationNode);
		auto it = std::find_if(keys.begin(), keys.end(), [=](TimeVector3& v) -> bool { return v.time == selFramePos->time; });
		int nodeIdx = std::distance(nodes.begin(), _it);
		int keyIdx = std::distance(keys.begin(), it);

		undoData->stringData[0][nullptr] = openedAnimation->getLocation();
		undoData->stringData[1][nullptr] = openedAnimation->getName();
		undoData->intData[0][nullptr] = nodeIdx;
		undoData->intData[1][nullptr] = keyIdx;
		undoData->vec3Data[0][nullptr] = selFramePos->value;
		undoData->vec3Data[1][nullptr] = val;
	}

	void AnimationEditorWindow::addRotationUndo(glm::quat val)
	{
		UndoData* undoData = Undo::addUndo("Change animation clip key value");
		undoData->stringData.resize(2);
		undoData->intData.resize(2);
		undoData->quatData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			AnimationClip* anim = AnimationClip::load(data->stringData[0][nullptr], data->stringData[1][nullptr]);
			if (anim != nullptr)
			{
				int idx = data->intData[0][nullptr];
				AnimationClipNode* _node = anim->getAnimationClipNodes()[idx];
				if (_node != nullptr)
				{
					auto& _keys = _node->getRotationKeys();
					TimeQuaternion& _key = _keys[data->intData[1][nullptr]];
					_key.value = data->quatData[0][nullptr];

					if (selFrameRot != nullptr)
						selFrameRotVal = Mathf::toEuler(selFrameRot->value);
				}
			}
		};

		undoData->redoAction = [=](UndoData* data)
		{
			AnimationClip* anim = AnimationClip::load(data->stringData[0][nullptr], data->stringData[1][nullptr]);
			if (anim != nullptr)
			{
				int idx = data->intData[0][nullptr];
				AnimationClipNode* _node = anim->getAnimationClipNodes()[idx];
				if (_node != nullptr)
				{
					auto& _keys = _node->getRotationKeys();
					TimeQuaternion& _key = _keys[data->intData[1][nullptr]];
					_key.value = data->quatData[1][nullptr];

					if (selFrameRot != nullptr)
						selFrameRotVal = Mathf::toEuler(selFrameRot->value);
				}
			}
		};

		auto& nodes = openedAnimation->getAnimationClipNodes();
		auto& keys = openedAnimationNode->getRotationKeys();
		auto _it = std::find(nodes.begin(), nodes.end(), openedAnimationNode);
		auto it = std::find_if(keys.begin(), keys.end(), [=](TimeQuaternion& v) -> bool { return v.time == selFrameRot->time; });
		int nodeIdx = std::distance(nodes.begin(), _it);
		int keyIdx = std::distance(keys.begin(), it);

		undoData->stringData[0][nullptr] = openedAnimation->getLocation();
		undoData->stringData[1][nullptr] = openedAnimation->getName();
		undoData->intData[0][nullptr] = nodeIdx;
		undoData->intData[1][nullptr] = keyIdx;
		undoData->quatData[0][nullptr] = selFrameRot->value;
		undoData->quatData[1][nullptr] = val;
	}

	void AnimationEditorWindow::addScaleUndo(glm::vec3 val)
	{
		UndoData* undoData = Undo::addUndo("Change animation clip key value");
		undoData->stringData.resize(2);
		undoData->intData.resize(2);
		undoData->vec3Data.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			AnimationClip* anim = AnimationClip::load(data->stringData[0][nullptr], data->stringData[1][nullptr]);
			if (anim != nullptr)
			{
				int idx = data->intData[0][nullptr];
				AnimationClipNode* _node = anim->getAnimationClipNodes()[idx];
				if (_node != nullptr)
				{
					auto& _keys = _node->getScalingKeys();
					TimeVector3& _key = _keys[data->intData[1][nullptr]];
					_key.value = data->vec3Data[0][nullptr];

					if (selFrameScl != nullptr)
						selFrameSclVal = selFrameScl->value;
				}
			}
		};

		undoData->redoAction = [=](UndoData* data)
		{
			AnimationClip* anim = AnimationClip::load(data->stringData[0][nullptr], data->stringData[1][nullptr]);
			if (anim != nullptr)
			{
				int idx = data->intData[0][nullptr];
				AnimationClipNode* _node = anim->getAnimationClipNodes()[idx];
				if (_node != nullptr)
				{
					auto& _keys = _node->getScalingKeys();
					TimeVector3& _key = _keys[data->intData[1][nullptr]];
					_key.value = data->vec3Data[1][nullptr];

					if (selFrameScl != nullptr)
						selFrameSclVal = selFrameScl->value;
				}
			}
		};

		auto& nodes = openedAnimation->getAnimationClipNodes();
		auto& keys = openedAnimationNode->getScalingKeys();
		auto _it = std::find(nodes.begin(), nodes.end(), openedAnimationNode);
		auto it = std::find_if(keys.begin(), keys.end(), [=](TimeVector3& v) -> bool { return v.time == selFrameScl->time; });
		int nodeIdx = std::distance(nodes.begin(), _it);
		int keyIdx = std::distance(keys.begin(), it);

		undoData->stringData[0][nullptr] = openedAnimation->getLocation();
		undoData->stringData[1][nullptr] = openedAnimation->getName();
		undoData->intData[0][nullptr] = nodeIdx;
		undoData->intData[1][nullptr] = keyIdx;
		undoData->vec3Data[0][nullptr] = selFrameScl->value;
		undoData->vec3Data[1][nullptr] = val;
	}
}