#include "LightingWindow.h"

#include <imgui.h>
#include <imgui_internal.h>

#include "../Engine/Core/Engine.h"
#include "../Engine/Core/GameObject.h"
#include "../Engine/Classes/IO.h"
#include "../Engine/Renderer/Renderer.h"
#include "../Engine/Renderer/BatchedGeometry.h"
#include "../Engine/Assets/Material.h"
#include "../Engine/Assets/Scene.h"
#include "../Engine/Assets/Mesh.h"
#include "../Engine/Assets/Texture.h"
#include "../Engine/Components/MeshRenderer.h"
#include "../Classes/LightmapBaker.h"

#include "../Classes/TreeView.h"
#include "../Classes/TreeNode.h"
#include "../Dialogs/DialogProgress.h"
#include "../Classes/Undo.h"
#include "../Serialization/EditorSettings.h"

#include "../Engine/UI/ImGUIWidgets.h"

#include "MainWindow.h"
#include "AssetsWindow.h"

namespace GX
{
	LightingWindow::LightingWindow()
	{
		progressDialog = new DialogProgress();
		progressDialog->setTitle("Baking lightmaps");
	}

	LightingWindow::~LightingWindow()
	{
		delete progressDialog;
	}

	void LightingWindow::init()
	{
		EditorSettings* settings = MainWindow::getSettings();

		LightmapBaker::setGIBake(settings->giBake);
		LightmapBaker::setGIBounces(settings->giBounces);
		LightmapBaker::setGIIntensity(settings->giIntensity);
		LightmapBaker::setGIQuality(static_cast<GIQuality>(settings->giQuality));
		LightmapBaker::setLightmapSize(settings->lightmapSize);

		materialTexture = MainWindow::loadEditorIcon("Assets/material.png");
	}

	void LightingWindow::onSceneLoaded()
	{
		ambientColor = Renderer::getSingleton()->getAmbientColor();
		fogColor = Renderer::getSingleton()->getFogColor();
		fogDensity = Renderer::getSingleton()->getFogDensity();
		realtimeGIIntensity = Renderer::getSingleton()->getGIIntensity();
		bakedGIIntensity = LightmapBaker::getGIIntensity();
	}

	void LightingWindow::update()
	{
		if (opened)
		{
			if (ImGui::Begin("Lighting", &opened, ImGuiWindowFlags_NoCollapse))
			{
				//Sky model
				ImGui::BeginColumns(std::string("lightingWindowColumns0").c_str(), 2, ImGuiColumnsFlags_NoResize | ImGuiColumnsFlags_NoBorder);
				ImGui::Text("Sky model");
				ImGui::NextColumn();
				ImGui::PushItemWidth(-1);
				SkyModel skyModel = Renderer::getSingleton()->getSkyModel();
				std::string skyModelPrev = skyModel == SkyModel::Box ? "Box" : "Sphere";
				if (ImGui::BeginCombo("##sky_model", skyModelPrev.c_str()))
				{
					if (ImGui::Selectable("Box", skyModel == SkyModel::Box))
						onChangeSkyModel(static_cast<int>(skyModel), 0);

					if (ImGui::Selectable("Sphere", skyModel == SkyModel::Sphere))
						onChangeSkyModel(static_cast<int>(skyModel), 1);

					ImGui::EndCombo();
				}
				ImGui::PopItemWidth();
				ImGui::EndColumns();

				//Sky material
				ImGui::BeginColumns(std::string("lightingWindowColumns1").c_str(), 2, ImGuiColumnsFlags_NoResize | ImGuiColumnsFlags_NoBorder);
				ImGui::Text("Sky material");

				ImGui::NextColumn();

				Material* skyboxMaterial = Renderer::getSingleton()->getSkyMaterial();
				
				std::string mat = "None";
				if (skyboxMaterial != nullptr && skyboxMaterial->isLoaded())
					mat = IO::GetFileNameWithExt(skyboxMaterial->getName());

				ImVec2 avSize = ImGui::GetContentRegionAvail();

				if (ImGui::ImageButtonWithText((void*)materialTexture->getHandle().idx, mat.c_str(), ImVec2(16, 16), ImVec2(0, 1), ImVec2(1, 0), ImVec2(avSize.x * 0.7f, -1)))
				{
					if (skyboxMaterial != nullptr)
						MainWindow::getSingleton()->getAssetsWindow()->focusOnFile(skyboxMaterial->getName());
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

								Material* newMat = Material::load(Engine::getSingleton()->getAssetsPath(), path);
								onDropSkyboxMaterial(newMat);
							}
						}
					}
					ImGui::EndDragDropTarget();
				}

