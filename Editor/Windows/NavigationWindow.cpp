#include "NavigationWindow.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <dear-imgui/misc/cpp/imgui_stdlib.h>
#include "../Engine/glm/gtc/type_ptr.hpp"

#include "../Windows/MainWindow.h"
#include "../Windows/AssetsWindow.h"
#include "../Windows/SceneWindow.h"
#include "../Dialogs/DialogProgress.h"

#include "../Engine/Core/Engine.h"
#include "../Engine/Renderer/Renderer.h"
#include "../Engine/Renderer/Primitives.h"
#include "../Engine/Renderer/VertexLayouts.h"
#include "../Engine/Components/Camera.h"
#include "../Engine/Core/NavigationManager.h"
#include "../Engine/Classes/IO.h"
#include "../Engine/Assets/Texture.h"
#include "../Serialization/EditorSettings.h"
#include "../Classes/Undo.h"

namespace GX
{
	NavigationWindow::NavigationWindow()
	{
		progressDialog = new DialogProgress();
		progressDialog->setTitle("Baking NavMesh");
	}

	NavigationWindow::~NavigationWindow()
	{
		delete progressDialog;

		if (bgfx::isValid(vbh))
			bgfx::destroy(vbh);

		vbh = { bgfx::kInvalidHandle };
	}

	void NavigationWindow::init()
	{
		Renderer::getSingleton()->addRenderCallback([=](int viewId, int viewLayer, Camera* camera) { drawNavMesh(viewId, viewLayer, camera); }, 1);
	}

