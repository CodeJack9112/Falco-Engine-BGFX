#include "WaterEditor.h"

#include "../../Engine/Components/Water.h"
#include "../../Engine/Assets/Texture.h"

#include "PropInt.h"
#include "PropFloat.h"
#include "PropBool.h"
#include "PropEditorHost.h"
#include "PropComboBox.h"

#include "MaterialEditor.h"
#include "../Engine/Classes/Helpers.h"

#include "../Windows/MainWindow.h"
#include "../Windows/InspectorWindow.h"
#include "../Classes/Undo.h"

namespace GX
{
	WaterEditor::WaterEditor()
	{
		setEditorName("WaterEditor");
	}

	WaterEditor::~WaterEditor()
	{
	}

	void WaterEditor::init(std::vector<Component*> comps)
	{
		ComponentEditor::init(comps);

		Water* water = (Water*)comps[0];

		PropInt* size = new PropInt(this, "Size", water->getSize());
		size->setMinValue(10);
		size->setMaxValue(500);
		size->setOnChangeCallback([=](Property* prop, int val) { onSetSize(prop, val); });
		
		addProperty(size);

		PropBool* reflections = new PropBool(this, "Reflections", water->getReflections());
		reflections->setOnChangeCallback([=](Property* prop, bool val) { onSetReflections(prop, val); });

		addProperty(reflections);

		if (water->getReflections())
		{
			PropBool* reflectSkybox = new PropBool(this, "Reflect skybox", water->getReflectSkybox());
			reflectSkybox->setOnChangeCallback([=](Property* prop, bool val) { onSetReflectSkybox(prop, val); });

			addProperty(reflectSkybox);

			PropBool* reflectObjects = new PropBool(this, "Reflect objects", water->getReflectObjects());
			reflectObjects->setOnChangeCallback([=](Property* prop, bool val) { onSetReflectObjects(prop, val); });

			addProperty(reflectObjects);

			PropFloat* reflectionsDistance = new PropFloat(this, "Reflections distance", water->getReflectionsDistance());
			reflectionsDistance->setOnChangeCallback([=](Property* prop, float val) { onSetReflectionsDistance(prop, val); });

			addProperty(reflectionsDistance);

			PropComboBox* reflectionsQuality = new PropComboBox(this, "Reflections quality", { "Low", "Normal", "High" });
			reflectionsQuality->setCurrentItem(water->getReflectionsQuality());
			reflectionsQuality->setOnChangeCallback([=](Property* prop, float val) { onSetReflectionsQuality(prop, val); });

			addProperty(reflectionsQuality);
		}

		MaterialEditor* materialEditor = new MaterialEditor();
		materialEditor->setIsShaderEditable(false);
		materialEditor->setSaveOnEdit(false);
		materialEditor->init(water->getMaterial());
		PropEditorHost* mat_prop = new PropEditorHost(this, "Water settings", materialEditor);
		mat_prop->icon = Texture::load(Helper::ExePath(), "Editor/Icons/Assets/material.png", false, Texture::CompressionMethod::None, true);

		addProperty(mat_prop);
	}

	void WaterEditor::onSetSize(Property* prop, int value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change water size");
		undoData->intData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->intData[0])
			{
				Water* element = (Water*)d.first;
				element->setSize(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->intData[1])
			{
				Water* element = (Water*)d.first;
				element->setSize(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto jt = components.begin(); jt != components.end(); ++jt)
		{
			Water* water = (Water*)*jt;

			undoData->intData[0][water] = water->getSize();
			undoData->intData[1][water] = value;

			water->setSize(value);
		}
	}

	void WaterEditor::onSetReflections(Property* prop, bool value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change water reflections enabled");
		undoData->boolData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[0])
			{
				Water* element = (Water*)d.first;
				element->setReflections(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[1])
			{
				Water* element = (Water*)d.first;
				element->setReflections(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto jt = components.begin(); jt != components.end(); ++jt)
		{
			Water* water = (Water*)*jt;

			undoData->boolData[0][water] = water->getReflections();
			undoData->boolData[1][water] = value;

			water->setReflections(value);
		}

		updateEditor();
	}

	void WaterEditor::onSetReflectSkybox(Property* prop, bool value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change water reflect skybox");
		undoData->boolData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[0])
			{
				Water* element = (Water*)d.first;
				element->setReflectSkybox(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[1])
			{
				Water* element = (Water*)d.first;
				element->setReflectSkybox(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto jt = components.begin(); jt != components.end(); ++jt)
		{
			Water* water = (Water*)*jt;

			undoData->boolData[0][water] = water->getReflectSkybox();
			undoData->boolData[1][water] = value;

			water->setReflectSkybox(value);
		}
	}

	void WaterEditor::onSetReflectObjects(Property* prop, bool value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change water reflect objects");
		undoData->boolData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[0])
			{
				Water* element = (Water*)d.first;
				element->setReflectObjects(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[1])
			{
				Water* element = (Water*)d.first;
				element->setReflectObjects(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto jt = components.begin(); jt != components.end(); ++jt)
		{
			Water* water = (Water*)*jt;

			undoData->boolData[0][water] = water->getReflectObjects();
			undoData->boolData[1][water] = value;

			water->setReflectObjects(value);
		}
	}

	void WaterEditor::onSetReflectionsDistance(Property* prop, float value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change water reflections distance");
		undoData->floatData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->floatData[0])
			{
				Water* element = (Water*)d.first;
				element->setReflectionsDistance(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->floatData[1])
			{
				Water* element = (Water*)d.first;
				element->setReflectionsDistance(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto jt = components.begin(); jt != components.end(); ++jt)
		{
			Water* water = (Water*)*jt;

			undoData->floatData[0][water] = water->getReflectionsDistance();
			undoData->floatData[1][water] = value;

			water->setReflectionsDistance(value);
		}
	}

	void WaterEditor::onSetReflectionsQuality(Property* prop, int value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change water reflections quality");
		undoData->intData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->intData[0])
			{
				Water* element = (Water*)d.first;
				element->setReflectionsQuality(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->intData[1])
			{
				Water* element = (Water*)d.first;
				element->setReflectionsQuality(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto jt = components.begin(); jt != components.end(); ++jt)
		{
			Water* water = (Water*)*jt;

			undoData->intData[0][water] = water->getReflectionsQuality();
			undoData->intData[1][water] = value;

			water->setReflectionsQuality(value);
		}
	}
}