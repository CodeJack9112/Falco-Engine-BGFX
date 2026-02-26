#include "TerrainEditor.h"

#include <cmath>
#include <imgui.h>
#include <imgui_internal.h>
#include <dear-imgui/misc/cpp/imgui_stdlib.h>

#include "../../Engine/glm/gtc/type_ptr.hpp"

#include "../Engine/Core/Engine.h"
#include "../Engine/Core/GameObject.h"
#include "../Engine/Components/Terrain.h"
#include "../Engine/Components/Camera.h"
#include "../Engine/Components/Transform.h"
#include "../Engine/Assets/Texture.h"
#include "../Engine/Assets/Material.h"
#include "../Engine/Assets/Mesh.h"
#include "../Engine/Classes/IO.h"
#include "../Engine/Classes/Helpers.h"
#include "../Engine/Core/InputManager.h"
#include "../Engine/Renderer/Renderer.h"
#include "../Engine/Renderer/Primitives.h"
#include "../Engine/Renderer/RenderTexture.h"
#include "../Engine/Math/Raycast.h"
#include "../Engine/Math/Mathf.h"

#include "../Engine/Serialization/Components/STerrain.h"

#include <sstream>

#include "PropFloat.h"
#include "PropInt.h"
#include "PropVector2.h"
#include "PropButton.h"
#include "PropCustom.h"

#include "../Windows/MainWindow.h"
#include "../Windows/SceneWindow.h"
#include "../Windows/AssetsWindow.h"
#include "../Windows/InspectorWindow.h"
#include "../Classes/TreeNode.h"
#include "../Classes/TreeView.h"
#include "../Classes/Undo.h"
#include "../Classes/Toast.h"

#include "../Engine/UI/ImGUIWidgets.h"

#include <SDL2/SDL.h>

namespace GX
{
	TerrainEditor::TerrainEditMode TerrainEditor::editMode = TerrainEditor::TerrainEditMode::NONE;

	TerrainEditor::TerrainEditor()
	{
		setEditorName("TerrainEditor");
	}

	TerrainEditor::~TerrainEditor()
	{
		editMode = TerrainEditMode::NONE;
		Renderer::getSingleton()->removeRenderCallback(renderCallbackId);
	}

	void TerrainEditor::init(std::vector<Component*> comps)
	{
		ComponentEditor::init(comps);

		renderCallbackId = Renderer::getSingleton()->addRenderCallback([=](int viewId, int viewLayer, Camera* camera) { drawBrushes(viewId, viewLayer, camera); });

		raiseIcon = Texture::load(Helper::ExePath(), "Editor/Icons/Terrain/terrain_raise.png", false, Texture::CompressionMethod::None, true);
		lowerIcon = Texture::load(Helper::ExePath(), "Editor/Icons/Terrain/terrain_lower.png", false, Texture::CompressionMethod::None, true);
		smoothIcon = Texture::load(Helper::ExePath(), "Editor/Icons/Terrain/terrain_smooth.png", false, Texture::CompressionMethod::None, true);
		flattenIcon = Texture::load(Helper::ExePath(), "Editor/Icons/Terrain/terrain_flatten.png", false, Texture::CompressionMethod::None, true);
		paintIcon = Texture::load(Helper::ExePath(), "Editor/Icons/Terrain/terrain_paint.png", false, Texture::CompressionMethod::None, true);
		grassIcon = Texture::load(Helper::ExePath(), "Editor/Icons/Terrain/terrain_grass.png", false, Texture::CompressionMethod::None, true);
		treeIcon = Texture::load(Helper::ExePath(), "Editor/Icons/Terrain/terrain_trees.png", false, Texture::CompressionMethod::None, true);
		meshIcon = Texture::load(Helper::ExePath(), "Editor/Icons/Terrain/terrain_detail_meshes.png", false, Texture::CompressionMethod::None, true);
		settingsIcon = Texture::load(Helper::ExePath(), "Editor/Icons/Terrain/terrain_settings.png", false, Texture::CompressionMethod::None, true);

		terrainIcon = MainWindow::loadEditorIcon("Inspector/terrain.png");
		materialIcon = MainWindow::loadEditorIcon("Assets/material.png");

		Terrain* terrain = (Terrain*)comps[0];

		/* TERRAIN EDITOR */

		PropCustom* terrainEditor = new PropCustom(this, "Terrain editor");
		terrainEditor->setOnUpdateCallback([=](Property* prop) { onUpdateTerrainEditor(prop); });

		addProperty(terrainEditor);
	}

	bool TerrainEditor::isEditModeActive()
	{
		return editMode != TerrainEditMode::NONE && editMode != TerrainEditMode::SETTINGS;
	}

	static void HelpMarker(const char* desc)
	{
		if (ImGui::IsItemHovered() && GImGui->HoveredIdTimer > 0.5f)
		{
			ImGui::BeginTooltip();
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextUnformatted(desc);
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
	}

	void TerrainEditor::onUpdateTerrainEditor(Property* prop)
	{
		int removeTexture = -1;
		int removeGrass = -1;
		int removeTree = -1;
		int removeMesh = -1;

		ImGui::Dummy(ImVec2(0, 5));

		Terrain* terrain = (Terrain*)components[0];

		ImVec4 col1 = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
		ImVec4 col2 = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
		ImVec4 col3 = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
		ImVec4 col4 = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
		ImVec4 col5 = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
		ImVec4 col6 = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
		ImVec4 col7 = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
		ImVec4 col8 = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
		ImVec4 col9 = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);

		if (editMode == TerrainEditMode::RAISE) col1 = ImVec4(1, 1, 1, 1);
		if (editMode == TerrainEditMode::LOWER) col2 = ImVec4(1, 1, 1, 1);
		if (editMode == TerrainEditMode::SMOOTH) col3 = ImVec4(1, 1, 1, 1);
		if (editMode == TerrainEditMode::FLATTEN) col4 = ImVec4(1, 1, 1, 1);
		if (editMode == TerrainEditMode::PAINT_TEXTURE) col5 = ImVec4(1, 1, 1, 1);
		if (editMode == TerrainEditMode::PAINT_TREES) col6 = ImVec4(1, 1, 1, 1);
		if (editMode == TerrainEditMode::PAINT_DETAIL_MESHES) col7 = ImVec4(1, 1, 1, 1);
		if (editMode == TerrainEditMode::PAINT_GRASS) col8 = ImVec4(1, 1, 1, 1);
		if (editMode == TerrainEditMode::SETTINGS) col9 = ImVec4(1, 1, 1, 1);

		ImVec2 btnSize = ImVec2(19, 19);

		if (ImGui::ImageButton((void*)raiseIcon->getHandle().idx, btnSize, ImVec2(0, 1), ImVec2(1, 0), -1, ImVec4(0, 0, 0, 0), col1))
		{
			editMode = TerrainEditMode::RAISE;
		}
		HelpMarker("Raise terrain");
		ImGui::SameLine();
		if (ImGui::ImageButton((void*)lowerIcon->getHandle().idx, btnSize, ImVec2(0, 1), ImVec2(1, 0), -1, ImVec4(0, 0, 0, 0), col2))
		{
			editMode = TerrainEditMode::LOWER;
		}
		HelpMarker("Lower terrain");
		ImGui::SameLine();
		if (ImGui::ImageButton((void*)smoothIcon->getHandle().idx, btnSize, ImVec2(0, 1), ImVec2(1, 0), -1, ImVec4(0, 0, 0, 0), col3))
		{
			editMode = TerrainEditMode::SMOOTH;
		}
		HelpMarker("Smooth terrain");
		ImGui::SameLine();
		if (ImGui::ImageButton((void*)flattenIcon->getHandle().idx, btnSize, ImVec2(0, 1), ImVec2(1, 0), -1, ImVec4(0, 0, 0, 0), col4))
		{
			editMode = TerrainEditMode::FLATTEN;
		}
		HelpMarker("Flatten terrain");
		ImGui::SameLine();
		if (ImGui::ImageButton((void*)paintIcon->getHandle().idx, btnSize, ImVec2(0, 1), ImVec2(1, 0), -1, ImVec4(0, 0, 0, 0), col5))
		{
			editMode = TerrainEditMode::PAINT_TEXTURE;
		}
		HelpMarker("Paint textures");
		ImGui::SameLine();
		if (ImGui::ImageButton((void*)treeIcon->getHandle().idx, btnSize, ImVec2(0, 1), ImVec2(1, 0), -1, ImVec4(0, 0, 0, 0), col6))
		{
			editMode = TerrainEditMode::PAINT_TREES;
		}
		HelpMarker("Paint trees");
		ImGui::SameLine();
		if (ImGui::ImageButton((void*)meshIcon->getHandle().idx, btnSize, ImVec2(0, 1), ImVec2(1, 0), -1, ImVec4(0, 0, 0, 0), col7))
		{
			editMode = TerrainEditMode::PAINT_DETAIL_MESHES;
		}
		HelpMarker("Paint detail meshes");
		ImGui::SameLine();
		if (ImGui::ImageButton((void*)grassIcon->getHandle().idx, btnSize, ImVec2(0, 1), ImVec2(1, 0), -1, ImVec4(0, 0, 0, 0), col8))
		{
			editMode = TerrainEditMode::PAINT_GRASS;
		}
		HelpMarker("Paint grass");
		ImGui::SameLine();
		if (ImGui::ImageButton((void*)settingsIcon->getHandle().idx, btnSize, ImVec2(0, 1), ImVec2(1, 0), -1, ImVec4(0, 0, 0, 0), col9))
		{
			editMode = TerrainEditMode::SETTINGS;
		}
		HelpMarker("Terrain settings");

		ImGui::Dummy(ImVec2(0, 5));

		if (editMode != TerrainEditMode::SETTINGS && editMode != TerrainEditMode::NONE)
		{
			ImGui::BeginColumns(std::string("tBrushSizeCln").c_str(), 2, ImGuiColumnsFlags_NoResize | ImGuiColumnsFlags_NoBorder);
			ImGui::Text("Brush size");
			ImGui::NextColumn();
			ImGui::PushItemWidth(-1);
			ImGui::SliderFloat("##brush_size", &brushSize, 0.0f, 50.0f, "%.4f");
			ImGui::PopItemWidth();
			ImGui::EndColumns();

			ImGui::BeginColumns(std::string("tBrushStrengthCln").c_str(), 2, ImGuiColumnsFlags_NoResize | ImGuiColumnsFlags_NoBorder);
			ImGui::Text("Brush strength");
			ImGui::NextColumn();
			ImGui::PushItemWidth(-1);
			ImGui::SliderFloat("##brush_strength", &brushStrength, 0.0f, 20.0f, "%.4f");
			ImGui::PopItemWidth();
			ImGui::EndColumns();
		}

		if (editMode == TerrainEditMode::FLATTEN)
		{
			ImGui::BeginColumns(std::string("tFlattenHeightCln").c_str(), 2, ImGuiColumnsFlags_NoResize | ImGuiColumnsFlags_NoBorder);
			ImGui::Text("Height");
			ImGui::NextColumn();
			ImGui::PushItemWidth(-1);
			ImGui::SliderFloat("##flatten_height", &flattenHeight, -100.0f, 100.0f, "%.4f");
			ImGui::PopItemWidth();
			ImGui::EndColumns();
		}

		//Texture list
		if (editMode == TerrainEditMode::PAINT_TEXTURE)
		{
			ImGuiWindow* window = GImGui->CurrentWindow;
			ImGuiID id = window->GetIDNoKeepAlive("##PaintTexturesVS");
			ImGui::BeginChild(id, ImVec2(0, 180), true);

			std::vector<TerrainTextureData>& paintTextures = terrain->getTextures();
			int i = 0;
			int j = 1;
			int cnt = (ImGui::GetWindowWidth() - 20) / 64.0;

			for (auto it = paintTextures.begin(); it != paintTextures.end(); ++it, ++i, ++j)
			{
				Texture* tex = it->diffuseTexture;
				if (tex == nullptr)
					tex = Texture::getNullTexture();

				if (i == selectedTexture)
				{
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.6f, 0.0f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.65f, 0.65f, 0.0f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.65f, 0.65f, 0.0f, 1.0f));
				}

				ImGui::ImageButton((void*)tex->getHandle().idx, ImVec2(64, 64), ImVec2(0, 1), ImVec2(1, 0));
				if (i == selectedTexture) ImGui::PopStyleColor(3);
				if (ImGui::IsItemClicked())
					selectedTexture = i;

				if (editTexture == -1)
				{
					if (ImGui::BeginPopupContextItem(("##texturePopup" + std::to_string(i)).c_str(), 1))
					{
						bool selected = false;

						if (ImGui::Selectable("Edit", false))
						{
							editTexture = i;
						}
						if (ImGui::Selectable("Remove", false))
						{
							removeTexture = i;
						}

						ImGui::EndPopup();
					}
				}

				if (j < cnt)
					ImGui::SameLine();
				else
					j = 0;
			}

			ImGui::EndChild();