	void NavigationWindow::update()
	{
		if (opened)
		{
			if (ImGui::Begin("Navigation", &opened, ImGuiWindowFlags_NoCollapse))
			{
				NavigationManager* navMgr = NavigationManager::getSingleton();

				ImGui::BeginColumns(std::string("navWindowColumns1").c_str(), 2, ImGuiColumnsFlags_NoResize | ImGuiColumnsFlags_NoBorder);
				ImGui::Text("Walkable slope angle");
				ImGui::NextColumn();
				ImGui::PushItemWidth(-1);
				float prevVal = navMgr->getWalkableSlopeAngle();
				if (ImGui::InputFloat("##walkable_slope_angle", &navMgr->getWalkableSlopeAngle(), 0, 0, "%.5f"))
				{
					onChangeWalkableSlopeAngle(prevVal, navMgr->getWalkableSlopeAngle());
				}
				ImGui::PopItemWidth();
				ImGui::EndColumns();

				ImGui::BeginColumns(std::string("navWindowColumns2").c_str(), 2, ImGuiColumnsFlags_NoResize | ImGuiColumnsFlags_NoBorder);
				ImGui::Text("Walkable height");
				ImGui::NextColumn();
				ImGui::PushItemWidth(-1);
				prevVal = navMgr->getWalkableHeight();
				if (ImGui::InputFloat("##walkable_height", &navMgr->getWalkableHeight(), 0, 0, "%.5f"))
				{
					onChangeWalkableHeight(prevVal, navMgr->getWalkableHeight());
				}
				ImGui::PopItemWidth();
				ImGui::EndColumns();

				ImGui::BeginColumns(std::string("navWindowColumns3").c_str(), 2, ImGuiColumnsFlags_NoResize | ImGuiColumnsFlags_NoBorder);
				ImGui::Text("Walkable climb");
				ImGui::NextColumn();
				ImGui::PushItemWidth(-1);
				prevVal = navMgr->getWalkableClimb();
				if (ImGui::InputFloat("##walkable_climb", &navMgr->getWalkableClimb(), 0, 0, "%.5f"))
				{
					onChangeWalkableClimb(prevVal, navMgr->getWalkableClimb());
				}
				ImGui::PopItemWidth();
				ImGui::EndColumns();

				ImGui::BeginColumns(std::string("navWindowColumns4").c_str(), 2, ImGuiColumnsFlags_NoResize | ImGuiColumnsFlags_NoBorder);
				ImGui::Text("Walkable radius");
				ImGui::NextColumn();
				ImGui::PushItemWidth(-1);
				prevVal = navMgr->getWalkableRadius();
				if (ImGui::InputFloat("##walkable_radius", &navMgr->getWalkableRadius(), 0, 0, "%.5f"))
				{
					onChangeWalkableRadius(prevVal, navMgr->getWalkableRadius());
				}
				ImGui::PopItemWidth();
				ImGui::EndColumns();

				ImGui::BeginColumns(std::string("navWindowColumns5").c_str(), 2, ImGuiColumnsFlags_NoResize | ImGuiColumnsFlags_NoBorder);
				ImGui::Text("Max edge length");
				ImGui::NextColumn();
				ImGui::PushItemWidth(-1);
				prevVal = navMgr->getMaxEdgeLen();
				if (ImGui::InputFloat("##max_edge_length", &navMgr->getMaxEdgeLen(), 0, 0, "%.5f"))
				{
					onChangeMaxEdgeLength(prevVal, navMgr->getMaxEdgeLen());
				}
				ImGui::PopItemWidth();
				ImGui::EndColumns();

				ImGui::BeginColumns(std::string("navWindowColumns6").c_str(), 2, ImGuiColumnsFlags_NoResize | ImGuiColumnsFlags_NoBorder);
				ImGui::Text("Max simplification error");
				ImGui::NextColumn();
				ImGui::PushItemWidth(-1);
				prevVal = navMgr->getMaxSimplificationError();
				if (ImGui::InputFloat("##max_simplification_error", &navMgr->getMaxSimplificationError(), 0, 0, "%.5f"))
				{
					onChangeMaxSimplificationError(prevVal, navMgr->getMaxSimplificationError());
				}
				ImGui::PopItemWidth();
				ImGui::EndColumns();

				ImGui::BeginColumns(std::string("navWindowColumns7").c_str(), 2, ImGuiColumnsFlags_NoResize | ImGuiColumnsFlags_NoBorder);
				ImGui::Text("Min region area");
				ImGui::NextColumn();
				ImGui::PushItemWidth(-1);
				prevVal = navMgr->getMinRegionArea();
				if (ImGui::InputFloat("##min_region_area", &navMgr->getMinRegionArea(), 0, 0, "%.5f"))
				{
					onChangeMinRegionArea(prevVal, navMgr->getMinRegionArea());
				}
				ImGui::PopItemWidth();
				ImGui::EndColumns();

				ImGui::BeginColumns(std::string("navWindowColumns8").c_str(), 2, ImGuiColumnsFlags_NoResize | ImGuiColumnsFlags_NoBorder);
				ImGui::Text("Merge region area");
				ImGui::NextColumn();
				ImGui::PushItemWidth(-1);
				prevVal = navMgr->getMergeRegionArea();
				if (ImGui::InputFloat("##merge_region_area", &navMgr->getMergeRegionArea(), 0, 0, "%.5f"))
				{
					onChangeMergeRegionArea(prevVal, navMgr->getMergeRegionArea());
				}
				ImGui::PopItemWidth();
				ImGui::EndColumns();

				ImGui::BeginColumns(std::string("navWindowColumns9").c_str(), 2, ImGuiColumnsFlags_NoResize | ImGuiColumnsFlags_NoBorder);
				ImGui::Text("Cell size");
				ImGui::NextColumn();
				ImGui::PushItemWidth(-1);
				prevVal = navMgr->getCellSize();
				if (ImGui::InputFloat("##cell_size", &navMgr->getCellSize(), 0, 0, "%.5f"))
				{
					onChangeCellSize(prevVal, navMgr->getCellSize());
				}
				ImGui::PopItemWidth();
				ImGui::EndColumns();

				ImGui::BeginColumns(std::string("navWindowColumns10").c_str(), 2, ImGuiColumnsFlags_NoResize | ImGuiColumnsFlags_NoBorder);
				ImGui::Text("Cell height");
				ImGui::NextColumn();
				ImGui::PushItemWidth(-1);
				prevVal = navMgr->getCellHeight();
				if (ImGui::InputFloat("##cell_height", &navMgr->getCellHeight(), 0, 0, "%.5f"))
				{
					onChangeCellHeight(prevVal, navMgr->getCellHeight());
				}
				ImGui::PopItemWidth();
				ImGui::EndColumns();

				if (ImGui::Button("Clear NavMesh"))
				{
					std::string loadedScene = navMgr->getLoadedScene();

					std::string sName = IO::GetFileName(loadedScene);
					std::string sPath = IO::GetFilePath(loadedScene);

					if (!loadedScene.empty())
					{
						std::string _navMeshPath = Engine::getSingleton()->getAssetsPath() + sPath + "/" + sName + "/" + sName + ".navmesh";

						if (IO::FileExists(_navMeshPath))
							IO::FileDelete(_navMeshPath);
					}

					navMgr->cleanup();
				}

				ImGui::SameLine();

				if (ImGui::Button("Build NavMesh"))
				{
					prevVerticesCount = 0;
					
					MainWindow::addOnEndUpdateCallback([=]()
						{
							progressDialog->show();

							navMgr->buildNavMesh([=](int progress, int totalIter, int currentIter)
								{
									progressDialog->setStatusText("Baking NavMesh... " + std::to_string(progress) + "%% (" + std::to_string(currentIter) + "/" + std::to_string(totalIter) + " tiles)");
									progressDialog->setProgress((float)progress / 100.0f);
								}
							);

							progressDialog->hide();

							MainWindow::getSingleton()->getAssetsWindow()->reloadFiles();
						}
					);
				}
			}

			ImGui::End();
			//progressDialog->update();
		}
	}