				ImGui::SameLine();
				if (ImGui::Button("Clear", ImVec2(avSize.x * 0.25f, 0.0f)))
				{
					onClearSkyboxMaterial();
				}
				ImGui::EndColumns();

				//Ambient light color
				ImGui::BeginColumns(std::string("lightingWindowColumns2").c_str(), 2, ImGuiColumnsFlags_NoResize | ImGuiColumnsFlags_NoBorder);
				ImGui::Text("Ambient light color");
				ImGui::NextColumn();
				ImGui::PushItemWidth(-1);
				ImGui::ColorEdit4("##ambient_light_color", ambientColor.ptr());
				if (ImGui::IsItemDeactivatedAfterEdit())
				{
					onChangeAmbientColor(Renderer::getSingleton()->getAmbientColor(), ambientColor);
				}
				ImGui::PopItemWidth();
				ImGui::EndColumns();

				ImGui::Dummy(ImVec2(20, 5));
				ImGui::Separator();
				ImGui::Dummy(ImVec2(20, 5));

				//Realtime GI
				ImGui::BeginColumns(std::string("lightingWindowColumnGI1").c_str(), 2, ImGuiColumnsFlags_NoResize | ImGuiColumnsFlags_NoBorder);
				ImGui::Text("Realtime GI");
				ImGui::NextColumn();
				ImGui::PushItemWidth(-1);
				bool prevGI = Renderer::getSingleton()->getGIEnabled();
				bool newGI = prevGI;
				if (ImGui::Checkbox("##enable_realtime_gi", &newGI))
				{
					onChangeRealtimeGI(prevGI, newGI);
				}
				ImGui::PopItemWidth();
				ImGui::EndColumns();

				if (Renderer::getSingleton()->getGIEnabled())
				{
					//Realtime GI intensity
					ImGui::BeginColumns(std::string("lightingWindowColumnGI2").c_str(), 2, ImGuiColumnsFlags_NoResize | ImGuiColumnsFlags_NoBorder);
					ImGui::Text("Realtime GI intensity");
					ImGui::NextColumn();
					ImGui::PushItemWidth(-1);
					ImGui::SliderFloat("##realtime_gi_intensity", &realtimeGIIntensity, 0.1f, 5.0f);
					if (ImGui::IsItemDeactivatedAfterEdit())
					{
						onChangeRealtimeGIIntensity(Renderer::getSingleton()->getGIIntensity(), realtimeGIIntensity);
					}
					ImGui::PopItemWidth();
					ImGui::EndColumns();

					ImGui::Dummy(ImVec2(20, 10));

					if (ImGui::Button("Update realtime GI"))
					{
						MainWindow::addOnEndUpdateCallback([=]() { Renderer::getSingleton()->updateEnvironmentMap(); });
					}
				}

				ImGui::Dummy(ImVec2(20, 5));
				ImGui::Separator();
				ImGui::Dummy(ImVec2(20, 5));

				//Fog
				ImGui::BeginColumns(std::string("lightingWindowColumnsFog1").c_str(), 2, ImGuiColumnsFlags_NoResize | ImGuiColumnsFlags_NoBorder);
				ImGui::Text("Enable fog");
				ImGui::NextColumn();
				ImGui::PushItemWidth(-1);
				bool prevFogVal = Renderer::getSingleton()->getFogEnabled();
				bool newFogVal = prevFogVal;
				if (ImGui::Checkbox("##enable_fog", &newFogVal))
				{
					onChangeFogEnabled(prevFogVal, newFogVal);
				}
				ImGui::PopItemWidth();
				ImGui::EndColumns();