			if (ImGui::Button("Add texture"))
			{
				addTerrainTexture();
			}
		}

		//Tree list
		if (editMode == TerrainEditor::TerrainEditMode::PAINT_TREES)
		{
			ImGuiWindow* window = GImGui->CurrentWindow;
			ImGuiID id = window->GetIDNoKeepAlive("##PaintTreesVS");
			ImGui::BeginChild(id, ImVec2(0, 180), true);

			std::vector<TerrainTreeData*>& list = terrain->getTrees();
			int i = 0;
			int j = 1;
			int cnt = (ImGui::GetWindowWidth() - 20) / 74.0;

			for (auto it = list.begin(); it != list.end(); ++it, ++i, ++j)
			{
				if (i == selectedTree)
				{
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.6f, 0.0f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.65f, 0.65f, 0.0f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.65f, 0.65f, 0.0f, 1.0f));
				}

				if (!bgfx::isValid((*it)->getImpostorTexture()))
					ImGui::ImageButton((void*)Texture::getNullTexture()->getHandle().idx, ImVec2(64, 64), ImVec2(0, 1), ImVec2(1, 0));
				else
					ImGui::ImageButton((void*)(*it)->getImpostorTexture().idx, ImVec2(64, 64), ImVec2(0, 1), ImVec2(1, 0));

				if (i == selectedTree) ImGui::PopStyleColor(3);
				if (ImGui::IsItemClicked())
					selectedTree = i;

				if (editTree == -1)
				{
					if (ImGui::BeginPopupContextItem(("##meshPopup" + std::to_string(i)).c_str(), 1))
					{
						bool selected = false;

						if (ImGui::Selectable("Edit", false))
						{
							editTree = i;
						}
						if (ImGui::Selectable("Remove", false))
						{
							removeTree = i;
						}

						ImGui::EndPopup();
					}
				}

				if (j < cnt)
					ImGui::SameLine();
				else
					j = 0;
			}

			ImGui::EndChild();

			if (ImGui::Button("Add tree"))
			{
				addTerrainTree();
			}

			ImGui::SameLine();

			if (ImGui::Button("Mass place trees"))
			{
				massPlaceTreesWnd = true;
			}
		}

		//Meshes list
		if (editMode == TerrainEditor::TerrainEditMode::PAINT_DETAIL_MESHES)
		{
			ImGuiWindow* window = GImGui->CurrentWindow;
			ImGuiID id = window->GetIDNoKeepAlive("##PaintMeshesVS");
			ImGui::BeginChild(id, ImVec2(0, 180), true);

			std::vector<TerrainDetailMeshData*>& list = terrain->getDetailMeshes();
			int i = 0;
			int j = 1;
			int cnt = (ImGui::GetWindowWidth() - 20) / 74.0;

			for (auto it = list.begin(); it != list.end(); ++it, ++i, ++j)
			{
				if (i == selectedMesh)
				{
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.6f, 0.0f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.65f, 0.65f, 0.0f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.65f, 0.65f, 0.0f, 1.0f));
				}
				
				if (!bgfx::isValid((*it)->getImpostorTexture()))
					ImGui::ImageButton((void*)Texture::getNullTexture()->getHandle().idx, ImVec2(64, 64), ImVec2(0, 1), ImVec2(1, 0));
				else
					ImGui::ImageButton((void*)(*it)->getImpostorTexture().idx, ImVec2(64, 64), ImVec2(0, 1), ImVec2(1, 0));

				if (i == selectedMesh) ImGui::PopStyleColor(3);
				if (ImGui::IsItemClicked())
					selectedMesh = i;

				if (editMesh == -1)
				{
					if (ImGui::BeginPopupContextItem(("##meshPopup" + std::to_string(i)).c_str(), 1))
					{
						bool selected = false;

						if (ImGui::Selectable("Edit", false))
						{
							editMesh = i;
						}
						if (ImGui::Selectable("Remove", false))
						{
							removeMesh = i;
						}

						ImGui::EndPopup();
					}
				}

				if (j < cnt)
					ImGui::SameLine();
				else
					j = 0;
			}

			ImGui::EndChild();

			if (ImGui::Button("Add mesh"))
			{
				addTerrainMesh();
			}
		}

		//Grass list
		if (editMode == TerrainEditor::TerrainEditMode::PAINT_GRASS)
		{
			ImGuiWindow* window = GImGui->CurrentWindow;
			ImGuiID id = window->GetIDNoKeepAlive("##PaintGrassVS");
			ImGui::BeginChild(id, ImVec2(0, 180), true);

			std::vector<TerrainGrassData*>& list = terrain->getGrass();
			int i = 0;
			int j = 1;
			int cnt = (ImGui::GetWindowWidth() - 20) / 74.0;

			for (auto it = list.begin(); it != list.end(); ++it, ++i, ++j)
			{
				Material* mat = (*it)->getMaterial();
				Texture* tex = nullptr;

				if (mat != nullptr)
				{
					std::vector<Uniform>& uniforms = mat->getUniforms();
					for (auto u = uniforms.begin(); u != uniforms.end(); ++u)
					{
						if (u->getType() == UniformType::Sampler2D)
						{
							tex = u->getValue<Sampler2DDef>().second;
							if (tex != nullptr)
								break;
						}
					}
				}

				if (tex == nullptr)
					tex = Texture::getNullTexture();

				if (i == selectedGrass)
				{
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.6f, 0.0f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.65f, 0.65f, 0.0f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.65f, 0.65f, 0.0f, 1.0f));
				}
				ImGui::ImageButton((void*)tex->getHandle().idx, ImVec2(64, 64), ImVec2(0, 1), ImVec2(1, 0));
				if (i == selectedGrass) ImGui::PopStyleColor(3);
				if (ImGui::IsItemClicked())
					selectedGrass = i;

				if (editGrass == -1)
				{
					if (ImGui::BeginPopupContextItem(("##grassPopup" + std::to_string(i)).c_str(), 1))
					{
						bool selected = false;

						if (ImGui::Selectable("Edit", false))
						{
							editGrass = i;
						}
						if (ImGui::Selectable("Remove", false))
						{
							removeGrass = i;
						}

						ImGui::EndPopup();
					}
				}

				if (j < cnt)
					ImGui::SameLine();
				else
					j = 0;
			}

			ImGui::EndChild();

			if (ImGui::Button("Add grass"))
			{
				addTerrainGrass();
			}
		}

		//Terrain settings
		if (editMode == TerrainEditor::TerrainEditMode::SETTINGS)
		{
			ImGui::BeginColumns(std::string("tTerrainFileCln").c_str(), 2, ImGuiColumnsFlags_NoResize | ImGuiColumnsFlags_NoBorder);
			ImGui::Text("Terrain file");
			ImGui::NextColumn();
			ImVec2 avSize = ImGui::GetContentRegionAvail();

			ImGui::PushItemWidth(-1);
			std::string tfileName = IO::GetFileNameWithExt(terrain->getFilePath());
			if (tfileName.empty()) tfileName = "None";
			if (ImGui::ImageButtonWithText((void*)terrainIcon->getHandle().idx, tfileName.c_str(), ImVec2(16, 16), ImVec2(0, 1), ImVec2(1, 0), ImVec2(avSize.x, -1)))
			{
				MainWindow::getAssetsWindow()->focusOnFile(terrain->getFilePath());
			}
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_TreeView"))
				{
					TreeNode* move_from = (TreeNode*)payload->Data;

					if (move_from->treeView->getTag() == "Assets")
					{
						if (IO::GetFileExtension(move_from->alias) == "terrain")
						{
							std::string path = move_from->getPath();
							setTerrainFilePath(path);
						}
					}
				}
				ImGui::EndDragDropTarget();
			}
			ImGui::PopItemWidth();
			ImGui::EndColumns();

			ImGui::BeginColumns(std::string("tMaterialCln").c_str(), 2, ImGuiColumnsFlags_NoResize | ImGuiColumnsFlags_NoBorder);
			ImGui::Text("Material");
			ImGui::NextColumn();
			ImGui::PushItemWidth(-1);
			Material* tmat = terrain->getMaterial();
			std::string tmatName = tmat != nullptr ? IO::GetFileName(tmat->getName()) : "None";
			if (ImGui::ImageButtonWithText((void*)materialIcon->getHandle().idx, tmatName.c_str(), ImVec2(16, 16), ImVec2(0, 1), ImVec2(1, 0), ImVec2(avSize.x, -1)))
			{
				MainWindow::getAssetsWindow()->focusOnFile(tmat->getName());
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
							setTerrainMaterial(path);
						}
					}
				}
				ImGui::EndDragDropTarget();
			}
			ImGui::PopItemWidth();
			ImGui::EndColumns();

			ImGui::BeginColumns(std::string("tSizeCln").c_str(), 2, ImGuiColumnsFlags_NoResize | ImGuiColumnsFlags_NoBorder);
			ImGui::Text("Size");
			ImGui::NextColumn();
			ImGui::PushItemWidth(-1);
			int current = terrain->getSize();
			if (ImGui::BeginCombo("##terrain_size", std::to_string(current).c_str()))
			{
				for (int n = 0; n < terrainSizes.size(); n++)
				{
					bool is_selected = (current == terrainSizes[n]);

					if (ImGui::Selectable(std::to_string(terrainSizes[n]).c_str(), is_selected))
					{
						current = terrainSizes[n];
						setTerrainSize(current);
					}

					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}

				ImGui::EndCombo();
			}
			ImGui::PopItemWidth();
			ImGui::EndColumns();

			ImGui::BeginColumns(std::string("tWSizeCln").c_str(), 2, ImGuiColumnsFlags_NoResize | ImGuiColumnsFlags_NoBorder);
			ImGui::Text("World size");
			ImGui::NextColumn();
			ImGui::PushItemWidth(-1);
			current = terrain->getWorldSize();
			if (ImGui::BeginCombo("##terrain_wsize", std::to_string(current).c_str()))
			{
				for (int n = 0; n < terrainWorldSizes.size(); n++)
				{
					bool is_selected = (current == terrainWorldSizes[n]);

					if (ImGui::Selectable(std::to_string(terrainWorldSizes[n]).c_str(), is_selected))
					{
						current = terrainWorldSizes[n];
						setTerrainWorldSize(current);
					}

					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}

				ImGui::EndCombo();
			}
			ImGui::PopItemWidth();
			ImGui::EndColumns();

			ImGui::Dummy(ImVec2(0, 2));
			ImGui::Separator();
			ImGui::Dummy(ImVec2(0, 2));

			ImGui::BeginColumns(std::string("tTreeImpostorDistCln").c_str(), 2, ImGuiColumnsFlags_NoResize | ImGuiColumnsFlags_NoBorder);
			ImGui::Text("Tree impostor start distance");
			ImGui::NextColumn();
			ImGui::PushItemWidth(-1);
			int tidist = terrain->getTreeImpostorStartDistance();
			if (ImGui::InputInt("##tree_impostor_distance", &tidist))
			{
				setTerrainTreeImpostorStartDistance(tidist);
			}
			ImGui::PopItemWidth();
			ImGui::EndColumns();

			ImGui::BeginColumns(std::string("tTreeDistCln").c_str(), 2, ImGuiColumnsFlags_NoResize | ImGuiColumnsFlags_NoBorder);
			ImGui::Text("Tree draw distance");
			ImGui::NextColumn();
			ImGui::PushItemWidth(-1);
			int tdist = terrain->getTreeDrawDistance();
			if (ImGui::InputInt("##tree_draw_distance", &tdist))
			{
				setTerrainTreeDrawDistance(tdist);
			}
			ImGui::PopItemWidth();
			ImGui::EndColumns();

			ImGui::BeginColumns(std::string("tMeshDistCln").c_str(), 2, ImGuiColumnsFlags_NoResize | ImGuiColumnsFlags_NoBorder);
			ImGui::Text("Detail meshes draw distance");
			ImGui::NextColumn();
			ImGui::PushItemWidth(-1);
			int mddist = terrain->getDetailMeshesDrawDistance();
			if (ImGui::InputInt("##detail_meshes_draw_distance", &mddist))
			{
				setTerrainDetailMeshesDrawDistance(mddist);
			}
			ImGui::PopItemWidth();
			ImGui::EndColumns();

			ImGui::BeginColumns(std::string("tGrassDistCln").c_str(), 2, ImGuiColumnsFlags_NoResize | ImGuiColumnsFlags_NoBorder);
			ImGui::Text("Grass draw distance");
			ImGui::NextColumn();
			ImGui::PushItemWidth(-1);
			int gddist = terrain->getGrassDrawDistance();
			if (ImGui::InputInt("##grass_draw_distance", &gddist))
			{
				setTerrainGrassDrawDistance(gddist);
			}
			ImGui::PopItemWidth();
			ImGui::EndColumns();

			ImGui::Dummy(ImVec2(0, 2));
			ImGui::Separator();
			ImGui::Dummy(ImVec2(0, 2));

			ImGui::BeginColumns(std::string("tTerrainCastShadowsCln").c_str(), 2, ImGuiColumnsFlags_NoResize | ImGuiColumnsFlags_NoBorder);
			ImGui::Text("Terrain cast shadows");
			ImGui::NextColumn();
			ImGui::PushItemWidth(-1);
			bool ttshadows = terrain->getCastShadows();
			if (ImGui::Checkbox("##terrain_cast_shadows", &ttshadows))
			{
				setTerrainCastShadows(ttshadows);
			}
			ImGui::PopItemWidth();
			ImGui::EndColumns();

			ImGui::BeginColumns(std::string("tTreesCastShadowsCln").c_str(), 2, ImGuiColumnsFlags_NoResize | ImGuiColumnsFlags_NoBorder);
			ImGui::Text("Trees cast shadows");
			ImGui::NextColumn();
			ImGui::PushItemWidth(-1);
			bool tshadows = terrain->getTreesCastShadows();
			if (ImGui::Checkbox("##trees_cast_shadows", &tshadows))
			{
				setTerrainTreesCastShadows(tshadows);
			}
			ImGui::PopItemWidth();
			ImGui::EndColumns();

			ImGui::BeginColumns(std::string("tDMeshesCastShadowsCln").c_str(), 2, ImGuiColumnsFlags_NoResize | ImGuiColumnsFlags_NoBorder);
			ImGui::Text("Detail meshes cast shadows");
			ImGui::NextColumn();
			ImGui::PushItemWidth(-1);
			bool dmshadows = terrain->getDetailMeshesCastShadows();
			if (ImGui::Checkbox("##dmeshes_cast_shadows", &dmshadows))
			{
				setTerrainDetailMeshesCastShadows(dmshadows);
			}
			ImGui::PopItemWidth();
			ImGui::EndColumns();

			ImGui::BeginColumns(std::string("tGrassCastShadowsCln").c_str(), 2, ImGuiColumnsFlags_NoResize | ImGuiColumnsFlags_NoBorder);
			ImGui::Text("Grass cast shadows");
			ImGui::NextColumn();
			ImGui::PushItemWidth(-1);
			bool gshadows = terrain->getGrassCastShadows();
			if (ImGui::Checkbox("##grass_cast_shadows", &gshadows))
			{
				setTerrainGrassCastShadows(gshadows);
			}
			ImGui::PopItemWidth();
			ImGui::EndColumns();

			ImGui::Dummy(ImVec2(0, 2));
			ImGui::Separator();
			ImGui::Dummy(ImVec2(0, 2));

			ImGui::BeginColumns(std::string("tDrawTreesCln").c_str(), 2, ImGuiColumnsFlags_NoResize | ImGuiColumnsFlags_NoBorder);
			ImGui::Text("Draw trees");
			ImGui::NextColumn();
			ImGui::PushItemWidth(-1);
			bool dtrees = terrain->getDrawTrees();
			if (ImGui::Checkbox("##draw_trees", &dtrees))
			{
				setTerrainDrawTrees(dtrees);
			}
			ImGui::PopItemWidth();
			ImGui::EndColumns();

			ImGui::BeginColumns(std::string("tDrawDMeshesCln").c_str(), 2, ImGuiColumnsFlags_NoResize | ImGuiColumnsFlags_NoBorder);
			ImGui::Text("Draw detail meshes");
			ImGui::NextColumn();
			ImGui::PushItemWidth(-1);
			bool ddmeshes = terrain->getDrawDetailMeshes();
			if (ImGui::Checkbox("##draw_dmeshes", &ddmeshes))
			{
				setTerrainDrawDetailMeshes(ddmeshes);
			}
			ImGui::PopItemWidth();
			ImGui::EndColumns();

			ImGui::BeginColumns(std::string("tDrawGrassCln").c_str(), 2, ImGuiColumnsFlags_NoResize | ImGuiColumnsFlags_NoBorder);
			ImGui::Text("Draw grass");
			ImGui::NextColumn();
			ImGui::PushItemWidth(-1);
			bool dgrass = terrain->getDrawGrass();
			if (ImGui::Checkbox("##draw_grass", &dgrass))
			{
				setTerrainDrawGrass(dgrass);
			}
			ImGui::PopItemWidth();
			ImGui::EndColumns();
		}

		if (editTexture > -1)
			textureEditorWindow();

		if (editGrass > -1)
			grassEditorWindow();

		if (editTree > -1)
			treeEditorWindow();

		if (editMesh > -1)
			meshEditorWindow();

		if (massPlaceTreesWnd)
			massPlaceTreesWindow();

		if (removeTexture > -1)
		{
			removeTerrainTexture(removeTexture);
			removeTexture = -1;
		}

		if (removeGrass > -1)
		{
			removeTerrainGrass(removeGrass);
			removeGrass = -1;
		}

		if (removeTree > -1)
		{
			removeTerrainTree(removeTree);
			removeTree = -1;
		}

		if (removeMesh > -1)
		{
			removeTerrainMesh(removeMesh);
			removeMesh = -1;
		}

		ImGui::Dummy(ImVec2(0, 5));

		onEditTerrain();
	}

	void TerrainEditor::paintTerrain()
	{
		Terrain* terrain = (Terrain*)components[0];

		int32_t terrainSize = (int32_t)terrain->getSize();
		int32_t terrainWorldSize = (int32_t)terrain->getWorldSize();

		float* heightMap = terrain->getHeightMap();
		TerrainTextureData* texData = nullptr;
		std::vector<TerrainTextureData>& texDataList = terrain->getTextures();
		if (selectedTexture > -1 && texDataList.size() > 0)
			texData = &texDataList[selectedTexture];

		TerrainGrassData* grassData = nullptr;
		std::vector<TerrainGrassData*>& grassDataList = terrain->getGrass();
		if (selectedGrass > -1 && grassDataList.size() > 0)
			grassData = grassDataList[selectedGrass];

		TerrainTreeData* treeData = nullptr;
		std::vector<TerrainTreeData*>& treeDataList = terrain->getTrees();
		if (selectedTree > -1 && treeDataList.size() > 0)
			treeData = treeDataList[selectedTree];

		TerrainDetailMeshData* meshData = nullptr;
		std::vector<TerrainDetailMeshData*>& meshDataList = terrain->getDetailMeshes();
		if (selectedMesh > -1 && meshDataList.size() > 0)
			meshData = meshDataList[selectedMesh];

		float strength = (brushStrength / 100.0f);

		std::vector<glm::vec2> addPositions;
		float scale = terrain->getScale();
		float offset = 1.0f * terrain->getScale();
		glm::vec3 tp = terrain->getGameObject()->getTransform()->getPosition();

		for (int32_t area_y = -brushSize; area_y < brushSize; ++area_y)
		{
			for (int32_t area_x = -brushSize; area_x < brushSize; ++area_x)
			{
				int32_t brush_x = lastHitPos.x + area_x;
				int32_t brush_y = lastHitPos.z + area_y;

				float brush_wx = lastHitPosWorld.x + area_x - 1.0f;
				float brush_wy = lastHitPosWorld.z + area_y + 1.0f;

				if (brush_wx <= tp.x - ((terrainWorldSize / 2.0f))) continue;
				if (brush_wx >= tp.x + (terrainWorldSize / 2.0f)) continue;
				if (brush_wy <= tp.z - (terrainWorldSize / 2.0f)) continue;
				if (brush_wy >= tp.z + ((terrainWorldSize / 2.0f))) continue;

				// Brush attenuation
				float a2 = (float)(area_x * area_x);
				float b2 = (float)(area_y * area_y);
				float brushAttn = (brushSize - std::sqrt(a2 + b2)) / scale;
				if (brushAttn < 0.0f) brushAttn = 0.0f;

				float height = terrain->getHeightAtWorldPosUnclamped(glm::vec3(brush_wx, 0, brush_wy));

				// Raise/Lower terrain
				if (editMode == TerrainEditMode::RAISE || editMode == TerrainEditMode::LOWER)
				{
					bool raise = editMode == TerrainEditMode::RAISE && (!InputManager::getSingleton()->getKey(SDL_SCANCODE_LSHIFT));

					// Raise/Lower and scale by brush power.
					float add = raise ? 1.0f * strength : -1.0f * strength;
					add *= brushAttn;

					height += add;

					if (height < -500.0f) height = -500.0f;
					if (height > 500.0f) height = 500.0f;

					terrain->setHeightAtWorldPos(glm::vec3(brush_wx, 0, brush_wy), height);
				}
				if (editMode == TerrainEditMode::SMOOTH)
				{
					float minHeight = FLT_MAX;
					float maxHeight = FLT_MIN;

					float avg = 0.0f;
					float num = 0.0f;

					float blockSize = brushSize / 4.0f;

					for (int32_t ay = area_y - blockSize; ay < area_y + blockSize; ++ay)
					{
						for (int32_t ax = area_x - blockSize; ax < area_x + blockSize; ++ax)
						{
							int32_t bx = lastHitPosWorld.x + ax - 1;
							int32_t by = lastHitPosWorld.z + ay + 1;

							if (bx <= tp.x - ((terrainWorldSize / 2.0f))) continue;
							if (bx >= tp.x + (terrainWorldSize / 2.0f)) continue;
							if (by <= tp.z - (terrainWorldSize / 2.0f)) continue;
							if (by >= tp.z + ((terrainWorldSize / 2.0f))) continue;

							float h = terrain->getHeightAtWorldPosUnclamped(glm::vec3(bx, 0, by));

							avg += h;
							num += 1.0f;
						}
					}

					if (avg != 0.0f && num != 0.0f)
					{
						float add = (avg / num);

						height = Mathf::lerp(height, add, strength * brushAttn);

						if (height < -500.0f) height = -500.0f;
						if (height > 500.0f) height = 500.0f;

						terrain->setHeightAtWorldPos(glm::vec3(brush_wx, 0, brush_wy), height);
					}
				}
				if (editMode == TerrainEditMode::FLATTEN)
				{
					if (!InputManager::getSingleton()->getKey(SDL_SCANCODE_LSHIFT))
					{
						float add = (flattenHeight - height) * strength;
						add *= brushAttn;

						height += add;

						if (height < -500.0f) height = -500.0f;
						if (height > 500.0f) height = 500.0f;

						terrain->setHeightAtWorldPos(glm::vec3(brush_wx, 0, brush_wy), height);
					}
					else
					{
						flattenHeight = terrain->getHeightAtWorldPos(glm::vec3(lastHitPosWorld.x, 0, lastHitPosWorld.z));
					}
				}
				if (editMode == TerrainEditMode::PAINT_TEXTURE)
				{
					if (texData != nullptr)
					{
						float add = 1.0f * brushAttn * strength;

						int selTexIdx = (int)std::ceil(selectedTexture / 4);
						Texture* splatTexture = terrain->getSplatTextures()[selTexIdx];

						int texWidth = splatTexture->getWidth();

						int tw = ((float)brush_x / scale) + ((texWidth - (texWidth / scale)) / 2.0f);
						int th = ((float)brush_y / scale) + ((texWidth - (texWidth / scale)) / 2.0f);

						if (tw < 0 || th < 0) continue;
						if (tw >= texWidth || th >= texWidth) continue;

						//Clear other splat textures at this point
						if (texDataList.size() > selectedTexture)
						{
							int otherTex = selectedTexture;
							for (auto td = texDataList.begin() + selectedTexture; td != texDataList.end(); ++td, ++otherTex)
							{
								TerrainTextureData& otherTexData = *td;
								if (otherTex == selectedTexture)
									continue;

								int texIdx = (int)std::ceil(otherTex / 4);
								Texture* otherSplatTexture = terrain->getSplatTextures()[texIdx];

								int pixelPos = ((th * texWidth) + tw) * 4 + (otherTex - (texIdx * 4));
								if (pixelPos < 0 || pixelPos >= (texWidth * texWidth) * 4)
									continue;

								int otherColor = otherSplatTexture->getData()[pixelPos] - add * 255;

								if (otherColor < 0) otherColor = 0;
								if (otherColor > 255) otherColor = 255;

								otherSplatTexture->getData()[pixelPos] = otherColor;
							}
						}
						//

						int pixelPos = ((th * texWidth) + tw) * 4 + (selectedTexture - (selTexIdx * 4));
						if (pixelPos < 0 || pixelPos >= (texWidth * texWidth) * 4)
							continue;

						int color = splatTexture->getData()[pixelPos] + add * 255;
						
						if (color < 0) color = 0;
						if (color > 255) color = 255;

						splatTexture->getData()[pixelPos] = color;
					}
				}
				if (editMode == TerrainEditMode::PAINT_GRASS)
				{
					if (grassData != nullptr && terrain->getDrawGrass())
					{
						float add = 1.0f * brushAttn * strength;

						if (add > 0)
						{
							float _xx = Mathf::RandomFloat(lastHitPos.x - brushSize, lastHitPos.x + brushSize);
							float _yy = Mathf::RandomFloat(lastHitPos.z - brushSize, lastHitPos.z + brushSize);

							glm::vec2 gpos = glm::vec2(_xx, _yy);
							addPositions.push_back(gpos);
						}
					}
				}
				if (editMode == TerrainEditMode::PAINT_TREES)
				{
					if (treeData != nullptr && terrain->getDrawTrees())
					{
						float add = 1.0f * brushAttn * strength;

						if (add > 0)
						{
							float _xx = Mathf::RandomFloat(lastHitPos.x - brushSize, lastHitPos.x + brushSize);
							float _yy = Mathf::RandomFloat(lastHitPos.z - brushSize, lastHitPos.z + brushSize);

							glm::vec2 gpos = glm::vec2(_xx, _yy);

							if (!InputManager::getSingleton()->getKey(SDL_SCANCODE_LSHIFT))
							{
								bool add = treeData->addTree(gpos, strength * 5.0f);
								if (add)
									treesModified = true;
							}
							else
							{
								for (auto& d : treeDataList)
								{
									bool rem = d->removeTree(gpos, strength * 5.0f);
									if (rem)
										treesModified = true;
								}
							}
						}
					}
				}
				if (editMode == TerrainEditMode::PAINT_DETAIL_MESHES)
				{
					if (meshData != nullptr && terrain->getDrawDetailMeshes())
					{
						float add = 1.0f * brushAttn * strength;

						if (add > 0)
						{
							float _xx = Mathf::RandomFloat(lastHitPos.x - brushSize, lastHitPos.x + brushSize);
							float _yy = Mathf::RandomFloat(lastHitPos.z - brushSize, lastHitPos.z + brushSize);

							glm::vec2 gpos = glm::vec2(_xx, _yy);
							addPositions.push_back(gpos);
						}
					}
				}
			}
		}

		if (editMode == TerrainEditMode::PAINT_GRASS)
		{
			if (grassData != nullptr && addPositions.size() > 0)
			{
				if (!InputManager::getSingleton()->getKey(SDL_SCANCODE_LSHIFT))
				{
					bool add = grassData->addGrass(addPositions, strength * 30.0f);
					if (add)
						grassModified = true;
				}
				else
				{
					for (auto& d : grassDataList)
					{
						bool rem = d->removeGrass(addPositions, strength * 30.0f);
						if (rem)
							grassModified = true;
					}
				}

				addPositions.clear();
			}
		}

		if (editMode == TerrainEditMode::PAINT_DETAIL_MESHES)
		{
			if (meshData != nullptr && addPositions.size() > 0)
			{
				if (!InputManager::getSingleton()->getKey(SDL_SCANCODE_LSHIFT))
				{
					bool add = meshData->addMesh(addPositions, strength * 30.0f);
					if (add)
						meshesModified = true;
				}
				else
				{
					for (auto& d : meshDataList)
					{
						bool rem = d->removeMesh(addPositions, strength * 30.0f);
						if (rem)
							meshesModified = true;
					}
				}

				addPositions.clear();
			}
		}

		if (editMode == TerrainEditMode::PAINT_TEXTURE)
		{
			std::vector<Texture*>& splatTextures = terrain->getSplatTextures();

			for (auto td = splatTextures.begin(); td != splatTextures.end(); ++td)
				(*td)->updateTexture();
		}
	}

	void TerrainEditor::mousePickTerrain()
	{
		auto mouseState = InputManager::getSingleton()->getMouseRelativePosition();
		float screenX = mouseState.first;
		float screenY = mouseState.second;

		bool posChanged = true;
		if (prevScreenX == screenX && prevScreenY == screenY)
			posChanged = false;

		prevScreenX = screenX;
		prevScreenY = screenY;

		Terrain* terrain = (Terrain*)components[0];
		Camera* camera = MainWindow::getSceneWindow()->getCamera();

		int32_t terrainSize = (int32_t)terrain->getSize();
		float* heightMap = terrain->getHeightMap();

		Ray ray = camera->getCameraToViewportRay(screenX, screenY);
		Raycast raycast;
		auto inf = raycast.execute(ray);

		for (auto it = inf.begin(); it != inf.end(); ++it)
		{
			if (it->object == terrain->getGameObject()->getTransform())
			{
				lastHitPosWorld = it->hitPoint;
				lastHitPos = it->hitPoint - terrain->getGameObject()->getTransform()->getPosition() + (terrainSize / 2.0f);
				lastHitPos.x = terrainSize - lastHitPos.x;
				
				if (clickInWindow)
				{
					if (InputManager::getSingleton()->getMouseButtonDown(0))
						paintTerrain();

					if (posChanged)
					{
						if (InputManager::getSingleton()->getMouseButton(0))
							paintTerrain();
					}
				}
			}
		}

		if (InputManager::getSingleton()->getMouseButtonUp(0))
		{
			if (editMode == TerrainEditMode::RAISE
				|| editMode == TerrainEditMode::LOWER
				|| editMode == TerrainEditMode::SMOOTH
				|| editMode == TerrainEditMode::FLATTEN)
			{
				//glm::vec3 _min = glm::vec3(lastHitPos.x - brushSize);
				//glm::vec3 _max = glm::vec3(lastHitPos.z + brushSize);
				terrain->updatePositions(/*_min, _max*/);
			}
		}
	}

	void TerrainEditor::onEditTerrain()
	{
		if (!isEditModeActive())
			return;

		if (components.size() == 0)
			return;

		Terrain* terr = (Terrain*)components[0];
		if (terr == nullptr)
			return;

		if (!terr->getEnabled() || !terr->getGameObject()->getActive())
			return;

		bool editTerr = (editMode == TerrainEditMode::RAISE ||
			editMode == TerrainEditMode::LOWER ||
			editMode == TerrainEditMode::FLATTEN ||
			editMode == TerrainEditMode::SMOOTH);

		
		if (InputManager::getSingleton()->getMouseButtonDown(0))
		{
			lmbDown = true;
		}

		if (MainWindow::getSceneWindow()->isHovered())
		{
			if (InputManager::getSingleton()->getMouseButtonDown(0))
			{
				clickInWindow = true;

				if (editTerr)
				{
					grassVisible = terr->getDrawGrass();
					treesVisible = terr->getDrawTrees();
					meshesVisible = terr->getDrawDetailMeshes();

					terr->setDrawGrass(false);
					terr->setDrawTrees(false);
					terr->setDrawDetailMeshes(false);

					//Undo
					undoDataHeightmap = Undo::addUndo("Paint terrain height");
					undoDataHeightmap->object2Data.resize(2);
					undoDataHeightmap->intData.resize(3);

					undoDataHeightmap->intData[0][nullptr] = static_cast<int>(editMode);

					undoDataHeightmap->undoAction = [=](UndoData* data)
					{
						for (auto& d : data->object2Data[0])
						{
							Terrain* terrain = (Terrain*)d.first;
							int dsize = data->intData[1][terrain];

							std::memcpy((void*)terrain->getHeightMap(), (void*)data->object2Data[0][terrain], dsize);
							terrain->setIsDirty();

							terrain->updatePositions();
						}

						editMode = static_cast<TerrainEditMode>(data->intData[0][nullptr]);
					};

					undoDataHeightmap->redoAction = [=](UndoData* data)
					{
						for (auto& d : data->object2Data[1])
						{
							Terrain* terrain = (Terrain*)d.first;
							int dsize = data->intData[2][terrain];

							std::memcpy((void*)terrain->getHeightMap(), (void*)data->object2Data[1][terrain], dsize);
							terrain->setIsDirty();

							terrain->updatePositions();
						}

						editMode = static_cast<TerrainEditMode>(data->intData[0][nullptr]);
					};

					undoDataHeightmap->destroyAction = [=](UndoData* data)
					{
						for (auto& d : data->object2Data)
						{
							for (auto& d2 : d)
								delete[] d2.second;
						}
					};

					for (auto& t : components)
					{
						Terrain* terrain = (Terrain*)t;

						int terrHeightSz = terrain->getSize() * terrain->getSize();
						float* terrHeight = new float[terrHeightSz];
						std::memcpy(terrHeight, terrain->getHeightMap(), terrHeightSz * sizeof(float));

						undoDataHeightmap->intData[1][terrain] = terrHeightSz * sizeof(float);
						undoDataHeightmap->object2Data[0][terrain] = terrHeight;
					}
					//
				}

				if (editMode == TerrainEditMode::PAINT_TEXTURE)
				{
					//Undo
					int splatCount = terr->getSplatTextures().size() * 2;
					undoDataTextures = Undo::addUndo("Paint terrain texture");
					undoDataTextures->object2Data.resize(splatCount);
					undoDataTextures->intData.resize(splatCount);

					undoDataTextures->undoAction = [=](UndoData* data)
					{
						for (int i = 0; i < data->object2Data.size() / 2; ++i)
						{
							for (auto& d : data->object2Data[i])
							{
								Terrain* terrain = (Terrain*)d.first;
								auto& splats = terrain->getSplatTextures();

								Texture* splatTexture = splats[i];
								int tsz = data->intData[i][terrain];
								char* data1 = (char*)d.second;
								splatTexture->allocData(tsz);
								memcpy(splatTexture->getData(), data1, tsz);
								splatTexture->updateTexture();

								editMode = TerrainEditMode::PAINT_TEXTURE;
							}
						}
					};

					undoDataTextures->redoAction = [=](UndoData* data)
					{
						int stride = data->object2Data.size() / 2;

						for (int i = stride; i < data->object2Data.size(); ++i)
						{
							for (auto& d : data->object2Data[i])
							{
								Terrain* terrain = (Terrain*)d.first;
								auto& splats = terrain->getSplatTextures();

								Texture* splatTexture = splats[i - stride];
								int tsz = data->intData[i][terrain];
								char* data1 = (char*)d.second;
								splatTexture->allocData(tsz);
								memcpy(splatTexture->getData(), data1, tsz);
								splatTexture->updateTexture();

								editMode = TerrainEditMode::PAINT_TEXTURE;
							}
						}
					};

					undoDataTextures->destroyAction = [=](UndoData* data)
					{
						for (auto& d : data->object2Data)
						{
							for (auto& d2 : d)
								delete[] d2.second;
						}
					};

					for (auto& t : components)
					{
						Terrain* terrain = (Terrain*)t;

						auto& splats = terrain->getSplatTextures();
						for (int i = 0; i < splats.size(); ++i)
						{
							Texture* splatTexture = splats[i];
							int tsz = splatTexture->getSize();
							char* data1 = new char[tsz];
							memcpy(data1, splatTexture->getData(), tsz);
							undoDataTextures->object2Data[i][terrain] = data1;
							undoDataTextures->intData[i][terrain] = tsz;
						}
					}
					//
				}

				if (editMode == TerrainEditMode::PAINT_TREES)
				{
					//Undo
					undoDataTrees = Undo::addUndo("Paint terrain trees");
					undoDataTrees->object2Data.resize(terr->getTrees().size() * 2);
					undoDataTrees->intData.resize(1);

					undoDataTrees->undoAction = [=](UndoData* data)
					{
						int tcount = data->object2Data.size() / 2;
						for (int i = 0; i < tcount; ++i)
						{
							for (auto& d : data->object2Data[i])
							{
								Terrain* terrain = (Terrain*)d.first;
								auto& trees = terrain->getTrees();

								STerrainTreeData* td = (STerrainTreeData*)d.second;
								TerrainTreeData* gd = terrain->deserializeTree(*td);
								terrain->removeTreeData(i);
								trees.insert(trees.begin() + i, gd);
							}
						}

						editMode = TerrainEditMode::PAINT_TREES;
					};

					undoDataTrees->redoAction = [=](UndoData* data)
					{
						int tcount = data->object2Data.size() / 2;
						for (int i = tcount; i < tcount * 2; ++i)
						{
							for (auto& d : data->object2Data[i])
							{
								Terrain* terrain = (Terrain*)d.first;
								auto& trees = terrain->getTrees();

								STerrainTreeData* td = (STerrainTreeData*)d.second;
								TerrainTreeData* gd = terrain->deserializeTree(*td);
								terrain->removeTreeData(i - tcount);
								trees.insert(trees.begin() + (i - tcount), gd);
							}
						}

						editMode = TerrainEditMode::PAINT_TREES;
					};

					undoDataTrees->destroyAction = [=](UndoData* data)
					{
						for (auto& d : data->object2Data)
						{
							for (auto& d2 : d)
								delete d2.second;
						}
					};

					for (auto& t : components)
					{
						Terrain* terrain = (Terrain*)t;

						auto& trees = terrain->getTrees();
						for (int i = 0; i < trees.size(); ++i)
						{
							STerrainTreeData* s = new STerrainTreeData(terrain->serializeTree(trees[i]));
							undoDataTrees->object2Data[i][terrain] = s;
						}
					}
					//
				}

				if (editMode == TerrainEditMode::PAINT_DETAIL_MESHES)
				{
					//Undo
					undoDataMeshes = Undo::addUndo("Paint terrain detail meshes");
					undoDataMeshes->object2Data.resize(terr->getDetailMeshes().size() * 2);
					undoDataMeshes->intData.resize(1);

					undoDataMeshes->undoAction = [=](UndoData* data)
					{
						int tcount = data->object2Data.size() / 2;
						for (int i = 0; i < tcount; ++i)
						{
							for (auto& d : data->object2Data[i])
							{
								Terrain* terrain = (Terrain*)d.first;
								auto& meshes = terrain->getDetailMeshes();

								STerrainDetailMeshData* td = (STerrainDetailMeshData*)d.second;
								TerrainDetailMeshData* gd = terrain->deserializeDetailMesh(*td);
								terrain->removeDetailMeshData(i);
								meshes.insert(meshes.begin() + i, gd);
							}
						}

						editMode = TerrainEditMode::PAINT_DETAIL_MESHES;
					};

					undoDataMeshes->redoAction = [=](UndoData* data)
					{
						int tcount = data->object2Data.size() / 2;
						for (int i = tcount; i < tcount * 2; ++i)
						{
							for (auto& d : data->object2Data[i])
							{
								Terrain* terrain = (Terrain*)d.first;
								auto& meshes = terrain->getDetailMeshes();

								STerrainDetailMeshData* td = (STerrainDetailMeshData*)d.second;
								TerrainDetailMeshData* gd = terrain->deserializeDetailMesh(*td);
								terrain->removeDetailMeshData(i - tcount);
								meshes.insert(meshes.begin() + (i - tcount), gd);
							}
						}

						editMode = TerrainEditMode::PAINT_DETAIL_MESHES;
					};

					undoDataMeshes->destroyAction = [=](UndoData* data)
					{
						for (auto& d : data->object2Data)
						{
							for (auto& d2 : d)
								delete d2.second;
						}
					};

					for (auto& t : components)
					{
						Terrain* terrain = (Terrain*)t;

						auto& meshes = terrain->getDetailMeshes();
						for (int i = 0; i < meshes.size(); ++i)
						{
							STerrainDetailMeshData* s = new STerrainDetailMeshData(terrain->serializeDetailMesh(meshes[i]));
							undoDataMeshes->object2Data[i][terrain] = s;
						}
					}
					//
				}

				if (editMode == TerrainEditMode::PAINT_GRASS)
				{
					//Undo
					undoDataGrass = Undo::addUndo("Paint terrain grass");
					undoDataGrass->object2Data.resize(terr->getGrass().size() * 2);
					undoDataGrass->intData.resize(1);

					undoDataGrass->undoAction = [=](UndoData* data)
					{
						int tcount = data->object2Data.size() / 2;
						for (int i = 0; i < tcount; ++i)
						{
							for (auto& d : data->object2Data[i])
							{
								Terrain* terrain = (Terrain*)d.first;
								auto& grass = terrain->getGrass();

								STerrainGrassData* td = (STerrainGrassData*)d.second;
								TerrainGrassData* gd = terrain->deserializeGrass(*td);
								terrain->removeGrassData(i);
								grass.insert(grass.begin() + i, gd);
							}
						}

						editMode = TerrainEditMode::PAINT_GRASS;
					};

					undoDataGrass->redoAction = [=](UndoData* data)
					{
						int tcount = data->object2Data.size() / 2;
						for (int i = tcount; i < tcount * 2; ++i)
						{
							for (auto& d : data->object2Data[i])
							{
								Terrain* terrain = (Terrain*)d.first;
								auto& grass = terrain->getGrass();

								STerrainGrassData* td = (STerrainGrassData*)d.second;
								TerrainGrassData* gd = terrain->deserializeGrass(*td);
								terrain->removeGrassData(i - tcount);
								grass.insert(grass.begin() + (i - tcount), gd);
							}
						}

						editMode = TerrainEditMode::PAINT_GRASS;
					};

					undoDataGrass->destroyAction = [=](UndoData* data)
					{
						for (auto& d : data->object2Data)
						{
							for (auto& d2 : d)
								delete d2.second;
						}
					};

					for (auto& t : components)
					{
						Terrain* terrain = (Terrain*)t;

						auto& grass = terrain->getGrass();
						for (int i = 0; i < grass.size(); ++i)
						{
							STerrainGrassData* s = new STerrainGrassData(terrain->serializeGrass(grass[i]));
							undoDataGrass->object2Data[i][terrain] = s;
						}
					}
					//
				}
			}
		}

		if (MainWindow::getSceneWindow()->isHovered())
		{
			mousePickTerrain();
		}

		if (InputManager::getSingleton()->getMouseButtonUp(0))
		{
			if (lmbDown)
			{
				if (editTerr)
				{
					if (clickInWindow)
					{
						terr->setDrawGrass(grassVisible);
						terr->setDrawTrees(treesVisible);
						terr->setDrawDetailMeshes(meshesVisible);
					}

					if (undoDataHeightmap != nullptr)
					{
						//Undo
						for (auto& t : components)
						{
							Terrain* terrain = (Terrain*)t;

							int terrHeightSz = terrain->getSize() * terrain->getSize();
							float* terrHeight = new float[terrHeightSz];
							std::memcpy(terrHeight, terrain->getHeightMap(), terrHeightSz * sizeof(float));

							undoDataHeightmap->intData[2][terrain] = terrHeightSz * sizeof(float);
							undoDataHeightmap->object2Data[1][terrain] = terrHeight;
						}
						//
					}

					undoDataHeightmap = nullptr;
				}

				if (editMode == TerrainEditMode::PAINT_TEXTURE)
				{
					//Undo
					if (undoDataTextures != nullptr)
					{
						for (auto& t : components)
						{
							Terrain* terrain = (Terrain*)t;

							auto& splats = terrain->getSplatTextures();
							for (int i = 0; i < splats.size(); ++i)
							{
								Texture* splatTexture = splats[i];
								int tsz = splatTexture->getSize();
								char* data1 = new char[tsz];
								memcpy(data1, splatTexture->getData(), tsz);
								undoDataTextures->object2Data[splats.size() + i][terrain] = data1;
								undoDataTextures->intData[splats.size() + i][terrain] = tsz;
							}
						}
					}

					undoDataTextures = nullptr;
					//
				}

				if (editMode == TerrainEditMode::PAINT_TREES)
				{
					if (undoDataTrees != nullptr)
					{
						if (treesModified)
						{
							//Undo
							for (auto& t : components)
							{
								Terrain* terrain = (Terrain*)t;

								auto& trees = terrain->getTrees();
								for (int i = 0; i < trees.size(); ++i)
								{
									STerrainTreeData* s = new STerrainTreeData(terrain->serializeTree(trees[i]));
									undoDataTrees->object2Data[trees.size() + i][terrain] = s;
								}
							}
							//
						}
						else
							Undo::removeUndo(undoDataTrees);
					}

					undoDataTrees = nullptr;
				}

				if (editMode == TerrainEditMode::PAINT_DETAIL_MESHES)
				{
					if (undoDataMeshes != nullptr)
					{
						if (meshesModified)
						{
							//Undo
							for (auto& t : components)
							{
								Terrain* terrain = (Terrain*)t;

								auto& meshes = terrain->getDetailMeshes();
								for (int i = 0; i < meshes.size(); ++i)
								{
									STerrainDetailMeshData* s = new STerrainDetailMeshData(terrain->serializeDetailMesh(meshes[i]));
									undoDataMeshes->object2Data[meshes.size() + i][terrain] = s;
								}
							}
							//
						}
						else
							Undo::removeUndo(undoDataMeshes);
					}

					undoDataMeshes = nullptr;
				}

				if (editMode == TerrainEditMode::PAINT_GRASS)
				{
					if (undoDataGrass != nullptr)
					{
						if (grassModified)
						{
							//Undo
							for (auto& t : components)
							{
								Terrain* terrain = (Terrain*)t;

								auto& grass = terrain->getGrass();
								for (int i = 0; i < grass.size(); ++i)
								{
									STerrainGrassData* s = new STerrainGrassData(terrain->serializeGrass(grass[i]));
									undoDataGrass->object2Data[grass.size() + i][terrain] = s;
								}
							}
							//
						}
						else
							Undo::removeUndo(undoDataGrass);
					}

					undoDataGrass = nullptr;
				}

				lmbDown = false;
			}

			clickInWindow = false;

			treesModified = false;
			meshesModified = false;
			grassModified = false;
		}
	}

	void TerrainEditor::addTerrainTexture()
	{
		Terrain* terrain = (Terrain*)components[0];

		if (terrain->getTextures().size() == MAX_TERRAIN_TEXTURES)
			return;

		//Undo
		UndoData* undoData = Undo::addUndo("Add terrain texture");
		undoData->objectData.resize(1);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->objectData[0])
			{
				Terrain* terr = (Terrain*)d;
				auto& textures = terr->getTextures();
				textures.erase(textures.begin() + textures.size() - 1);
			}
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->objectData[0])
			{
				Terrain* terr = (Terrain*)d;
				auto& textures = terr->getTextures();
				TerrainTextureData data = TerrainTextureData();
				textures.push_back(data);
			}
		};
		//

		for (auto& t : components)
		{
			Terrain* terr = (Terrain*)t;
			TerrainTextureData data = TerrainTextureData();
			terr->getTextures().push_back(data);

			undoData->objectData[0].push_back(terr);
		}
	}

	void TerrainEditor::removeTerrainTexture(int index)
	{
		if (selectedTexture >= index)
		{
			if (index - 1 > 0)
				selectedTexture = index - 1;
			else
				selectedTexture = 0;
		}

		//Undo
		UndoData* undoData = Undo::addUndo("Remove terrain texture");
		undoData->floatData.resize(1);
		undoData->stringData.resize(2);
		undoData->intData.resize(1);

		undoData->intData[0][nullptr] = index;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->floatData[0])
			{
				Terrain* terr = (Terrain*)d.first;
				auto& textures = terr->getTextures();

				Texture* tex1 = Texture::load(Engine::getSingleton()->getAssetsPath(), data->stringData[0][terr]);
				Texture* tex2 = Texture::load(Engine::getSingleton()->getAssetsPath(), data->stringData[1][terr]);

				TerrainTextureData texture;
				texture.worldSize = d.second;
				texture.diffuseTexture = tex1;
				texture.normalTexture = tex2;

				textures.insert(textures.begin() + idx, texture);
			}
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->floatData[0])
			{
				Terrain* terr = (Terrain*)d.first;
				auto& textures = terr->getTextures();
				textures.erase(textures.begin() + idx);
			}
		};
		//

		for (auto& t : components)
		{
			Terrain* terrain = (Terrain*)t;

			std::vector<TerrainTextureData>& textures = terrain->getTextures();

			TerrainTextureData tex = textures[index];
			if (tex.diffuseTexture != nullptr)
				undoData->stringData[0][terrain] = tex.diffuseTexture->getName();
			if (tex.normalTexture != nullptr)
				undoData->stringData[1][terrain] = tex.normalTexture->getName();
			undoData->floatData[0][terrain] = tex.worldSize;

			auto it = textures.begin() + index;
			textures.erase(it);
		}
	}

	void TerrainEditor::addTerrainGrass()
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Add terrain grass");
		undoData->objectData.resize(1);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->objectData[0])
			{
				Terrain* terr = (Terrain*)d;
				auto& grass = terr->getGrass();
				grass.erase(grass.begin() + grass.size() - 1);
			}
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->objectData[0])
			{
				Terrain* terr = (Terrain*)d;
				terr->addGrassData();
			}
		};
		//

		for (auto& t : components)
		{
			Terrain* terr = (Terrain*)t;
			terr->addGrassData();

			undoData->objectData[0].push_back(terr);
		}
	}

	void TerrainEditor::removeTerrainGrass(int index)
	{
		if (selectedGrass >= index)
		{
			if (index - 1 > 0)
				selectedGrass = index - 1;
			else
				selectedGrass = 0;
		}

		//Undo
		UndoData* undoData = Undo::addUndo("Remove terrain grass");
		undoData->stringData.resize(1);
		undoData->intData.resize(1);

		undoData->intData[0][nullptr] = index;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->stringData[0])
			{
				Terrain* terr = (Terrain*)d.first;
				
				STerrainGrassData g;
				std::stringstream ifs(d.second);
				BinarySerializer s;
				s.deserialize(&ifs, &g, "");
				
				TerrainGrassData* gd = terr->deserializeGrass(g);
				auto& grass = terr->getGrass();
				grass.insert(grass.begin() + idx, gd);
			}

			editMode = TerrainEditMode::PAINT_GRASS;
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->stringData[0])
			{
				Terrain* terr = (Terrain*)d.first;
				terr->removeGrassData(idx);
			}

			editMode = TerrainEditMode::PAINT_GRASS;
		};
		//

		for (auto& t : components)
		{
			Terrain* terrain = (Terrain*)t;

			auto& grass = terrain->getGrass();
			STerrainGrassData g = terrain->serializeGrass(grass[index]);
			std::stringstream ofs;
			BinarySerializer s;
			s.serialize(&ofs, &g, "");
			
			undoData->stringData[0][terrain] = ofs.str();

			terrain->removeGrassData(index);
		}
	}

	void TerrainEditor::addTerrainTree()
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Add terrain tree");
		undoData->objectData.resize(1);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->objectData[0])
			{
				Terrain* terr = (Terrain*)d;
				auto& trees = terr->getTrees();
				trees.erase(trees.begin() + trees.size() - 1);
			}
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->objectData[0])
			{
				Terrain* terr = (Terrain*)d;
				terr->addTreeData();
			}
		};
		//

		for (auto& t : components)
		{
			Terrain* terr = (Terrain*)t;
			terr->addTreeData();

			undoData->objectData[0].push_back(terr);
		}
	}

	void TerrainEditor::removeTerrainTree(int index)
	{
		if (selectedTree >= index)
		{
			if (index - 1 > 0)
				selectedTree = index - 1;
			else
				selectedTree = 0;
		}

		//Undo
		UndoData* undoData = Undo::addUndo("Remove terrain tree");
		undoData->stringData.resize(1);
		undoData->intData.resize(1);

		undoData->intData[0][nullptr] = index;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->stringData[0])
			{
				Terrain* terr = (Terrain*)d.first;
				
				STerrainTreeData g;
				std::stringstream ifs(d.second);
				BinarySerializer s;
				s.deserialize(&ifs, &g, "");
				
				TerrainTreeData* gd = terr->deserializeTree(g);
				auto& trees = terr->getTrees();
				trees.insert(trees.begin() + idx, gd);
			}

			editMode = TerrainEditMode::PAINT_TREES;
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->stringData[0])
			{
				Terrain* terr = (Terrain*)d.first;
				terr->removeTreeData(idx);
			}

			editMode = TerrainEditMode::PAINT_TREES;
		};
		//

		for (auto& t : components)
		{
			Terrain* terrain = (Terrain*)t;

			auto& trees = terrain->getTrees();
			STerrainTreeData g = terrain->serializeTree(trees[index]);
			std::stringstream ofs;
			BinarySerializer s;
			s.serialize(&ofs, &g, "");
			
			undoData->stringData[0][terrain] = ofs.str();

			terrain->removeTreeData(index);
		}
	}

	void TerrainEditor::addTerrainMesh()
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Add terrain detail mesh");
		undoData->objectData.resize(1);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->objectData[0])
			{
				Terrain* terr = (Terrain*)d;
				auto& meshes = terr->getDetailMeshes();
				meshes.erase(meshes.begin() + meshes.size() - 1);
			}
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->objectData[0])
			{
				Terrain* terr = (Terrain*)d;
				terr->addDetailMeshData();
			}
		};
		//

		for (auto& t : components)
		{
			Terrain* terr = (Terrain*)t;
			terr->addDetailMeshData();

			undoData->objectData[0].push_back(terr);
		}
	}

	void TerrainEditor::removeTerrainMesh(int index)
	{
		if (selectedMesh >= index)
		{
			if (index - 1 > 0)
				selectedMesh = index - 1;
			else
				selectedMesh = 0;
		}

		//Undo
		UndoData* undoData = Undo::addUndo("Remove terrain detail mesh");
		undoData->stringData.resize(1);
		undoData->intData.resize(1);

		undoData->intData[0][nullptr] = index;

		undoData->undoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->stringData[0])
			{
				Terrain* terr = (Terrain*)d.first;

				STerrainDetailMeshData g;
				std::stringstream ifs(d.second);
				BinarySerializer s;
				s.deserialize(&ifs, &g, "");

				TerrainDetailMeshData* gd = terr->deserializeDetailMesh(g);
				auto& detailMeshes = terr->getDetailMeshes();
				detailMeshes.insert(detailMeshes.begin() + idx, gd);
			}

			editMode = TerrainEditMode::PAINT_DETAIL_MESHES;
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int idx = data->intData[0][nullptr];

			for (auto& d : data->stringData[0])
			{
				Terrain* terr = (Terrain*)d.first;
				terr->removeDetailMeshData(idx);
			}

			editMode = TerrainEditMode::PAINT_DETAIL_MESHES;
		};
		//

		for (auto& t : components)
		{
			Terrain* terrain = (Terrain*)t;

			auto& detailMeshes = terrain->getDetailMeshes();
			STerrainDetailMeshData g = terrain->serializeDetailMesh(detailMeshes[index]);
			std::stringstream ofs;
			BinarySerializer s;
			s.serialize(&ofs, &g, "");

			undoData->stringData[0][terrain] = ofs.str();

			terrain->removeDetailMeshData(index);
		}
	}

	void TerrainEditor::setTerrainFilePath(std::string path)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Set terrain file path");
		undoData->stringData.resize(2);
		undoData->object2Data.resize(1);
		
		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->object2Data[0])
			{
				Terrain* terr = (Terrain*)d.first;

				STerrainData* td = (STerrainData*)d.second;
				terr->deserialize(*td, data->stringData[0][terr]);
			}

			editMode = TerrainEditMode::SETTINGS;
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->stringData[1])
			{
				Terrain* terr = (Terrain*)d.first;
				terr->load(d.second);
			}

			editMode = TerrainEditMode::SETTINGS;
		};

		undoData->destroyAction = [=](UndoData* data)
		{
			for (auto& d : data->object2Data)
			{
				for (auto& d2 : d)
					delete[] d2.second;
			}
		};
		//

		for (auto& comp : components)
		{
			Terrain* terrain = (Terrain*)comp;

			undoData->stringData[0][terrain] = terrain->getFilePath();
			undoData->stringData[1][terrain] = path;
			undoData->object2Data[0][terrain] = new STerrainData(terrain->serialize());

			terrain->load(path);
		}
	}

	void TerrainEditor::setTerrainMaterial(std::string path)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Set terrain material");
		undoData->stringData.resize(4);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->stringData[0])
			{
				Terrain* terr = (Terrain*)d.first;
				Material* mat = Material::load(data->stringData[0][terr], data->stringData[1][terr]);
				terr->setMaterial(mat);
			}

			editMode = TerrainEditMode::SETTINGS;
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->stringData[2])
			{
				Terrain* terr = (Terrain*)d.first;
				Material* mat = Material::load(data->stringData[2][terr], data->stringData[3][terr]);
				terr->setMaterial(mat);
			}

			editMode = TerrainEditMode::SETTINGS;
		};
		//

		for (auto& comp : components)
		{
			Terrain* terrain = (Terrain*)comp;

			Material* prevMat = terrain->getMaterial();
			
			undoData->stringData[0][terrain] = prevMat != nullptr ? prevMat->getLocation() : "";
			undoData->stringData[1][terrain] = prevMat != nullptr ? prevMat->getName() : "";

			undoData->stringData[2][terrain] = Engine::getSingleton()->getAssetsPath();
			undoData->stringData[3][terrain] = path;

			Material* tmat = Material::load(Engine::getSingleton()->getAssetsPath(), path);
			if (tmat != nullptr)
				terrain->setMaterial(tmat);
		}
	}

	void TerrainEditor::setTerrainSize(int value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Set terrain size");
		undoData->intData.resize(1);
		undoData->object2Data.resize(1);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->object2Data[0])
			{
				Terrain* terr = (Terrain*)d.first;

				STerrainData* td = (STerrainData*)d.second;
				terr->deserialize(*td, terr->getFilePath());
			}

			editMode = TerrainEditMode::SETTINGS;
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->intData[0])
			{
				Terrain* terr = (Terrain*)d.first;
				terr->setSize(d.second);
			}

			editMode = TerrainEditMode::SETTINGS;
		};

		undoData->destroyAction = [=](UndoData* data)
		{
			for (auto& d : data->object2Data)
			{
				for (auto& d2 : d)
					delete[] d2.second;
			}
		};
		//

		for (auto& comp : components)
		{
			Terrain* terrain = (Terrain*)comp;

			undoData->intData[0][terrain] = value;
			undoData->object2Data[0][terrain] = new STerrainData(terrain->serialize());

			terrain->setSize(value);
		}
	}

	void TerrainEditor::setTerrainWorldSize(int value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Set terrain world size");
		undoData->intData.resize(1);
		undoData->object2Data.resize(1);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->object2Data[0])
			{
				Terrain* terr = (Terrain*)d.first;

				STerrainData* td = (STerrainData*)d.second;
				terr->deserialize(*td, terr->getFilePath());
			}

			editMode = TerrainEditMode::SETTINGS;
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->intData[0])
			{
				Terrain* terr = (Terrain*)d.first;
				terr->setWorldSize(d.second);
			}

			editMode = TerrainEditMode::SETTINGS;
		};

		undoData->destroyAction = [=](UndoData* data)
		{
			for (auto& d : data->object2Data)
			{
				for (auto& d2 : d)
					delete[] d2.second;
			}
		};
		//

		for (auto& comp : components)
		{
			Terrain* terrain = (Terrain*)comp;

			undoData->intData[0][terrain] = value;
			undoData->object2Data[0][terrain] = new STerrainData(terrain->serialize());

			terrain->setWorldSize(value);
		}
	}

	void TerrainEditor::setTerrainTreeImpostorStartDistance(int value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Set terrain tree impostor start distance");
		undoData->intData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->intData[0])
			{
				Terrain* terr = (Terrain*)d.first;
				terr->setTreeImpostorStartDistance(d.second);
			}

			editMode = TerrainEditMode::SETTINGS;
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->intData[1])
			{
				Terrain* terr = (Terrain*)d.first;
				terr->setTreeImpostorStartDistance(d.second);
			}

			editMode = TerrainEditMode::SETTINGS;
		};
		//

		for (auto& comp : components)
		{
			Terrain* terrain = (Terrain*)comp;

			undoData->intData[0][terrain] = terrain->getTreeImpostorStartDistance();
			undoData->intData[1][terrain] = value;

			terrain->setTreeImpostorStartDistance(value);
		}
	}

	void TerrainEditor::setTerrainTreeDrawDistance(int value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Set terrain tree draw distance");
		undoData->intData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->intData[0])
			{
				Terrain* terr = (Terrain*)d.first;
				terr->setTreeDrawDistance(d.second);
			}

			editMode = TerrainEditMode::SETTINGS;
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->intData[1])
			{
				Terrain* terr = (Terrain*)d.first;
				terr->setTreeDrawDistance(d.second);
			}

			editMode = TerrainEditMode::SETTINGS;
		};
		//

		for (auto& comp : components)
		{
			Terrain* terrain = (Terrain*)comp;

			undoData->intData[0][terrain] = terrain->getTreeDrawDistance();
			undoData->intData[1][terrain] = value;

			terrain->setTreeDrawDistance(value);
		}
	}

	void TerrainEditor::setTerrainDetailMeshesDrawDistance(int value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Set terrain detail meshes draw distance");
		undoData->intData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->intData[0])
			{
				Terrain* terr = (Terrain*)d.first;
				terr->setDetailMeshesDrawDistance(d.second);
			}

			editMode = TerrainEditMode::SETTINGS;
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->intData[1])
			{
				Terrain* terr = (Terrain*)d.first;
				terr->setDetailMeshesDrawDistance(d.second);
			}

			editMode = TerrainEditMode::SETTINGS;
		};
		//

		for (auto& comp : components)
		{
			Terrain* terrain = (Terrain*)comp;

			undoData->intData[0][terrain] = terrain->getDetailMeshesDrawDistance();
			undoData->intData[1][terrain] = value;

			terrain->setDetailMeshesDrawDistance(value);
		}
	}

	void TerrainEditor::setTerrainGrassDrawDistance(int value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Set terrain grass draw distance");
		undoData->intData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->intData[0])
			{
				Terrain* terr = (Terrain*)d.first;
				terr->setGrassDrawDistance(d.second);
			}

			editMode = TerrainEditMode::SETTINGS;
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->intData[1])
			{
				Terrain* terr = (Terrain*)d.first;
				terr->setGrassDrawDistance(d.second);
			}

			editMode = TerrainEditMode::SETTINGS;
		};
		//

		for (auto& comp : components)
		{
			Terrain* terrain = (Terrain*)comp;

			undoData->intData[0][terrain] = terrain->getGrassDrawDistance();
			undoData->intData[1][terrain] = value;

			terrain->setGrassDrawDistance(value);
		}
	}

	void TerrainEditor::setTerrainCastShadows(bool value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Set terrain cast shadows");
		undoData->boolData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[0])
			{
				Terrain* terr = (Terrain*)d.first;
				terr->setCastShadows(d.second);
			}

			editMode = TerrainEditMode::SETTINGS;
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[1])
			{
				Terrain* terr = (Terrain*)d.first;
				terr->setCastShadows(d.second);
			}

			editMode = TerrainEditMode::SETTINGS;
		};
		//

		for (auto& comp : components)
		{
			Terrain* terrain = (Terrain*)comp;

			undoData->boolData[0][terrain] = terrain->getCastShadows();
			undoData->boolData[1][terrain] = value;

			terrain->setCastShadows(value);
		}
	}

	void TerrainEditor::setTerrainTreesCastShadows(bool value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Set terrain trees cast shadows");
		undoData->boolData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[0])
			{
				Terrain* terr = (Terrain*)d.first;
				terr->setTreesCastShadows(d.second);
			}

			editMode = TerrainEditMode::SETTINGS;
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[1])
			{
				Terrain* terr = (Terrain*)d.first;
				terr->setTreesCastShadows(d.second);
			}

			editMode = TerrainEditMode::SETTINGS;
		};
		//

		for (auto& comp : components)
		{
			Terrain* terrain = (Terrain*)comp;

			undoData->boolData[0][terrain] = terrain->getTreesCastShadows();
			undoData->boolData[1][terrain] = value;

			terrain->setTreesCastShadows(value);
		}
	}

	void TerrainEditor::setTerrainDetailMeshesCastShadows(bool value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Set terrain detail meshes cast shadows");
		undoData->boolData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[0])
			{
				Terrain* terr = (Terrain*)d.first;
				terr->setDetailMeshesCastShadows(d.second);
			}

			editMode = TerrainEditMode::SETTINGS;
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[1])
			{
				Terrain* terr = (Terrain*)d.first;
				terr->setDetailMeshesCastShadows(d.second);
			}

			editMode = TerrainEditMode::SETTINGS;
		};
		//

		for (auto& comp : components)
		{
			Terrain* terrain = (Terrain*)comp;

			undoData->boolData[0][terrain] = terrain->getDetailMeshesCastShadows();
			undoData->boolData[1][terrain] = value;

			terrain->setDetailMeshesCastShadows(value);
		}
	}

	void TerrainEditor::setTerrainGrassCastShadows(bool value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Set terrain grass cast shadows");
		undoData->boolData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[0])
			{
				Terrain* terr = (Terrain*)d.first;
				terr->setGrassCastShadows(d.second);
			}

			editMode = TerrainEditMode::SETTINGS;
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[1])
			{
				Terrain* terr = (Terrain*)d.first;
				terr->setGrassCastShadows(d.second);
			}

			editMode = TerrainEditMode::SETTINGS;
		};
		//

		for (auto& comp : components)
		{
			Terrain* terrain = (Terrain*)comp;
			
			undoData->boolData[0][terrain] = terrain->getGrassCastShadows();
			undoData->boolData[1][terrain] = value;

			terrain->setGrassCastShadows(value);
		}
	}

	void TerrainEditor::setTerrainDrawTrees(bool value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Set terrain draw trees");
		undoData->boolData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[0])
			{
				Terrain* terr = (Terrain*)d.first;
				terr->setDrawTrees(d.second);
				treesVisible = d.second;
			}

			editMode = TerrainEditMode::SETTINGS;
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[1])
			{
				Terrain* terr = (Terrain*)d.first;
				terr->setDrawTrees(d.second);
				treesVisible = d.second;
			}

			editMode = TerrainEditMode::SETTINGS;
		};
		//

		for (auto& comp : components)
		{
			Terrain* terrain = (Terrain*)comp;

			undoData->boolData[0][terrain] = terrain->getDrawTrees();
			undoData->boolData[1][terrain] = value;

			terrain->setDrawTrees(value);
		}

		treesVisible = value;
	}

	void TerrainEditor::setTerrainDrawDetailMeshes(bool value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Set terrain draw detail meshes");
		undoData->boolData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[0])
			{
				Terrain* terr = (Terrain*)d.first;
				terr->setDrawDetailMeshes(d.second);
				meshesVisible = d.second;
			}

			editMode = TerrainEditMode::SETTINGS;
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[1])
			{
				Terrain* terr = (Terrain*)d.first;
				terr->setDrawDetailMeshes(d.second);
				meshesVisible = d.second;
			}

			editMode = TerrainEditMode::SETTINGS;
		};
		//

		for (auto& comp : components)
		{
			Terrain* terrain = (Terrain*)comp;

			undoData->boolData[0][terrain] = terrain->getDrawDetailMeshes();
			undoData->boolData[1][terrain] = value;

			terrain->setDrawDetailMeshes(value);
		}

		meshesVisible = value;
	}

	void TerrainEditor::setTerrainDrawGrass(bool value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Set terrain draw grass");
		undoData->boolData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[0])
			{
				Terrain* terr = (Terrain*)d.first;
				terr->setDrawGrass(d.second);
				grassVisible = d.second;
			}

			editMode = TerrainEditMode::SETTINGS;
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[1])
			{
				Terrain* terr = (Terrain*)d.first;
				terr->setDrawGrass(d.second);
				grassVisible = d.second;
			}

			editMode = TerrainEditMode::SETTINGS;
		};
		//

		for (auto& comp : components)
		{
			Terrain* terrain = (Terrain*)comp;

			undoData->boolData[0][terrain] = terrain->getDrawGrass();
			undoData->boolData[1][terrain] = value;

			terrain->setDrawGrass(value);
		}

		grassVisible = value;
	}

	void TerrainEditor::textureEditorWindow()
	{
		if (ImGui::Begin("Edit terrain texture", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDocking))
		{
			Terrain* terrain = (Terrain*)components[0];

			std::vector<TerrainTextureData>& paintTextures = terrain->getTextures();

			Texture* diffuse = paintTextures[editTexture].diffuseTexture;
			Texture* normal = paintTextures[editTexture].normalTexture;

			if (diffuse == nullptr)
				diffuse = Texture::getNullTexture();

			if (normal == nullptr)
				normal = Texture::getNullTexture();

			TerrainTextureData& dt = paintTextures.at(editTexture);

			if (ImGui::ImageButton((void*)diffuse->getHandle().idx, ImVec2(64, 64)))
			{
				if (diffuse != nullptr)
					MainWindow::getAssetsWindow()->focusOnFile(diffuse->getName());
			}
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_TreeView"))
				{
					TreeNode* move_from = (TreeNode*)payload->Data;
					if (move_from->treeView->getTag() == "Assets")
					{
						auto images = Engine::getImageFileFormats();
						if (find(images.begin(), images.end(), IO::GetFileExtension(move_from->alias)) != images.end())
						{
							//Undo
							UndoData* undoData = Undo::addUndo("Change terrain diffuse texture");
							undoData->stringData.resize(2);
							undoData->intData.resize(1);

							undoData->intData[0][nullptr] = editTexture;

							undoData->undoAction = [=](UndoData* data)
							{
								int idx = data->intData[0][nullptr];

								for (auto& d : data->stringData[0])
								{
									Terrain* terr = (Terrain*)d.first;
									auto& textures = terr->getTextures();
									TerrainTextureData& tex = textures[idx];
									tex.diffuseTexture = Texture::load(Engine::getSingleton()->getAssetsPath(), d.second);
								}

								editMode = TerrainEditMode::PAINT_TEXTURE;
							};

							undoData->redoAction = [=](UndoData* data)
							{
								int idx = data->intData[0][nullptr];

								for (auto& d : data->stringData[1])
								{
									Terrain* terr = (Terrain*)d.first;
									auto& textures = terr->getTextures();
									TerrainTextureData& tex = textures[idx];
									tex.diffuseTexture = Texture::load(Engine::getSingleton()->getAssetsPath(), d.second);
								}

								editMode = TerrainEditMode::PAINT_TEXTURE;
							};
							//

							std::string path = move_from->getPath();

							undoData->stringData[0][terrain] = dt.diffuseTexture != nullptr ? dt.diffuseTexture->getName() : "";
							undoData->stringData[1][terrain] = path;

							dt.diffuseTexture = Texture::load(Engine::getSingleton()->getAssetsPath(), path);
						}
					}
				}

				ImGui::EndDragDropTarget();
			}

			ImGui::SameLine();
			if (ImGui::ImageButton((void*)normal->getHandle().idx, ImVec2(64, 64)))
			{
				if (normal != nullptr)
					MainWindow::getAssetsWindow()->focusOnFile(normal->getName());
			}
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_TreeView"))
				{
					TreeNode* move_from = (TreeNode*)payload->Data;
					if (move_from->treeView->getTag() == "Assets")
					{
						auto images = Engine::getImageFileFormats();
						if (find(images.begin(), images.end(), IO::GetFileExtension(move_from->alias)) != images.end())
						{
							//Undo
							UndoData* undoData = Undo::addUndo("Change terrain normal texture");
							undoData->stringData.resize(2);
							undoData->intData.resize(1);

							undoData->intData[0][nullptr] = editTexture;

							undoData->undoAction = [=](UndoData* data)
							{
								int idx = data->intData[0][nullptr];

								for (auto& d : data->stringData[0])
								{
									Terrain* terr = (Terrain*)d.first;
									auto& textures = terr->getTextures();
									TerrainTextureData& tex = textures[idx];
									tex.normalTexture = Texture::load(Engine::getSingleton()->getAssetsPath(), d.second);
								}

								editMode = TerrainEditMode::PAINT_TEXTURE;
							};

							undoData->redoAction = [=](UndoData* data)
							{
								int idx = data->intData[0][nullptr];

								for (auto& d : data->stringData[1])
								{
									Terrain* terr = (Terrain*)d.first;
									auto& textures = terr->getTextures();
									TerrainTextureData& tex = textures[idx];
									tex.normalTexture = Texture::load(Engine::getSingleton()->getAssetsPath(), d.second);
								}

								editMode = TerrainEditMode::PAINT_TEXTURE;
							};
							//

							std::string path = move_from->getPath();

							undoData->stringData[0][terrain] = dt.normalTexture != nullptr ? dt.normalTexture->getName() : "";
							undoData->stringData[1][terrain] = path;

							dt.normalTexture = Texture::load(Engine::getSingleton()->getAssetsPath(), path, true, Texture::CompressionMethod::Default, false);
						}
					}
				}

				ImGui::EndDragDropTarget();
			}

			ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal);

			ImGui::Text("World size");

			float prevWs = dt.worldSize;
			if (ImGui::InputFloat("##texture_world_size", &dt.worldSize))
			{
				//Undo
				UndoData* undoData = Undo::addUndo("Change terrain texture world size");
				undoData->floatData.resize(2);
				undoData->intData.resize(1);

				undoData->intData[0][nullptr] = editTexture;

				undoData->undoAction = [=](UndoData* data)
				{
					int idx = data->intData[0][nullptr];

					for (auto& d : data->floatData[0])
					{
						Terrain* terr = (Terrain*)d.first;
						auto& textures = terr->getTextures();
						TerrainTextureData& tex = textures[idx];
						tex.worldSize = d.second;
					}

					editMode = TerrainEditMode::PAINT_TEXTURE;
				};

				undoData->redoAction = [=](UndoData* data)
				{
					int idx = data->intData[0][nullptr];

					for (auto& d : data->floatData[1])
					{
						Terrain* terr = (Terrain*)d.first;
						auto& textures = terr->getTextures();
						TerrainTextureData& tex = textures[idx];
						tex.worldSize = d.second;
					}

					editMode = TerrainEditMode::PAINT_TEXTURE;
				};
				//

				undoData->floatData[0][terrain] = prevWs;
				undoData->floatData[1][terrain] = dt.worldSize;
			}

			if (ImGui::Button("Close"))
			{
				editTexture = -1;
			}
		}

		ImGui::End();
	}

	void TerrainEditor::grassEditorWindow()
	{
		if (ImGui::Begin("Edit grass data", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDocking))
		{
			Terrain* terrain = (Terrain*)components[0];

			std::vector<TerrainGrassData*>& list = terrain->getGrass();

			TerrainGrassData* dt = list.at(editGrass);

			Material* mat = dt->getMaterial();
			Texture* tex = nullptr;

			if (mat != nullptr)
			{
				std::vector<Uniform>& uniforms = mat->getUniforms();
				for (auto u = uniforms.begin(); u != uniforms.end(); ++u)
				{
					if (u->getType() == UniformType::Sampler2D)
					{
						tex = u->getValue<Sampler2DDef>().second;
						if (tex != nullptr)
							break;
					}
				}
			}

			if (tex == nullptr)
				tex = Texture::getNullTexture();

			if (ImGui::ImageButton((void*)tex->getHandle().idx, ImVec2(64, 64), ImVec2(0, 1), ImVec2(1, 0)))
			{
				if (mat != nullptr)
					MainWindow::getAssetsWindow()->focusOnFile(mat->getName());
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
							//Undo
							UndoData* undoData = Undo::addUndo("Change terrain grass material");
							undoData->stringData.resize(2);
							undoData->intData.resize(1);

							undoData->intData[0][nullptr] = editGrass;

							undoData->undoAction = [=](UndoData* data)
							{
								int idx = data->intData[0][nullptr];

								for (auto& d : data->stringData[0])
								{
									Terrain* terr = (Terrain*)d.first;
									auto& grass = terr->getGrass();
									TerrainGrassData* grs = grass[idx];
									grs->setMaterial(Material::load(Engine::getSingleton()->getAssetsPath(), d.second));
								}

								editMode = TerrainEditMode::PAINT_GRASS;
							};

							undoData->redoAction = [=](UndoData* data)
							{
								int idx = data->intData[0][nullptr];

								for (auto& d : data->stringData[1])
								{
									Terrain* terr = (Terrain*)d.first;
									auto& grass = terr->getGrass();
									TerrainGrassData* grs = grass[idx];
									grs->setMaterial(Material::load(Engine::getSingleton()->getAssetsPath(), d.second));
								}

								editMode = TerrainEditMode::PAINT_GRASS;
							};
							//

							std::string path = move_from->getPath();

							undoData->stringData[0][terrain] = dt->getMaterial() != nullptr ? dt->getMaterial()->getName() : "";
							undoData->stringData[1][terrain] = path;

							dt->setMaterial(Material::load(Engine::getSingleton()->getAssetsPath(), path));
						}
					}
				}

				ImGui::EndDragDropTarget();
			}

			if (dt->getMaterial() != nullptr)
				ImGui::Text(dt->getMaterial()->getName().c_str());
			else
				ImGui::Text("No material");

			ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal);

			ImGui::Text("Min size");

			glm::vec2 prevMinSize = dt->getMinSize();
			if (ImGui::InputFloat2("##grass_min_size", glm::value_ptr(dt->getMinSize())))
			{
				dt->setMinSize(dt->getMinSize());

				//Undo
				UndoData* undoData = Undo::addUndo("Change terrain grass min size");
				undoData->vec2Data.resize(2);
				undoData->intData.resize(1);

				undoData->intData[0][nullptr] = editGrass;

				undoData->undoAction = [=](UndoData* data)
				{
					int idx = data->intData[0][nullptr];

					for (auto& d : data->vec2Data[0])
					{
						Terrain* terr = (Terrain*)d.first;
						auto& grass = terr->getGrass();
						TerrainGrassData* grs = grass[idx];
						grs->setMinSize(d.second);
					}

					editMode = TerrainEditMode::PAINT_GRASS;
				};

				undoData->redoAction = [=](UndoData* data)
				{
					int idx = data->intData[0][nullptr];

					for (auto& d : data->vec2Data[1])
					{
						Terrain* terr = (Terrain*)d.first;
						auto& grass = terr->getGrass();
						TerrainGrassData* grs = grass[idx];
						grs->setMinSize(d.second);
					}

					editMode = TerrainEditMode::PAINT_GRASS;
				};
				//

				undoData->vec2Data[0][terrain] = prevMinSize;
				undoData->vec2Data[1][terrain] = dt->getMinSize();
			}

			ImGui::Text("Max size");

			glm::vec2 prevMaxSize = dt->getMaxSize();
			if (ImGui::InputFloat2("##grass_max_size", glm::value_ptr(dt->getMaxSize())))
			{
				dt->setMaxSize(dt->getMaxSize());

				//Undo
				UndoData* undoData = Undo::addUndo("Change terrain grass max size");
				undoData->vec2Data.resize(2);
				undoData->intData.resize(1);

				undoData->intData[0][nullptr] = editGrass;

				undoData->undoAction = [=](UndoData* data)
				{
					int idx = data->intData[0][nullptr];

					for (auto& d : data->vec2Data[0])
					{
						Terrain* terr = (Terrain*)d.first;
						auto& grass = terr->getGrass();
						TerrainGrassData* grs = grass[idx];
						grs->setMaxSize(d.second);
					}

					editMode = TerrainEditMode::PAINT_GRASS;
				};

				undoData->redoAction = [=](UndoData* data)
				{
					int idx = data->intData[0][nullptr];

					for (auto& d : data->vec2Data[1])
					{
						Terrain* terr = (Terrain*)d.first;
						auto& grass = terr->getGrass();
						TerrainGrassData* grs = grass[idx];
						grs->setMaxSize(d.second);
					}

					editMode = TerrainEditMode::PAINT_GRASS;
				};
				//

				undoData->vec2Data[0][terrain] = prevMaxSize;
				undoData->vec2Data[1][terrain] = dt->getMaxSize();
			}

			if (ImGui::Button("Close"))
			{
				editGrass = -1;
			}
		}

		ImGui::End();
	}

	void TerrainEditor::treeEditorWindow()
	{
		if (ImGui::Begin("Edit tree data", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDocking))
		{
			Terrain* terrain = (Terrain*)components[0];

			std::vector<TerrainTreeData*>& list = terrain->getTrees();

			TerrainTreeData* dt = list.at(editTree);

			if (!bgfx::isValid(dt->getImpostorTexture()))
				ImGui::ImageButton((void*)Texture::getNullTexture()->getHandle().idx, ImVec2(64, 64), ImVec2(0, 1), ImVec2(1, 0));
			else
				ImGui::ImageButton((void*)dt->getImpostorTexture().idx, ImVec2(64, 64), ImVec2(0, 1), ImVec2(1, 0));
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_TreeView"))
				{
					TreeNode* move_from = (TreeNode*)payload->Data;
					if (move_from->treeView->getTag() == "Assets")
					{
						std::string ext = IO::GetFileExtension(move_from->alias);
						std::vector<std::string>& formats = Engine::getModel3dFileFormats();
						if (std::find(formats.begin(), formats.end(), ext) != formats.end() || ext == "prefab")
						{
							//Undo
							UndoData* undoData = Undo::addUndo("Change terrain tree model");
							undoData->stringData.resize(2);
							undoData->intData.resize(1);

							undoData->intData[0][nullptr] = editTree;

							undoData->undoAction = [=](UndoData* data)
							{
								int idx = data->intData[0][nullptr];

								for (auto& d : data->stringData[0])
								{
									Terrain* terr = (Terrain*)d.first;
									auto& trees = terr->getTrees();
									TerrainTreeData* tree = trees[idx];
									tree->setName(d.second);
								}

								editMode = TerrainEditMode::PAINT_TREES;
							};

							undoData->redoAction = [=](UndoData* data)
							{
								int idx = data->intData[0][nullptr];

								for (auto& d : data->stringData[1])
								{
									Terrain* terr = (Terrain*)d.first;
									auto& trees = terr->getTrees();
									TerrainTreeData* tree = trees[idx];
									tree->setName(d.second);
								}

								editMode = TerrainEditMode::PAINT_TREES;
							};
							//

							std::string path = move_from->getPath();

							undoData->stringData[0][terrain] = dt->getName();
							undoData->stringData[1][terrain] = path;

							dt->setName(path);
						}
					}
				}

				ImGui::EndDragDropTarget();
			}

			ImGui::Text(dt->getName().c_str());

			ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal);

			ImGui::Text("Min scale");

			float prevMinScale = dt->getMinScale();
			if (ImGui::InputFloat("##mesh_min_scale", &dt->getMinScale()))
			{
				dt->setMinScale(dt->getMinScale());

				//Undo
				UndoData* undoData = Undo::addUndo("Change terrain tree min scale");
				undoData->floatData.resize(2);
				undoData->intData.resize(1);

				undoData->intData[0][nullptr] = editTree;

				undoData->undoAction = [=](UndoData* data)
				{
					int idx = data->intData[0][nullptr];

					for (auto& d : data->floatData[0])
					{
						Terrain* terr = (Terrain*)d.first;
						auto& trees = terr->getTrees();
						TerrainTreeData* tree = trees[idx];
						tree->setMinScale(d.second);
					}

					editMode = TerrainEditMode::PAINT_TREES;
				};

				undoData->redoAction = [=](UndoData* data)
				{
					int idx = data->intData[0][nullptr];

					for (auto& d : data->floatData[1])
					{
						Terrain* terr = (Terrain*)d.first;
						auto& trees = terr->getTrees();
						TerrainTreeData* tree = trees[idx];
						tree->setMinScale(d.second);
					}

					editMode = TerrainEditMode::PAINT_TREES;
				};
				//

				undoData->floatData[0][terrain] = prevMinScale;
				undoData->floatData[1][terrain] = dt->getMinScale();
			}

			ImGui::Text("Max scale");

			float prevMaxScale = dt->getMaxScale();
			if (ImGui::InputFloat("##mesh_max_scale", &dt->getMaxScale()))
			{
				dt->setMaxScale(dt->getMaxScale());

				//Undo
				UndoData* undoData = Undo::addUndo("Change terrain tree max scale");
				undoData->floatData.resize(2);
				undoData->intData.resize(1);

				undoData->intData[0][nullptr] = editTree;

				undoData->undoAction = [=](UndoData* data)
				{
					int idx = data->intData[0][nullptr];

					for (auto& d : data->floatData[0])
					{
						Terrain* terr = (Terrain*)d.first;
						auto& trees = terr->getTrees();
						TerrainTreeData* tree = trees[idx];
						tree->setMaxScale(d.second);
					}

					editMode = TerrainEditMode::PAINT_TREES;
				};

				undoData->redoAction = [=](UndoData* data)
				{
					int idx = data->intData[0][nullptr];

					for (auto& d : data->floatData[1])
					{
						Terrain* terr = (Terrain*)d.first;
						auto& trees = terr->getTrees();
						TerrainTreeData* tree = trees[idx];
						tree->setMaxScale(d.second);
					}

					editMode = TerrainEditMode::PAINT_TREES;
				};
				//

				undoData->floatData[0][terrain] = prevMaxScale;
				undoData->floatData[1][terrain] = dt->getMaxScale();
			}

			if (ImGui::Button("Close"))
			{
				editTree = -1;
			}
		}

		ImGui::End();
	}

	void TerrainEditor::meshEditorWindow()
	{
		if (ImGui::Begin("Edit detail mesh data", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDocking))
		{
			Terrain* terrain = (Terrain*)components[0];

			std::vector<TerrainDetailMeshData*>& list = terrain->getDetailMeshes();

			TerrainDetailMeshData* dt = list.at(editMesh);

			if (!bgfx::isValid(dt->getImpostorTexture()))
				ImGui::ImageButton((void*)Texture::getNullTexture()->getHandle().idx, ImVec2(64, 64), ImVec2(0, 1), ImVec2(1, 0));
			else
				ImGui::ImageButton((void*)dt->getImpostorTexture().idx, ImVec2(64, 64), ImVec2(0, 1), ImVec2(1, 0));

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_TreeView"))
				{
					TreeNode* move_from = (TreeNode*)payload->Data;
					if (move_from->treeView->getTag() == "Assets")
					{
						std::string ext = IO::GetFileExtension(move_from->alias);
						std::vector<std::string>& formats = Engine::getModel3dFileFormats();
						if (std::find(formats.begin(), formats.end(), ext) != formats.end() || ext == "prefab")
						{
							//Undo
							UndoData* undoData = Undo::addUndo("Change terrain detail mesh model");
							undoData->stringData.resize(2);
							undoData->intData.resize(1);

							undoData->intData[0][nullptr] = editMesh;

							undoData->undoAction = [=](UndoData* data)
							{
								int idx = data->intData[0][nullptr];

								for (auto& d : data->stringData[0])
								{
									Terrain* terr = (Terrain*)d.first;
									auto& meshes = terr->getDetailMeshes();
									TerrainDetailMeshData* mesh = meshes[idx];
									mesh->setName(d.second);
								}

								editMode = TerrainEditMode::PAINT_DETAIL_MESHES;
							};

							undoData->redoAction = [=](UndoData* data)
							{
								int idx = data->intData[0][nullptr];

								for (auto& d : data->stringData[1])
								{
									Terrain* terr = (Terrain*)d.first;
									auto& meshes = terr->getDetailMeshes();
									TerrainDetailMeshData* mesh = meshes[idx];
									mesh->setName(d.second);
								}

								editMode = TerrainEditMode::PAINT_DETAIL_MESHES;
							};
							//

							std::string path = move_from->getPath();

							undoData->stringData[0][terrain] = dt->getName();
							undoData->stringData[1][terrain] = path;

							dt->setName(path);
						}
					}
				}

				ImGui::EndDragDropTarget();
			}

			ImGui::Text(dt->getName().c_str());

			ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal);

			ImGui::Text("Min scale");

			float prevMinScale = dt->getMinScale();
			if (ImGui::InputFloat("##mesh_min_scale", &dt->getMinScale()))
			{
				dt->setMinScale(dt->getMinScale());

				//Undo
				UndoData* undoData = Undo::addUndo("Change terrain detail mesh min scale");
				undoData->floatData.resize(2);
				undoData->intData.resize(1);

				undoData->intData[0][nullptr] = editMesh;

				undoData->undoAction = [=](UndoData* data)
				{
					int idx = data->intData[0][nullptr];

					for (auto& d : data->floatData[0])
					{
						Terrain* terr = (Terrain*)d.first;
						auto& meshes = terr->getDetailMeshes();
						TerrainDetailMeshData* mesh = meshes[idx];
						mesh->setMinScale(d.second);
					}

					editMode = TerrainEditMode::PAINT_TREES;
				};

				undoData->redoAction = [=](UndoData* data)
				{
					int idx = data->intData[0][nullptr];

					for (auto& d : data->floatData[1])
					{
						Terrain* terr = (Terrain*)d.first;
						auto& meshes = terr->getDetailMeshes();
						TerrainDetailMeshData* mesh = meshes[idx];
						mesh->setMinScale(d.second);
					}

					editMode = TerrainEditMode::PAINT_TREES;
				};
				//

				undoData->floatData[0][terrain] = prevMinScale;
				undoData->floatData[1][terrain] = dt->getMinScale();
			}

			ImGui::Text("Max scale");

			float prevMaxScale = dt->getMaxScale();
			if (ImGui::InputFloat("##mesh_max_scale", &dt->getMaxScale()))
			{
				dt->setMaxScale(dt->getMaxScale());

				//Undo
				UndoData* undoData = Undo::addUndo("Change terrain detail mesh max scale");
				undoData->floatData.resize(2);
				undoData->intData.resize(1);

				undoData->intData[0][nullptr] = editMesh;

				undoData->undoAction = [=](UndoData* data)
				{
					int idx = data->intData[0][nullptr];

					for (auto& d : data->floatData[0])
					{
						Terrain* terr = (Terrain*)d.first;
						auto& meshes = terr->getDetailMeshes();
						TerrainDetailMeshData* mesh = meshes[idx];
						mesh->setMaxScale(d.second);
					}

					editMode = TerrainEditMode::PAINT_TREES;
				};

				undoData->redoAction = [=](UndoData* data)
				{
					int idx = data->intData[0][nullptr];

					for (auto& d : data->floatData[1])
					{
						Terrain* terr = (Terrain*)d.first;
						auto& meshes = terr->getDetailMeshes();
						TerrainDetailMeshData* mesh = meshes[idx];
						mesh->setMaxScale(d.second);
					}

					editMode = TerrainEditMode::PAINT_TREES;
				};
				//

				undoData->floatData[0][terrain] = prevMaxScale;
				undoData->floatData[1][terrain] = dt->getMaxScale();
			}

			if (ImGui::Button("Close"))
			{
				editMesh = -1;
			}
		}

		ImGui::End();
	}

	void TerrainEditor::massPlaceTreesWindow()
	{
		if (ImGui::Begin("Mass place trees", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDocking))
		{
			ImGui::Text("Number Of Trees");
			ImGui::InputInt("##numTrees", &massPlaceNumTrees);

			ImGui::Checkbox("##keepTrees", &massPlaceKeepTrees); ImGui::SameLine();
			ImGui::Text("Keep Existing Trees");

			ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal);

			if (ImGui::Button("Close"))
				massPlaceTreesWnd = false;

			ImGui::SameLine();

			if (ImGui::Button("Place"))
			{
				massPlaceTreesWnd = false;
				
				Terrain* terrain = (Terrain*)components[0];
				Transform* transform = terrain->getGameObject()->getTransform();

				std::vector<TerrainTreeData*>& list = terrain->getTrees();

				if (list.size() > 0)
				{
					for (int i = 0; i < massPlaceNumTrees; ++i)
					{
						float worldSize = terrain->getWorldSize();

						float offset = worldSize / 2.0f - terrain->getSize() / 2.0f;

						float rX = Mathf::RandomFloat(0, worldSize) - offset + transform->getPosition().x;
						float rY = Mathf::RandomFloat(0, worldSize) - offset + transform->getPosition().z;

						int rTree = Mathf::RandomInt(0, list.size() - 1);

						auto& treeData = list[rTree];
						treeData->addTree(glm::vec2(rX, rY), massPlaceNumTrees * 0.1f);
					}
				}
				else
				{
					Toast::showMessage("No trees added in terrain", ToastIcon::TI_WARNING);
				}
			}
		}

		ImGui::End();
	}

	void TerrainEditor::drawCircle(unsigned long long primitiveType, int viewId, Camera* camera, Color color, float radius)
	{
		Terrain* terrain = (Terrain*)components[0];

		int32_t terrainSize = (int32_t)terrain->getWorldSize();
		float* heightMap = terrain->getHeightMap();

		uint64_t state = BGFX_STATE_WRITE_RGB
			| BGFX_STATE_WRITE_A
			| BGFX_STATE_WRITE_Z
			| BGFX_STATE_DEPTH_TEST_ALWAYS
			| BGFX_STATE_BLEND_FUNC_SEPARATE(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA, BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_ONE)
			| primitiveType;

		std::vector<glm::vec3> vertices;

		glm::vec3 tp = terrain->getGameObject()->getTransform()->getPosition();
		glm::vec3 hitPos = lastHitPosWorld - tp;

		float height0 = terrain->getHeightAtWorldPos(hitPos + tp);
		float offset = 1.0f * terrain->getScale();

		int num_segments = 32;
		int tri = -1;
		for (int ii = 0; ii < num_segments + 1; ++ii)
		{
			++tri;
			float theta = 2.0f * 3.1415926f * float(ii) / float(num_segments);
			float _x = radius * cosf(theta);
			float _z = radius * sinf(theta);

			float brush_x = hitPos.x - _x;
			float brush_y = hitPos.z - _z;

			if (brush_x < -((terrainSize / 2.0f) - offset)) brush_x = -(terrainSize / 2.0f) + offset;
			if (brush_x >= (terrainSize / 2.0f)) brush_x = (terrainSize / 2.0f);
			if (brush_y < -(terrainSize / 2.0f)) brush_y = -terrainSize / 2.0f;
			if (brush_y >= ((terrainSize / 2.0f) - offset)) brush_y = (terrainSize / 2.0f) - offset;

			float height = (terrain->getHeightAtWorldPos(glm::vec3(brush_x, 0, brush_y) + tp) - 0.1f);

			vertices.push_back(glm::vec3(brush_x, height, brush_y));

			if (tri == 1)
			{
				if (primitiveType == BGFX_STATE_PT_TRISTRIP)
					vertices.push_back(glm::vec3(hitPos.x, height0, hitPos.z));

				tri = 0;
			}
		}

		if (vertices.size() > 0)
		{
			glm::mat4x4 trans = terrain->getGameObject()->getTransform()->getTransformMatrix();
			Primitives::mesh(trans, vertices, color, viewId, state, Renderer::getSingleton()->getSimpleProgram(), camera);
		}

		vertices.clear();
	}

	void TerrainEditor::drawBrushes(int viewId, int viewLayer, Camera* camera)
	{
		if (!isEditModeActive())
			return;

		if (components.size() == 0)
			return;

		Terrain* terrain = (Terrain*)components[0];
		if (terrain == nullptr)
			return;

		if (!terrain->getEnabled() || !terrain->getGameObject()->getActive())
			return;

		int overlayViewId = Renderer::getSingleton()->getOverlayViewId() + viewLayer;

		drawCircle(BGFX_STATE_PT_TRISTRIP, overlayViewId, camera, Color(0.301f, 0.905, 1.0f, 0.5f), brushSize);
		drawCircle(BGFX_STATE_PT_LINESTRIP | BGFX_STATE_LINEAA, overlayViewId, camera, Color(0.7f, 0.905, 1.0f, 0.8f), brushSize);
	}
}