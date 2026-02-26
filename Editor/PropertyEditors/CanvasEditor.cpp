#include "CanvasEditor.h"

#include "../Engine/Components/Canvas.h"

#include "../Windows/MainWindow.h"
#include "../Windows/InspectorWindow.h"

#include "PropVector2.h"
#include "PropFloat.h"
#include "PropComboBox.h"

#include "../Classes/Undo.h"

namespace GX
{
	CanvasEditor::CanvasEditor()
	{
		setEditorName("CanvasEditor");
	}

	void CanvasEditor::init(std::vector<Component*> comps)
	{
		ComponentEditor::init(comps);

		Canvas* canvas = (Canvas*)comps[0];

		PropComboBox* mode = new PropComboBox(this, "Mode", { "Scale to screen size", "Match screen size" });
		mode->setOnChangeCallback([=](Property* prop, int val) { onChangeMode(prop, val); });
		mode->setCurrentItem(static_cast<int>(canvas->getMode()));

		PropVector2* refScreenSize = new PropVector2(this, "Reference screen size", canvas->getRefScreenSize());
		refScreenSize->setOnChangeCallback([=](Property* prop, glm::vec2 val) { onChangeRefScreenSize(prop, val); });

		addProperty(mode);
		addProperty(refScreenSize);

		if (canvas->getMode() == CanvasMode::ScaleToScreenSize)
		{
			PropFloat* screenMatchSide = new PropFloat(this, "Screen match side", canvas->getScreenMatchSide());
			screenMatchSide->setOnChangeCallback(([=](Property* prop, float val) { onChangeScreenMatchSide(prop, val); }));
			screenMatchSide->setMinValue(0.0f);
			screenMatchSide->setMaxValue(1.0f);

			addProperty(screenMatchSide);
		}
	}

	void CanvasEditor::onChangeRefScreenSize(Property* prop, glm::vec2 value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change canvas ref screen size");
		undoData->vec2Data.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec2Data[0])
			{
				Canvas* element = (Canvas*)d.first;
				element->setRefScreenSize(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->vec2Data[1])
			{
				Canvas* element = (Canvas*)d.first;
				element->setRefScreenSize(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Canvas* canvas = (Canvas*)*it;

			undoData->vec2Data[0][canvas] = canvas->getRefScreenSize();
			undoData->vec2Data[1][canvas] = value;

			canvas->setRefScreenSize(value);
		}
	}

	void CanvasEditor::onChangeScreenMatchSide(Property* prop, float value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change canvas screen match side");
		undoData->floatData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->floatData[0])
			{
				Canvas* element = (Canvas*)d.first;
				element->setScreenMatchSide(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->floatData[1])
			{
				Canvas* element = (Canvas*)d.first;
				element->setScreenMatchSide(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Canvas* canvas = (Canvas*)*it;

			undoData->floatData[0][canvas] = canvas->getScreenMatchSide();
			undoData->floatData[1][canvas] = value;

			canvas->setScreenMatchSide(value);
		}
	}

	void CanvasEditor::onChangeMode(Property* prop, int value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change canvas mode");
		undoData->intData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->intData[0])
			{
				Canvas* element = (Canvas*)d.first;
				element->setMode(static_cast<CanvasMode>(d.second));
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->intData[1])
			{
				Canvas* element = (Canvas*)d.first;
				element->setMode(static_cast<CanvasMode>(d.second));
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Canvas* canvas = (Canvas*)*it;

			undoData->intData[0][canvas] = static_cast<int>(canvas->getMode());
			undoData->intData[1][canvas] = value;

			canvas->setMode(static_cast<CanvasMode>(value));
		}

		updateEditor();
	}
}