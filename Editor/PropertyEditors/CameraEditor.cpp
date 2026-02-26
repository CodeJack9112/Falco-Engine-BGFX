#include "CameraEditor.h"

#include "../Engine/Core/GameObject.h"
#include "../Engine/Components/Camera.h"
#include "../Engine/Core/LayerMask.h"

#include "PropFloat.h"
#include "PropInt.h"
#include "PropVector2.h"
#include "PropColorPicker.h"
#include "PropComboBox.h"
#include "PropBool.h"

#include "../Classes/Undo.h"
#include "../Windows/MainWindow.h"
#include "../Windows/InspectorWindow.h"

namespace GX
{
	CameraEditor::CameraEditor()
	{
		setEditorName("CameraEditor");
	}

	CameraEditor::~CameraEditor()
	{
	}

	void CameraEditor::init(std::vector<Component*> comps)
	{
		ComponentEditor::init(comps);

		Camera* camera = (Camera*)comps[0];

		PropComboBox* clearFlags = new PropComboBox(this, "Clear flags", { "Skybox", "Solid Color", "Depth Only", "Don't Clear" });
		clearFlags->setCurrentItem(static_cast<int>(camera->getClearFlags()));
		clearFlags->setOnChangeCallback([=](Property* prop, int val) { onChangeClearFlags(prop, val); });

		auto layers = LayerMask::getAllLayers();
		layers.insert(layers.begin(), "Everything");
		layers.insert(layers.begin(), "Nothing");

		PropComboBox* cullingMask = new PropComboBox(this, "Culling mask", layers);
		LayerMask& mask = camera->getCullingMask();
		bool allVals = true;
		bool noneVals = true;
		for (int i = 2; i < layers.size(); ++i)
		{
			if (mask.getLayer(i - 2))
				noneVals = false;
			else
				allVals = false;
			cullingMask->setValueEnabled(i, mask.getLayer(i - 2));
		}
		cullingMask->setValueEnabled(0, noneVals);
		cullingMask->setValueEnabled(1, allVals);
		if (allVals) cullingMask->setLabel(layers[1]);
		else if (noneVals) cullingMask->setLabel(layers[0]);
		else cullingMask->setLabel("Mixed");

		cullingMask->setOnChangeCallback([=](Property* prop, int val) { onChangeCullingMask(prop, val); });

		PropColorPicker* clearColor = new PropColorPicker(this, "Clear color", camera->getClearColor());
		clearColor->setOnChangeCallback([=](Property* prop, Color val) { onChangeClearColor(prop, val); });

		PropInt* depth = new PropInt(this, "Depth", camera->getDepth());
		depth->setMinValue(0);
		depth->setMaxValue(1000);
		depth->setIsSlider(false);
		depth->setIsDraggable(true);
		depth->setOnChangeCallback([=](Property* prop, int val) { onChangeDepth(prop, val); });

		ProjectionType projType = camera->getProjectionType();

		PropComboBox* projectionType = new PropComboBox(this, "Projection", { "Perspective", "Orthographic" });
		projectionType->setOnChangeCallback([=](Property* prop, int val) { onChangeProjectionType(prop, val); });
		projectionType->setCurrentItem(projType == ProjectionType::Perspective ? 0 : 1);

		addProperty(clearFlags);
		addProperty(cullingMask);
		addProperty(clearColor);
		addProperty(depth);
		addProperty(projectionType);

		if (projType == ProjectionType::Perspective)
		{
			PropFloat* fov = new PropFloat(this, "FOV", camera->getFOVy());
			fov->setOnChangeCallback([=](Property* prop, float val) { onChangeFov(prop, val); });

			addProperty(fov);
		}
		else
		{
			PropFloat* orthoSize = new PropFloat(this, "Size", camera->getOrthographicSize());
			orthoSize->setOnChangeCallback([=](Property* prop, float val) { onChangeOrthographicSize(prop, val); });

			addProperty(orthoSize);
		}

		PropVector2* clipPlanes = new PropVector2(this, "Clipping planes", glm::vec2(camera->getNear(), camera->getFar()));
		clipPlanes->setOnChangeCallback([=](Property* prop, glm::vec2 val) { onChangeClipPlanes(prop, val); });

		PropVector2* viewportPos = new PropVector2(this, "Viewport position", glm::vec2(camera->getViewportLeft(), camera->getViewportTop()));
		viewportPos->setOnChangeCallback([=](Property* prop, glm::vec2 val) { onChangeViewportPos(prop, val); });

		PropVector2* viewportSize = new PropVector2(this, "Viewport size", glm::vec2(camera->getViewportWidth(), camera->getViewportHeight()));
		viewportSize->setOnChangeCallback([=](Property* prop, glm::vec2 val) { onChangeViewportSize(prop, val); });

		addProperty(clipPlanes);
		addProperty(viewportPos);
		addProperty(viewportSize);

		PropBool* occlusionCulling = new PropBool(this, "Occlusion culling", camera->getOcclusionCulling());
		occlusionCulling->setOnChangeCallback([=](Property* prop, bool val) { onChangeOcclusionCulling(prop, val); });

		addProperty(occlusionCulling);
	}