				if (Renderer::getSingleton()->getFogEnabled())
				{
					ImGui::BeginColumns(std::string("lightingWindowColumnsFog2").c_str(), 2, ImGuiColumnsFlags_NoResize | ImGuiColumnsFlags_NoBorder);
					ImGui::Text("Fog color");
					ImGui::NextColumn();
					ImGui::PushItemWidth(-1);
					ImGui::ColorEdit4("##fog_color", fogColor.ptr());
					if (ImGui::IsItemDeactivatedAfterEdit())
					{
						onChangeFogColor(Renderer::getSingleton()->getFogColor(), fogColor);
					}
					ImGui::PopItemWidth();
					ImGui::EndColumns();

					ImGui::BeginColumns(std::string("lightingWindowColumnsFog3").c_str(), 2, ImGuiColumnsFlags_NoResize | ImGuiColumnsFlags_NoBorder);
					ImGui::Text("Fog start distance");
					ImGui::NextColumn();
					ImGui::PushItemWidth(-1);
					float prevFogDist = Renderer::getSingleton()->getFogStartDistance();
					float newFogDist = prevFogDist;
					if (ImGui::InputFloat("##fog_start_distance", &newFogDist))
					{
						onChangeFogStartDistance(prevFogDist, newFogDist);
					}
					ImGui::PopItemWidth();
					ImGui::EndColumns();

					ImGui::BeginColumns(std::string("lightingWindowColumnsFog4").c_str(), 2, ImGuiColumnsFlags_NoResize | ImGuiColumnsFlags_NoBorder);
					ImGui::Text("Fog end distance");
					ImGui::NextColumn();
					ImGui::PushItemWidth(-1);
					float prevFogDistEnd = Renderer::getSingleton()->getFogEndDistance();
					float newFogDistEnd = prevFogDistEnd;
					if (ImGui::InputFloat("##fog_end_distance", &newFogDistEnd))
					{
						onChangeFogEndDistance(prevFogDistEnd, newFogDistEnd);
					}
					ImGui::PopItemWidth();
					ImGui::EndColumns();

					ImGui::BeginColumns(std::string("lightingWindowColumnsFog5").c_str(), 2, ImGuiColumnsFlags_NoResize | ImGuiColumnsFlags_NoBorder);
					ImGui::Text("Fog density");
					ImGui::NextColumn();
					ImGui::PushItemWidth(-1);
					float prevFogDen = Renderer::getSingleton()->getFogDensity();
					float newFogDen = prevFogDen;
					ImGui::SliderFloat("##fog_density", &fogDensity, 0.0f, 1.0f);
					if (ImGui::IsItemDeactivatedAfterEdit())
					{
						onChangeFogDensity(Renderer::getSingleton()->getFogDensity(), fogDensity);
					}
					ImGui::PopItemWidth();
					ImGui::EndColumns();

					ImGui::BeginColumns(std::string("lightingWindowColumnsFog6").c_str(), 2, ImGuiColumnsFlags_NoResize | ImGuiColumnsFlags_NoBorder);
					ImGui::Text("Fog type");
					ImGui::NextColumn();
					ImGui::PushItemWidth(-1);
					int prevF = Renderer::getSingleton()->getFogType();
					std::string prevFText = "Linear";
					if (prevF == 1)
						prevFText = "Exponential";
					if (prevF == 2)
						prevFText = "Exponential squared";

					if (ImGui::BeginCombo("##fog_type", prevFText.c_str()))
					{
						if (ImGui::Selectable("Linear", prevF == 0))
							onChangeFogType(prevF, 0);

						if (ImGui::Selectable("Exponential", prevF == 1))
							onChangeFogType(prevF, 1);

						if (ImGui::Selectable("Exponential squared", prevF == 2))
							onChangeFogType(prevF, 2);

						ImGui::EndCombo();
					}
					ImGui::PopItemWidth();
					ImGui::EndColumns();

					ImGui::BeginColumns(std::string("lightingWindowColumnsFog7").c_str(), 2, ImGuiColumnsFlags_NoResize | ImGuiColumnsFlags_NoBorder);
					ImGui::Text("Include skybox");
					ImGui::NextColumn();
					ImGui::PushItemWidth(-1);
					bool prevFogSky = Renderer::getSingleton()->getFogIncludeSkybox();
					bool newFogSky = prevFogSky;
					if (ImGui::Checkbox("##include_skybox", &newFogSky))
					{
						onChangeFogIncludeSkybox(prevFogSky, newFogSky);
					}
					ImGui::PopItemWidth();
					ImGui::EndColumns();
				}