	void NavigationWindow::onChangeWalkableSlopeAngle(float prevVal, float newVal)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change navigation walkable slope angle");
		undoData->floatData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			NavigationManager* navMgr = NavigationManager::getSingleton();
			navMgr->setWalkableSlopeAngle(data->floatData[0][nullptr]);
		};

		undoData->redoAction = [=](UndoData* data)
		{
			NavigationManager* navMgr = NavigationManager::getSingleton();
			navMgr->setWalkableSlopeAngle(data->floatData[1][nullptr]);
		};
		//

		undoData->floatData[0][nullptr] = prevVal;
		undoData->floatData[1][nullptr] = newVal;
	}

	void NavigationWindow::onChangeWalkableHeight(float prevVal, float newVal)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change navigation walkable height");
		undoData->floatData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			NavigationManager* navMgr = NavigationManager::getSingleton();
			navMgr->setWalkableHeight(data->floatData[0][nullptr]);
		};

		undoData->redoAction = [=](UndoData* data)
		{
			NavigationManager* navMgr = NavigationManager::getSingleton();
			navMgr->setWalkableHeight(data->floatData[1][nullptr]);
		};
		//

		undoData->floatData[0][nullptr] = prevVal;
		undoData->floatData[1][nullptr] = newVal;
	}

	void NavigationWindow::onChangeWalkableClimb(float prevVal, float newVal)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change navigation walkable climb");
		undoData->floatData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			NavigationManager* navMgr = NavigationManager::getSingleton();
			navMgr->setWalkableClimb(data->floatData[0][nullptr]);
		};

		undoData->redoAction = [=](UndoData* data)
		{
			NavigationManager* navMgr = NavigationManager::getSingleton();
			navMgr->setWalkableClimb(data->floatData[1][nullptr]);
		};
		//

		undoData->floatData[0][nullptr] = prevVal;
		undoData->floatData[1][nullptr] = newVal;
	}

	void NavigationWindow::onChangeWalkableRadius(float prevVal, float newVal)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change navigation walkable radius");
		undoData->floatData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			NavigationManager* navMgr = NavigationManager::getSingleton();
			navMgr->setWalkableRadius(data->floatData[0][nullptr]);
		};

		undoData->redoAction = [=](UndoData* data)
		{
			NavigationManager* navMgr = NavigationManager::getSingleton();
			navMgr->setWalkableRadius(data->floatData[1][nullptr]);
		};
		//

		undoData->floatData[0][nullptr] = prevVal;
		undoData->floatData[1][nullptr] = newVal;
	}

	void NavigationWindow::onChangeMaxEdgeLength(float prevVal, float newVal)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change navigation max edge length");
		undoData->floatData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			NavigationManager* navMgr = NavigationManager::getSingleton();
			navMgr->setMaxEdgeLen(data->floatData[0][nullptr]);
		};

		undoData->redoAction = [=](UndoData* data)
		{
			NavigationManager* navMgr = NavigationManager::getSingleton();
			navMgr->setMaxEdgeLen(data->floatData[1][nullptr]);
		};
		//

		undoData->floatData[0][nullptr] = prevVal;
		undoData->floatData[1][nullptr] = newVal;
	}

	void NavigationWindow::onChangeMaxSimplificationError(float prevVal, float newVal)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change navigation max simplification error");
		undoData->floatData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			NavigationManager* navMgr = NavigationManager::getSingleton();
			navMgr->setMaxSimplificationError(data->floatData[0][nullptr]);
		};

		undoData->redoAction = [=](UndoData* data)
		{
			NavigationManager* navMgr = NavigationManager::getSingleton();
			navMgr->setMaxSimplificationError(data->floatData[1][nullptr]);
		};
		//

		undoData->floatData[0][nullptr] = prevVal;
		undoData->floatData[1][nullptr] = newVal;
	}

	void NavigationWindow::onChangeMinRegionArea(float prevVal, float newVal)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change navigation min region area");
		undoData->floatData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			NavigationManager* navMgr = NavigationManager::getSingleton();
			navMgr->setMinRegionArea(data->floatData[0][nullptr]);
		};

		undoData->redoAction = [=](UndoData* data)
		{
			NavigationManager* navMgr = NavigationManager::getSingleton();
			navMgr->setMinRegionArea(data->floatData[1][nullptr]);
		};
		//

		undoData->floatData[0][nullptr] = prevVal;
		undoData->floatData[1][nullptr] = newVal;
	}

	void NavigationWindow::onChangeMergeRegionArea(float prevVal, float newVal)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change navigation merge region area");
		undoData->floatData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			NavigationManager* navMgr = NavigationManager::getSingleton();
			navMgr->setMergeRegionArea(data->floatData[0][nullptr]);
		};

		undoData->redoAction = [=](UndoData* data)
		{
			NavigationManager* navMgr = NavigationManager::getSingleton();
			navMgr->setMergeRegionArea(data->floatData[1][nullptr]);
		};
		//

		undoData->floatData[0][nullptr] = prevVal;
		undoData->floatData[1][nullptr] = newVal;
	}

	void NavigationWindow::onChangeCellSize(float prevVal, float newVal)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change navigation cell size");
		undoData->floatData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			NavigationManager* navMgr = NavigationManager::getSingleton();
			navMgr->setCellSize(data->floatData[0][nullptr]);
		};

		undoData->redoAction = [=](UndoData* data)
		{
			NavigationManager* navMgr = NavigationManager::getSingleton();
			navMgr->setCellSize(data->floatData[1][nullptr]);
		};
		//

		undoData->floatData[0][nullptr] = prevVal;
		undoData->floatData[1][nullptr] = newVal;
	}

	void NavigationWindow::onChangeCellHeight(float prevVal, float newVal)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change navigation cell height");
		undoData->floatData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			NavigationManager* navMgr = NavigationManager::getSingleton();
			navMgr->setCellHeight(data->floatData[0][nullptr]);
		};

		undoData->redoAction = [=](UndoData* data)
		{
			NavigationManager* navMgr = NavigationManager::getSingleton();
			navMgr->setCellHeight(data->floatData[1][nullptr]);
		};
		//

		undoData->floatData[0][nullptr] = prevVal;
		undoData->floatData[1][nullptr] = newVal;
	}

	void NavigationWindow::drawNavMesh(int viewId, int viewLayer, Camera* camera)
	{
		if (!MainWindow::getSingleton()->getSceneWindow()->showNavMesh)
			return;

		uint64_t navMeshState = BGFX_STATE_WRITE_RGB
			| BGFX_STATE_WRITE_A
			| BGFX_STATE_WRITE_Z
			| BGFX_STATE_DEPTH_TEST_LEQUAL
			| BGFX_STATE_BLEND_FUNC_SEPARATE(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA, BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_ONE)
			| BGFX_STATE_CULL_CW;

		glm::mat4x4 trans = glm::identity<glm::mat4x4>();

		auto& vertices = NavigationManager::getSingleton()->getNavMeshVertices();

		if (prevVerticesCount != vertices.size())
		{
			prevVerticesCount = vertices.size();
			if (bgfx::isValid(vbh))
				bgfx::destroy(vbh);

			vbh = { bgfx::kInvalidHandle };
			
			if (prevVerticesCount > 0)
			{
				vbh = bgfx::createDynamicVertexBuffer(vertices.size(), VertexLayouts::verticesOnlyVertexLayout, BGFX_BUFFER_ALLOW_RESIZE);

				if (bgfx::isValid(vbh))
				{
					const bgfx::Memory* mem = bgfx::makeRef(&vertices[0], (sizeof(float) * 3) * vertices.size(), nullptr);
					bgfx::update(vbh, 0, mem);
				}
			}
		}

		if (!bgfx::isValid(vbh))
			return;

		//Draw
		float color[4] = { 0.0f, 0.66, 1.0f, 0.5f };

		bgfx::setVertexBuffer(0, vbh);

		bgfx::setTransform(glm::value_ptr(trans), 1);
		bgfx::setState(navMeshState);

		bgfx::setTexture(0, Renderer::getSingleton()->getAlbedoMapUniform(), Texture::getNullTexture()->getHandle());
		bgfx::setUniform(Renderer::getSingleton()->getColorUniform(), color, 1);

		bgfx::submit(viewId, Renderer::getSingleton()->getTransparentProgram());
	}
}