	void CameraEditor::onChangeClearFlags(Property* prop, int val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change camera clear flags");
		undoData->intData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->intData[0])
			{
				Camera* comp = (Camera*)d.first;
				comp->setClearFlags(static_cast<Camera::ClearFlags>(d.second));
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->intData[1])
			{
				Camera* comp = (Camera*)d.first;
				comp->setClearFlags(static_cast<Camera::ClearFlags>(d.second));
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Camera* cam = (Camera*)(*it);

			undoData->intData[0][cam] = static_cast<int>(cam->getClearFlags());
			undoData->intData[1][cam] = val;

			cam->setClearFlags(static_cast<Camera::ClearFlags>(val));
		}
	}

	void CameraEditor::onChangeCullingMask(Property* prop, int val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change camera culling mask");
		undoData->intData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->intData[0])
			{
				Camera* comp = (Camera*)d.first;
				LayerMask& mask = comp->getCullingMask();
				mask.fromULong((unsigned long)d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->intData[1])
			{
				Camera* comp = (Camera*)d.first;
				LayerMask& mask = comp->getCullingMask();
				mask.fromULong((unsigned long)d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		if (val == 0)
		{
			for (auto it = components.begin(); it != components.end(); ++it)
			{
				Camera* cam = (Camera*)(*it);
				LayerMask& mask = cam->getCullingMask();

				undoData->intData[0][cam] = (int)mask.toULong();
				
				for (int i = 0; i < LayerMask::MAX_LAYERS; ++i)
					mask.setLayer(i, false);

				undoData->intData[1][cam] = (int)mask.toULong();
			}
		}
		else if (val == 1)
		{
			for (auto it = components.begin(); it != components.end(); ++it)
			{
				Camera* cam = (Camera*)(*it);
				LayerMask& mask = cam->getCullingMask();

				undoData->intData[0][cam] = (int)mask.toULong();

				for (int i = 0; i < LayerMask::MAX_LAYERS; ++i)
					mask.setLayer(i, true);

				undoData->intData[1][cam] = (int)mask.toULong();
			}
		}
		else
		{
			Camera* cam = (Camera*)components[0];
			LayerMask& mask = cam->getCullingMask();
			bool lVal = !mask.getLayer(val - 2);

			for (auto it = components.begin(); it != components.end(); ++it)
			{
				cam = (Camera*)(*it);
				mask = cam->getCullingMask();

				undoData->intData[0][cam] = (int)mask.toULong();

				mask.setLayer(val - 2, lVal);

				undoData->intData[1][cam] = (int)mask.toULong();
			}
		}

		updateEditor();
	}

	void CameraEditor::onChangeClearColor(Property* prop, Color val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change camera clear color");
		undoData->vec4Data.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec4Data[0])
			{
				Camera* comp = (Camera*)d.first;
				comp->setClearColor(Color(d.second.x, d.second.y, d.second.z, d.second.w));
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec4Data[1])
			{
				Camera* comp = (Camera*)d.first;
				comp->setClearColor(Color(d.second.x, d.second.y, d.second.z, d.second.w));
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Camera* cam = (Camera*)(*it);

			Color col = cam->getClearColor();
			undoData->vec4Data[0][cam] = glm::vec4(col[0], col[1], col[2], col[3]);
			undoData->vec4Data[1][cam] = glm::vec4(val[0], val[1], val[2], val[3]);

			cam->setClearColor(val);
		}
	}

	void CameraEditor::onChangeDepth(Property* prop, int val)
	{
		if (val > -1)
		{
			//Undo
			UndoData* undoData = Undo::addUndo("Change camera depth");
			undoData->intData.resize(2);

			undoData->undoAction = [=](UndoData* data)
			{
				for (auto& d : data->intData[0])
				{
					Camera* comp = (Camera*)d.first;
					comp->setDepth(d.second);
				}

				MainWindow::getInspectorWindow()->updateCurrentEditor();
			};

			undoData->redoAction = [=](UndoData* data)
			{
				for (auto& d : data->intData[1])
				{
					Camera* comp = (Camera*)d.first;
					comp->setDepth(d.second);
				}

				MainWindow::getInspectorWindow()->updateCurrentEditor();
			};
			//

			for (auto it = components.begin(); it != components.end(); ++it)
			{
				Camera* cam = (Camera*)(*it);
				
				undoData->intData[0][cam] = cam->getDepth();
				undoData->intData[1][cam] = val;

				cam->setDepth(val);
			}
		}
	}

	void CameraEditor::onChangeFov(Property* prop, float val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change camera fov");
		undoData->floatData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->floatData[0])
			{
				Camera* comp = (Camera*)d.first;
				comp->setFOVy(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->floatData[1])
			{
				Camera* comp = (Camera*)d.first;
				comp->setFOVy(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Camera* cam = (Camera*)(*it);

			undoData->floatData[0][cam] = cam->getFOVy();
			undoData->floatData[1][cam] = val;

			cam->setFOVy(val);
		}
	}

	void CameraEditor::onChangeClipPlanes(Property* prop, glm::vec2 val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change camera clipping planes");
		undoData->vec2Data.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec2Data[0])
			{
				Camera* comp = (Camera*)d.first;
				comp->setNear(d.second.x);
				comp->setFar(d.second.y);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec2Data[1])
			{
				Camera* comp = (Camera*)d.first;
				comp->setNear(d.second.x);
				comp->setFar(d.second.y);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Camera* cam = (Camera*)(*it);

			undoData->vec2Data[0][cam] = glm::vec2(cam->getNear(), cam->getFar());
			undoData->vec2Data[1][cam] = val;

			cam->setNear(val.x);
			cam->setFar(val.y);
		}
	}

	void CameraEditor::onChangeViewportPos(Property* prop, glm::vec2 val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change camera viewport position");
		undoData->vec2Data.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec2Data[0])
			{
				Camera* comp = (Camera*)d.first;
				comp->setViewportLeft(d.second.x);
				comp->setViewportTop(d.second.y);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec2Data[1])
			{
				Camera* comp = (Camera*)d.first;
				comp->setViewportLeft(d.second.x);
				comp->setViewportTop(d.second.y);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Camera* cam = (Camera*)(*it);

			undoData->vec2Data[0][cam] = glm::vec2(cam->getViewportLeft(), cam->getViewportTop());
			undoData->vec2Data[1][cam] = val;

			cam->setViewportLeft(val.x);
			cam->setViewportTop(val.y);
		}
	}

	void CameraEditor::onChangeViewportSize(Property* prop, glm::vec2 val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change camera viewport size");
		undoData->vec2Data.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec2Data[0])
			{
				Camera* comp = (Camera*)d.first;
				comp->setViewportWidth(d.second.x);
				comp->setViewportHeight(d.second.y);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec2Data[1])
			{
				Camera* comp = (Camera*)d.first;
				comp->setViewportWidth(d.second.x);
				comp->setViewportHeight(d.second.y);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Camera* cam = (Camera*)(*it);

			undoData->vec2Data[0][cam] = glm::vec2(cam->getViewportWidth(), cam->getViewportHeight());
			undoData->vec2Data[1][cam] = val;

			cam->setViewportWidth(val.x);
			cam->setViewportHeight(val.y);
		}
	}

	void CameraEditor::onChangeProjectionType(Property* prop, int val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change camera projection type");
		undoData->intData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->intData[0])
			{
				Camera* comp = (Camera*)d.first;
				comp->setProjectionType(static_cast<ProjectionType>(d.second));
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->intData[1])
			{
				Camera* comp = (Camera*)d.first;
				comp->setProjectionType(static_cast<ProjectionType>(d.second));
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Camera* cam = (Camera*)(*it);

			undoData->intData[0][cam] = static_cast<int>(cam->getProjectionType());
			undoData->intData[1][cam] = val;

			cam->setProjectionType(static_cast<ProjectionType>(val));
		}

		updateEditor();
	}

	void CameraEditor::onChangeOrthographicSize(Property* prop, float val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change camera orthographic size");
		undoData->floatData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->floatData[0])
			{
				Camera* comp = (Camera*)d.first;
				comp->setOrthographicSize(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->floatData[1])
			{
				Camera* comp = (Camera*)d.first;
				comp->setOrthographicSize(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Camera* cam = (Camera*)(*it);

			undoData->floatData[0][cam] = cam->getOrthographicSize();
			undoData->floatData[1][cam] = val;

			cam->setOrthographicSize(val);
		}
	}

	void CameraEditor::onChangeOcclusionCulling(Property* prop, bool val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change camera occlusion culling");
		undoData->boolData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[0])
			{
				Camera* comp = (Camera*)d.first;
				comp->setOcclusionCulling(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[1])
			{
				Camera* comp = (Camera*)d.first;
				comp->setOcclusionCulling(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Camera* cam = (Camera*)(*it);

			undoData->boolData[0][cam] = cam->getOcclusionCulling();
			undoData->boolData[1][cam] = val;

			cam->setOcclusionCulling(val);
		}
	}
}