				ImGui::Dummy(ImVec2(20, 5));
				ImGui::Separator();
				ImGui::Dummy(ImVec2(20, 5));

				//Lightmaps
				ImGui::BeginColumns(std::string("lightingWindowColumns3").c_str(), 2, ImGuiColumnsFlags_NoResize | ImGuiColumnsFlags_NoBorder);
				ImGui::Text("Default lightmap size");
				ImGui::NextColumn();
				int current = LightmapBaker::getLightmapSize();
				ImGui::PushItemWidth(-1);
				if (ImGui::BeginCombo("##default_lightmap_size", std::to_string(current).c_str()))
				{
					for (int n = 0; n < lightmapSizes.size(); n++)
					{
						bool is_selected = (current == lightmapSizes[n]);

						if (ImGui::Selectable(std::to_string(lightmapSizes[n]).c_str(), is_selected))
						{
							current = lightmapSizes[n];
							defaultLightmapSize = current;

							int prevSize = LightmapBaker::getLightmapSize();
							LightmapBaker::setLightmapSize(current);

							onChangeLightmapSize(prevSize, current);
						}

						if (is_selected)
							ImGui::SetItemDefaultFocus();
					}

					ImGui::EndCombo();
				}
				ImGui::PopItemWidth();
				ImGui::EndColumns();

				//GI bake
				ImGui::BeginColumns(std::string("lightingWindowColumns4").c_str(), 2, ImGuiColumnsFlags_NoResize | ImGuiColumnsFlags_NoBorder);
				ImGui::Text("Baked GI");
				ImGui::NextColumn();
				ImGui::PushItemWidth(-1);

				bool prevBake = LightmapBaker::getGIBake();
				bool _bake = prevBake;
				if (ImGui::Checkbox("##gi_bake", &_bake))
				{
					onChangeGIBake(prevBake, _bake);
				}
				ImGui::PopItemWidth();
				ImGui::EndColumns();

				if (prevBake)
				{
					//GI bounces
					ImGui::BeginColumns(std::string("lightingWindowColumns5").c_str(), 2, ImGuiColumnsFlags_NoResize | ImGuiColumnsFlags_NoBorder);
					ImGui::Text("Baked GI bounces");
					ImGui::NextColumn();
					ImGui::PushItemWidth(-1);

					int prevB = LightmapBaker::getGIBounces();
					std::string prevBText = "1";
					if (prevB == 2)
						prevBText = "2";
					if (prevB == 3)
						prevBText = "3";
					if (prevB == 4)
						prevBText = "4";
					if (prevB == 5)
						prevBText = "5";

					if (ImGui::BeginCombo("##gi_bounces", prevBText.c_str()))
					{
						if (ImGui::Selectable("1", prevB == 1))
							onChangeGIBounces(prevB, 1);

						if (ImGui::Selectable("2", prevB == 2))
							onChangeGIBounces(prevB, 2);

						if (ImGui::Selectable("3", prevB == 3))
							onChangeGIBounces(prevB, 3);

						if (ImGui::Selectable("4", prevB == 4))
							onChangeGIBounces(prevB, 4);

						if (ImGui::Selectable("5", prevB == 5))
							onChangeGIBounces(prevB, 5);

						ImGui::EndCombo();
					}
					ImGui::PopItemWidth();
					ImGui::EndColumns();

					//GI intensity
					ImGui::BeginColumns(std::string("lightingWindowColumns6").c_str(), 2, ImGuiColumnsFlags_NoResize | ImGuiColumnsFlags_NoBorder);
					ImGui::Text("Baked GI intensity");
					ImGui::NextColumn();
					ImGui::PushItemWidth(-1);
					ImGui::SliderFloat("##gi_intensity", &bakedGIIntensity, 0.1f, 3.0f, "%.2f");
					if (ImGui::IsItemDeactivatedAfterEdit())
					{
						onChangeGIIntensity(LightmapBaker::getGIIntensity(), bakedGIIntensity);
					}
					ImGui::PopItemWidth();
					ImGui::EndColumns();

					//GI quality
					ImGui::BeginColumns(std::string("lightingWindowColumns7").c_str(), 2, ImGuiColumnsFlags_NoResize | ImGuiColumnsFlags_NoBorder);
					ImGui::Text("Baked GI quality");
					ImGui::NextColumn();
					ImGui::PushItemWidth(-1);

					GIQuality prevGI = LightmapBaker::getGIQuality();
					std::string prevGIText = "Low";
					if (prevGI == GIQuality::Normal)
						prevGIText = "Normal";
					if (prevGI == GIQuality::High)
						prevGIText = "High";
					if (prevGI == GIQuality::VeryHigh)
						prevGIText = "Very High";

					if (ImGui::BeginCombo("##gi_quality", prevGIText.c_str()))
					{
						if (ImGui::Selectable("Low", prevGI == GIQuality::Low))
							onChangeGIQuality(static_cast<int>(prevGI), 0);

						if (ImGui::Selectable("Normal", prevGI == GIQuality::Normal))
							onChangeGIQuality(static_cast<int>(prevGI), 1);

						if (ImGui::Selectable("High", prevGI == GIQuality::High))
							onChangeGIQuality(static_cast<int>(prevGI), 2);

						if (ImGui::Selectable("Very High", prevGI == GIQuality::VeryHigh))
							onChangeGIQuality(static_cast<int>(prevGI), 3);

						ImGui::EndCombo();
					}
					ImGui::PopItemWidth();
					ImGui::EndColumns();
				}

				ImGui::Dummy(ImVec2(20, 10));

				if (ImGui::Button("Rebuild static geometry"))
				{
					MainWindow::addOnEndUpdateCallback([=]() { BatchedGeometry::getSingleton()->rebuild(true); });
				}

				ImGui::Dummy(ImVec2(20, 10));
				ImGui::Separator();
				ImGui::Dummy(ImVec2(20, 10));

				if (ImGui::Button("Clear baked data"))
				{
					MainWindow::addOnEndUpdateCallback([=]() { clearBakedData(); });
				}

				ImGui::SameLine();

				if (ImGui::Button("Bake lighting"))
				{
					MainWindow::addOnEndUpdateCallback([=]() { bakeLighting(); });
				}
			}

			ImGui::End();
		}
	}

	void LightingWindow::onDropSkyboxMaterial(Material* newMat)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change sky material");
		undoData->stringData.resize(4);

		undoData->undoAction = [=](UndoData* data)
		{
			Material* mat = Material::load(data->stringData[0][nullptr], data->stringData[1][nullptr]);
			Renderer::getSingleton()->setSkyMaterial(mat);
		};

		undoData->redoAction = [=](UndoData* data)
		{
			Material* mat = Material::load(data->stringData[2][nullptr], data->stringData[3][nullptr]);
			Renderer::getSingleton()->setSkyMaterial(mat);
		};
		//

		Material* material = Renderer::getSingleton()->getSkyMaterial();
		undoData->stringData[0][nullptr] = material != nullptr ? material->getLocation() : "";
		undoData->stringData[1][nullptr] = material != nullptr ? material->getName() : "";
		undoData->stringData[2][nullptr] = newMat != nullptr ? newMat->getLocation() : "";
		undoData->stringData[3][nullptr] = newMat != nullptr ? newMat->getName() : "";

		Renderer::getSingleton()->setSkyMaterial(newMat);
	}

	void LightingWindow::onClearSkyboxMaterial()
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change skybox material");
		undoData->stringData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			Material* mat = Material::load(data->stringData[0][nullptr], data->stringData[1][nullptr]);
			Renderer::getSingleton()->setSkyMaterial(mat);
		};

		undoData->redoAction = [=](UndoData* data)
		{
			Renderer::getSingleton()->setSkyMaterial(nullptr);
		};
		//

		Material* material = Renderer::getSingleton()->getSkyMaterial();
		undoData->stringData[0][nullptr] = material != nullptr ? material->getLocation() : "";
		undoData->stringData[1][nullptr] = material != nullptr ? material->getName() : "";

		Renderer::getSingleton()->setSkyMaterial(nullptr);
	}

	void LightingWindow::onChangeAmbientColor(Color prevColor, Color newColor)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change ambient color");
		undoData->vec4Data.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			glm::vec4 val = data->vec4Data[0][nullptr];
			Renderer::getSingleton()->setAmbientColor(Color(val.x, val.y, val.z, val.w));
			ambientColor = Color(val.x, val.y, val.z, val.w);
		};

		undoData->redoAction = [=](UndoData* data)
		{
			glm::vec4 val = data->vec4Data[1][nullptr];
			Renderer::getSingleton()->setAmbientColor(Color(val.x, val.y, val.z, val.w));
			ambientColor = Color(val.x, val.y, val.z, val.w);
		};
		//

		undoData->vec4Data[0][nullptr] = glm::vec4(prevColor[0], prevColor[1], prevColor[2], prevColor[3]);
		undoData->vec4Data[1][nullptr] = glm::vec4(newColor[0], newColor[1], newColor[2], newColor[3]);

		Renderer::getSingleton()->setAmbientColor(newColor);
	}

	void LightingWindow::onChangeLightmapSize(int prevSize, int newSize)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change default lightmap size");
		undoData->intData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			LightmapBaker::setLightmapSize(data->intData[0][nullptr]);
		};

		undoData->redoAction = [=](UndoData* data)
		{
			LightmapBaker::setLightmapSize(data->intData[1][nullptr]);
		};
		//

		undoData->intData[0][nullptr] = prevSize;
		undoData->intData[1][nullptr] = newSize;
	}

	void LightingWindow::onChangeSkyModel(int prevValue, int newValue)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change sky model");
		undoData->intData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			Renderer::getSingleton()->setSkyModel(static_cast<SkyModel>(data->intData[0][nullptr]));
		};

		undoData->redoAction = [=](UndoData* data)
		{
			Renderer::getSingleton()->setSkyModel(static_cast<SkyModel>(data->intData[1][nullptr]));
		};
		//

		undoData->intData[0][nullptr] = prevValue;
		undoData->intData[1][nullptr] = newValue;

		Renderer::getSingleton()->setSkyModel(static_cast<SkyModel>(newValue));
	}

	void LightingWindow::onChangeRealtimeGI(bool prevValue, bool newValue)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change realtime GI enabled");
		undoData->boolData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			Renderer::getSingleton()->setGIEnabled(data->boolData[0][nullptr]);
		};

		undoData->redoAction = [=](UndoData* data)
		{
			Renderer::getSingleton()->setGIEnabled(data->boolData[1][nullptr]);
		};
		//

		undoData->boolData[0][nullptr] = prevValue;
		undoData->boolData[1][nullptr] = newValue;

		Renderer::getSingleton()->setGIEnabled(newValue);
	}

	void LightingWindow::onChangeRealtimeGIIntensity(float prevValue, float newValue)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change realtime GI intensity");
		undoData->floatData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			Renderer::getSingleton()->setGIIntensity(data->floatData[0][nullptr]);
			realtimeGIIntensity = data->floatData[0][nullptr];
		};

		undoData->redoAction = [=](UndoData* data)
		{
			Renderer::getSingleton()->setGIIntensity(data->floatData[1][nullptr]);
			realtimeGIIntensity = data->floatData[1][nullptr];
		};
		//

		undoData->floatData[0][nullptr] = prevValue;
		undoData->floatData[1][nullptr] = newValue;

		Renderer::getSingleton()->setGIIntensity(newValue);
	}

	void LightingWindow::onChangeGIBake(bool prevValue, bool newValue)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Enable GI bake");
		undoData->boolData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			LightmapBaker::setGIBake(data->boolData[0][nullptr]);
		};

		undoData->redoAction = [=](UndoData* data)
		{
			LightmapBaker::setGIBake(data->boolData[1][nullptr]);
		};
		//

		undoData->boolData[0][nullptr] = prevValue;
		undoData->boolData[1][nullptr] = newValue;

		LightmapBaker::setGIBake(newValue);
	}

	void LightingWindow::onChangeGIBounces(int prevValue, int newValue)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change GI bounces");
		undoData->intData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			LightmapBaker::setGIBounces(data->intData[0][nullptr]);
		};

		undoData->redoAction = [=](UndoData* data)
		{
			LightmapBaker::setGIBounces(data->intData[1][nullptr]);
		};
		//

		undoData->intData[0][nullptr] = prevValue;
		undoData->intData[1][nullptr] = newValue;

		LightmapBaker::setGIBounces(newValue);
	}

	void LightingWindow::onChangeGIIntensity(float prevValue, float newValue)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change GI intensity");
		undoData->floatData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			LightmapBaker::setGIIntensity(data->floatData[0][nullptr]);
			bakedGIIntensity = data->floatData[0][nullptr];
		};

		undoData->redoAction = [=](UndoData* data)
		{
			LightmapBaker::setGIIntensity(data->floatData[1][nullptr]);
			bakedGIIntensity = data->floatData[1][nullptr];
		};
		//

		undoData->floatData[0][nullptr] = prevValue;
		undoData->floatData[1][nullptr] = newValue;

		LightmapBaker::setGIIntensity(newValue);
	}

	void LightingWindow::onChangeGIQuality(int prevValue, int newValue)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change GI quality");
		undoData->intData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			LightmapBaker::setGIQuality(static_cast<GIQuality>(data->intData[0][nullptr]));
		};

		undoData->redoAction = [=](UndoData* data)
		{
			LightmapBaker::setGIQuality(static_cast<GIQuality>(data->intData[1][nullptr]));
		};
		//

		undoData->intData[0][nullptr] = prevValue;
		undoData->intData[1][nullptr] = newValue;

		LightmapBaker::setGIQuality(static_cast<GIQuality>(newValue));
	}

	void LightingWindow::onChangeFogEnabled(bool prevValue, bool newValue)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change fog enabled");
		undoData->boolData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			Renderer::getSingleton()->setFogEnabled(data->boolData[0][nullptr]);
		};

		undoData->redoAction = [=](UndoData* data)
		{
			Renderer::getSingleton()->setFogEnabled(data->boolData[1][nullptr]);
		};
		//

		undoData->boolData[0][nullptr] = prevValue;
		undoData->boolData[1][nullptr] = newValue;

		Renderer::getSingleton()->setFogEnabled(newValue);
	}

	void LightingWindow::onChangeFogIncludeSkybox(bool prevValue, bool newValue)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change fog include skybox");
		undoData->boolData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			Renderer::getSingleton()->setFogIncludeSkybox(data->boolData[0][nullptr]);
		};

		undoData->redoAction = [=](UndoData* data)
		{
			Renderer::getSingleton()->setFogIncludeSkybox(data->boolData[1][nullptr]);
		};
		//

		undoData->boolData[0][nullptr] = prevValue;
		undoData->boolData[1][nullptr] = newValue;

		Renderer::getSingleton()->setFogIncludeSkybox(newValue);
	}

	void LightingWindow::onChangeFogStartDistance(float prevValue, float newValue)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change fog start distance");
		undoData->floatData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			Renderer::getSingleton()->setFogStartDistance(data->floatData[0][nullptr]);
		};

		undoData->redoAction = [=](UndoData* data)
		{
			Renderer::getSingleton()->setFogStartDistance(data->floatData[1][nullptr]);
		};
		//

		undoData->floatData[0][nullptr] = prevValue;
		undoData->floatData[1][nullptr] = newValue;

		Renderer::getSingleton()->setFogStartDistance(newValue);
	}

	void LightingWindow::onChangeFogEndDistance(float prevValue, float newValue)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change fog end distance");
		undoData->floatData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			Renderer::getSingleton()->setFogEndDistance(data->floatData[0][nullptr]);
		};

		undoData->redoAction = [=](UndoData* data)
		{
			Renderer::getSingleton()->setFogEndDistance(data->floatData[1][nullptr]);
		};
		//

		undoData->floatData[0][nullptr] = prevValue;
		undoData->floatData[1][nullptr] = newValue;

		Renderer::getSingleton()->setFogEndDistance(newValue);
	}

	void LightingWindow::onChangeFogDensity(float prevValue, float newValue)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change fog density");
		undoData->floatData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			Renderer::getSingleton()->setFogDensity(data->floatData[0][nullptr]);
			fogDensity = data->floatData[0][nullptr];
		};

		undoData->redoAction = [=](UndoData* data)
		{
			Renderer::getSingleton()->setFogDensity(data->floatData[1][nullptr]);
			fogDensity = data->floatData[1][nullptr];
		};
		//

		undoData->floatData[0][nullptr] = prevValue;
		undoData->floatData[1][nullptr] = newValue;

		Renderer::getSingleton()->setFogDensity(newValue);
	}

	void LightingWindow::onChangeFogColor(Color prevValue, Color newValue)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change fog color");
		undoData->vec4Data.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			glm::vec4 color = data->vec4Data[0][nullptr];
			Renderer::getSingleton()->setFogColor(Color(color.x, color.y, color.z, color.w));
			fogColor = Color(color.x, color.y, color.z, color.w);
		};

		undoData->redoAction = [=](UndoData* data)
		{
			glm::vec4 color = data->vec4Data[1][nullptr];
			Renderer::getSingleton()->setFogColor(Color(color.x, color.y, color.z, color.w));
			fogColor = Color(color.x, color.y, color.z, color.w);
		};
		//

		glm::vec4 prevColor = glm::vec4(prevValue[0], prevValue[1], prevValue[2], prevValue[3]);
		glm::vec4 newColor = glm::vec4(newValue[0], newValue[1], newValue[2], newValue[3]);

		undoData->vec4Data[0][nullptr] = prevColor;
		undoData->vec4Data[1][nullptr] = newColor;

		Renderer::getSingleton()->setFogColor(newValue);
	}

	void LightingWindow::onChangeFogType(int prevValue, int newValue)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change fog type");
		undoData->intData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			Renderer::getSingleton()->setFogType(data->intData[0][nullptr]);
		};

		undoData->redoAction = [=](UndoData* data)
		{
			Renderer::getSingleton()->setFogType(data->intData[1][nullptr]);
		};
		//

		undoData->intData[0][nullptr] = prevValue;
		undoData->intData[1][nullptr] = newValue;

		Renderer::getSingleton()->setFogType(newValue);
	}

	void LightingWindow::clearBakedData()
	{
		std::string loadedScene = Scene::getLoadedScene();
		std::string dirPath = Engine::getSingleton()->getAssetsPath() + IO::GetFilePath(loadedScene) + IO::GetFileName(loadedScene) + "/Lightmaps/";
		if (IO::DirExists(dirPath))
			IO::DirDeleteRecursive(dirPath);

		MainWindow::getSingleton()->onRestore();

		auto gameObjects = Engine::getSingleton()->getGameObjects();
		for (auto& gameObject : gameObjects)
		{
			if (!gameObject->getLightingStatic() || gameObject->getBatchingStatic())
				continue;

			MeshRenderer* renderer = (MeshRenderer*)gameObject->getComponent(MeshRenderer::COMPONENT_TYPE);
			if (renderer == nullptr)
				continue;

			renderer->reloadLightmaps();
		}

		BatchedGeometry::getSingleton()->reloadLightmaps();
	}

	void LightingWindow::bakeLighting()
	{
		MainWindow::addOnEndUpdateCallback([=]()
			{
				BatchedGeometry::getSingleton()->rebuild(true);
			
				clearBakedData();

				progressDialog->show();
				progressDialog->setTitle("Baking lighting");

				LightmapBaker::bakeAll([=](float p, std::string text)
					{
						progressDialog->setProgress(p);
						progressDialog->setStatusText(text);
					}
				);

				progressDialog->hide();

				MainWindow::getSingleton()->onRestore();
			}
		);
	}
}